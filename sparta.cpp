/* Franny 1.0
 * Sparta 2.0 disk format routines
 * by Rafal BOBER Ciepiela
 * contact: bob_er@interia.pl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sparta.h"

using namespace std;
using namespace sparta;
using namespace types;

const char *SPARTA_DIR_DELIM=">";

SPARTA::SPARTA(void)
{
	media_image=NULL;
	current_path=NULL;
	*file_mask='\0';
	sparta_buffer=new unsigned char[SECTOR_DOUBLE];
	main_dir_sector_map=dir_sector_map=dir_sector_map_cnt=disk_bit_map=disk_bit_map_cnt=disk_bit_map_cnt=0;
	set_mask=false;
}

SPARTA::SPARTA(IMAGE *media)
{
	media_image=media;
	sparta_buffer=new unsigned char[SECTOR_DOUBLE];
	media_image->ReadSector(sparta_buffer, 1);
	if((*(sparta_buffer+11)+(*(sparta_buffer+12)<<8))!=GetTotalSectors()) media_image->SetCorrupted(true);
	current_path=new char[3];
	strcpy(current_path, ">");
	main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	disk_bit_map=*(sparta_buffer+16)+(*(sparta_buffer+17)<<8);
	disk_bit_map_size=*(sparta_buffer+15);
	dir_sector_map_cnt=disk_bit_map_cnt=0;
	set_mask=false;
}

bool SPARTA::Open(IMAGE *media)
{
	media_image=media;
	media_image->ReadSector(sparta_buffer, 1);
	if((*(sparta_buffer+11)+(*(sparta_buffer+12)<<8))!=GetTotalSectors()) media_image->SetCorrupted(true);
	current_path=new char[3];
	strcpy(current_path, ">");
	main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	disk_bit_map=*(sparta_buffer+16)+(*(sparta_buffer+17)<<8);
	disk_bit_map_size=*(sparta_buffer+15);
	dir_sector_map_cnt=disk_bit_map_cnt=0;
	set_mask=false;
	return true;
}

bool SPARTA::CheckFileName(char *filename, char *mask)
{
	int filename_len, mask_len, filename_seek=0, mask_seek=0;
	bool asterisk=false;

	if((filename==NULL) || (mask==NULL)) return false;
	if(strcmp(mask, "*.*")==0) return true;
	filename_len=(int)strlen(filename);
	mask_len=(int)strlen(mask);
	while((filename_seek<filename_len) && (mask_seek<mask_len))
	{
		if((*(mask+mask_seek)=='?') && (*(filename+filename_seek)!='\0'))
			{ ++filename_seek; ++mask_seek; continue; }
		if(*(mask+mask_seek)=='*')
		{
			++mask_seek;
			if(*(mask+mask_seek)=='\0')
				{ filename_seek=filename_len; break; }
			asterisk=true;
		}
		if((*(filename+filename_seek)!=*(mask+mask_seek)) && (asterisk==false)) return false;
		if(asterisk==true)
		{
			if(*(mask+mask_seek)==*(filename+filename_seek)) asterisk=false;
		}
		++filename_seek;
		if(asterisk==false) ++mask_seek;
	}
	if((filename_seek<filename_len) || ((mask_seek<mask_len) && (asterisk==false))) return false;
	return true;
}

char * SPARTA::PrepareName(char *name)
{
	char a, b, c;
	char *result;

	if(name==NULL) return NULL;
	result=new char[MAX_FILENAME_LENGTH];
	memset(result, ' ', MAX_FILENAME_LENGTH);
	b=strlen(name);
	c=0;
	for(a=0;a<b;a++)
	{
		if(a>11) break;
		if(*(name+a)=='.') { c=8; continue; }
		else *(result+c)=*(name+a);
		if((*(result+c)>='a') && (*(result+c)<='z')) *(result+c)-=0x20;
		++c;
	}
	return result;
}

int SPARTA::FindNextFileSector(int bit_map, unsigned int *bit_map_cnt)
{
	int a, b, c;

	media_image->ReadSector(sparta_buffer, bit_map);
	c=media_image->GetSectorSize();
	a=*bit_map_cnt;
	while(((a<<1)+4)>=c)
	{
		b=*sparta_buffer+(*(sparta_buffer+1)<<8);
		if(b==0) return 0;
		media_image->ReadSector(sparta_buffer, b);
		a=a-((c-4)>>1);
	}
	c=*(sparta_buffer+4+(a<<1))+(*(sparta_buffer+5+(a<<1))<<8);
	return c;
}

bool SPARTA::FindFirstFile(ATR_FILE *file, char *mask)
{
	unsigned int a, b, c;
	unsigned char dir_entry[DIR_ENTRY_SIZE];
	char filename[16];

	if((file==NULL) || (strlen(mask)>MAX_FILENAME_LENGTH)) return false;
	if(dir_sector_map==0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	}
	memset(file, 0, sizeof(ATR_FILE));
	strcpy(file_mask, mask);
	set_mask=true;
	dir_sector_map_cnt=0;
	current_file=1;
	current_dir_index=current_file*DIR_ENTRY_SIZE;
	b=media_image->GetSectorSize();
	a=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
	if(a==0) return false;
	if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
	current_dir_bsize=*(sparta_buffer+3)+(*(sparta_buffer+4)<<8)+(*(sparta_buffer+5)<<16);
	while((current_file*DIR_ENTRY_SIZE)<current_dir_bsize)
	{
		++current_file;
		if((current_dir_index+DIR_ENTRY_SIZE)<b)
		{
			memcpy(dir_entry, sparta_buffer+current_dir_index, DIR_ENTRY_SIZE);
			current_dir_index+=DIR_ENTRY_SIZE;
		}
		else
		{
			a=b-current_dir_index;
			++dir_sector_map_cnt;
			memcpy(dir_entry, sparta_buffer+current_dir_index, a);
			c=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
			if(c==0) return false;
			if(b!=media_image->ReadSector(sparta_buffer, c)) return false;
			current_dir_index=DIR_ENTRY_SIZE-a;
			memcpy(dir_entry+a, sparta_buffer, current_dir_index);
		}
		if((dir_entry[0]&0x10)>0) continue;
		if(dir_entry[0]==0) return false;
		for(c=0;c<8;c++)
		{
			if(dir_entry[6+c]==' ') break;
			*(file->Name+c)=dir_entry[6+c];
		}
		*(file->Name+c)='\0';
		for(c=0;c<3;c++)
		{
			if(dir_entry[14+c]==' ') break;
			*(file->Ext+c)=dir_entry[14+c];
		}
		*(file->Ext+c)='\0';
		strcpy(filename, file->Name);
		strcat(filename, ".");
		strcat(filename, file->Ext);
		if(!CheckFileName(filename, file_mask)) continue;
		file->Attributes=dir_entry[0];
		file->SizeInBytes=dir_entry[3]+(dir_entry[4]<<8)+(dir_entry[5]<<16);
		(b==SECTOR_SINGLE)?file->SizeInSectors=(file->SizeInBytes>>7):file->SizeInSectors=(file->SizeInBytes>>8);
		(b==SECTOR_SINGLE)?c=0x7f:c=0xff;
		if((file->SizeInBytes&c)>0) ++file->SizeInSectors;
		file->FirstSector=dir_entry[1]+(dir_entry[2]<<8);
		file->FileId=current_file-1;
		file->Day=dir_entry[17];
		file->Month=dir_entry[18];
		file->Year=dir_entry[19];
		file->Hour=dir_entry[20];
		file->Minute=dir_entry[21];
		file->Second=dir_entry[22];
		return true;
	}
	return false;
}

bool SPARTA::FindNextFile(ATR_FILE *file)
{
	unsigned int a, b, c;
	unsigned char dir_entry[DIR_ENTRY_SIZE];
	char filename[16];

	if((file==NULL) || (set_mask==false)) return false;
	b=GetSectorSize();
	a=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
	if(a==0) return false;
	if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
	while((current_file*DIR_ENTRY_SIZE)<current_dir_bsize)
	{
		++current_file;
		if((current_dir_index+DIR_ENTRY_SIZE)<b)
		{
			memcpy(dir_entry, sparta_buffer+current_dir_index, DIR_ENTRY_SIZE);
			current_dir_index+=DIR_ENTRY_SIZE;
		}
		else
		{
			a=b-current_dir_index;
			++dir_sector_map_cnt;
			memcpy(dir_entry, sparta_buffer+current_dir_index, a);
			c=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
			if(c==0) return false;
			if(b!=media_image->ReadSector(sparta_buffer, c)) return false;
			current_dir_index=DIR_ENTRY_SIZE-a;
			memcpy(dir_entry+a, sparta_buffer, current_dir_index);
		}
		if((dir_entry[0]&0x10)>0) continue;
		if(dir_entry[0]==0) return false;
		for(c=0;c<8;c++)
		{
			if(dir_entry[6+c]==' ') break;
			*(file->Name+c)=dir_entry[6+c];
		}
		*(file->Name+c)='\0';
		for(c=0;c<3;c++)
		{
			if(dir_entry[14+c]==' ') break;
			*(file->Ext+c)=dir_entry[14+c];
		}
		*(file->Ext+c)='\0';
		strcpy(filename, file->Name);
		strcat(filename, ".");
		strcat(filename, file->Ext);
		if(!CheckFileName(filename, file_mask)) continue;
		file->Attributes=dir_entry[0];
		file->SizeInBytes=dir_entry[3]+(dir_entry[4]<<8)+(dir_entry[5]<<16);
		(b==SECTOR_SINGLE)?file->SizeInSectors=(file->SizeInBytes>>7):file->SizeInSectors=(file->SizeInBytes>>8);
		(b==SECTOR_SINGLE)?c=0x7f:c=0xff;
		if((file->SizeInBytes&c)>0) ++file->SizeInSectors;
		file->FirstSector=dir_entry[1]+(dir_entry[2]<<8);
		file->FileId=current_file-1;
		file->Day=dir_entry[17];
		file->Month=dir_entry[18];
		file->Year=dir_entry[19];
		file->Hour=dir_entry[20];
		file->Minute=dir_entry[21];
		file->Second=dir_entry[22];
		return true;
	}
	return false;
}

bool SPARTA::FindFirstFile_Private(ATR_FILE *file, char *mask)
{
	unsigned int a, b, c;
	unsigned int local_dir_sector_map_cnt, local_current_file, local_current_dir_index, local_current_dir_bsize;
	unsigned char dir_entry[DIR_ENTRY_SIZE];
	char filename[16];

	if((file==NULL) || (strlen(mask)>MAX_FILENAME_LENGTH)) return false;
	if(dir_sector_map==0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	}
	memset(file, 0, sizeof(ATR_FILE));
	local_dir_sector_map_cnt=0;
	local_current_file=1;
	local_current_dir_index=local_current_file*DIR_ENTRY_SIZE;
	b=GetSectorSize();
	a=FindNextFileSector(dir_sector_map, &local_dir_sector_map_cnt);
	if(a==0) return false;
	if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
	local_current_dir_bsize=*(sparta_buffer+3)+(*(sparta_buffer+4)<<8)+(*(sparta_buffer+5)<<16);
	while((local_current_file*DIR_ENTRY_SIZE)<local_current_dir_bsize)
	{
		++local_current_file;
		if((local_current_dir_index+DIR_ENTRY_SIZE)<b)
		{
			memcpy(dir_entry, sparta_buffer+local_current_dir_index, DIR_ENTRY_SIZE);
			local_current_dir_index+=DIR_ENTRY_SIZE;
		}
		else
		{
			a=b-local_current_dir_index;
			++local_dir_sector_map_cnt;
			memcpy(dir_entry, sparta_buffer+local_current_dir_index, a);
			c=FindNextFileSector(dir_sector_map, &local_dir_sector_map_cnt);
			if(c==0) return false;
			if(b!=media_image->ReadSector(sparta_buffer, c)) return false;
			local_current_dir_index=DIR_ENTRY_SIZE-a;
			memcpy(dir_entry+a, sparta_buffer, local_current_dir_index);
		}
		if((dir_entry[0]&0x10)>0) continue;
		if(dir_entry[0]==0) return false;
		for(c=0;c<8;c++)
		{
			if(dir_entry[6+c]==' ') break;
			*(file->Name+c)=dir_entry[6+c];
		}
		*(file->Name+c)='\0';
		for(c=0;c<3;c++)
		{
			if(dir_entry[14+c]==' ') break;
			*(file->Ext+c)=dir_entry[14+c];
		}
		*(file->Ext+c)='\0';
		strcpy(filename, file->Name);
		strcat(filename, ".");
		strcat(filename, file->Ext);
		if(!CheckFileName(filename, mask)) continue;
		file->Attributes=dir_entry[0];
		file->SizeInBytes=dir_entry[3]+(dir_entry[4]<<8)+(dir_entry[5]<<16);
		(b==SECTOR_SINGLE)?file->SizeInSectors=(file->SizeInBytes>>7):file->SizeInSectors=(file->SizeInBytes>>8);
		(b==SECTOR_SINGLE)?c=0x7f:c=0xff;
		if((file->SizeInBytes&c)>0) ++file->SizeInSectors;
		file->FirstSector=dir_entry[1]+(dir_entry[2]<<8);
		file->FileId=local_current_file-1;
		file->Day=dir_entry[17];
		file->Month=dir_entry[18];
		file->Year=dir_entry[19];
		file->Hour=dir_entry[20];
		file->Minute=dir_entry[21];
		file->Second=dir_entry[22];
		return true;
	}
	return false;
}

bool SPARTA::Format(void)
{
	int a, b;
	bool first_sector_map;

	memset(sparta_buffer, 0, SECTOR_SINGLE);
	b=GetTotalSectors();
	disk_bit_map=a=4;
	disk_bit_map_cnt=0;
	first_sector_map=true;
	while((b>>3)>GetSectorSize())
	{
		memset(sparta_buffer, 0xff, GetSectorSize());
		if(first_sector_map==true)
		{
			*(sparta_buffer)=0x0f;
			first_sector_map=false;
		}
		media_image->SaveSector(sparta_buffer, a);
		b-=(GetSectorSize()<<3);
		++a;
		++disk_bit_map_cnt;
	}
	if(b>0)
	{
		int c;
		memset(sparta_buffer, 0, GetSectorSize());
		for(c=0;c<b;c++)
		{
			*(sparta_buffer+(c>>3))=*(sparta_buffer+(c>>3))|SPARTA_BIT_MASKS[c&0x7];
		}
		if(first_sector_map==true) *(sparta_buffer)=0x0f;
		media_image->SaveSector(sparta_buffer, a);
		++disk_bit_map_cnt;
	}
	dir_sector_map=++a;
	++a;
	dir_sector_map_cnt=0;
	memset(sparta_buffer, 0, GetSectorSize());
	*(sparta_buffer+4)=(unsigned char)a;
	*(sparta_buffer+5)=(unsigned char)(a>>8);
	media_image->SaveSector(sparta_buffer, dir_sector_map);
	*(sparta_buffer)=0x28;
	*(sparta_buffer+1)=*(sparta_buffer+2)=0;
	*(sparta_buffer+3)=DIR_ENTRY_SIZE;
	*(sparta_buffer+4)=*(sparta_buffer+5)=0;
	memcpy((char *)sparta_buffer+6, "MAIN_DIR", 11);
	media_image->SaveSector(sparta_buffer, a++);
// ---------------------- first sector
	b=media_image->GetTotalSectors();
	*(sparta_buffer+7)=(unsigned char)0x80;
	*(sparta_buffer+9)=(unsigned char)(dir_sector_map&0xff);
	*(sparta_buffer+10)=(unsigned char)(dir_sector_map>>8);
	*(sparta_buffer+11)=(unsigned char)(b&0xff);
	*(sparta_buffer+12)=(unsigned char)(b>>8);
	b-=4;		// kolejne sektory zajmuje SetSectorState()
	*(sparta_buffer+13)=(unsigned char)(b&0xff);
	*(sparta_buffer+14)=(unsigned char)(b>>8);
	*(sparta_buffer+15)=(unsigned char)disk_bit_map_cnt;
	*(sparta_buffer+16)=(unsigned char)disk_bit_map;
	*(sparta_buffer+17)=(unsigned char)(disk_bit_map>>8);
	*(sparta_buffer+20)=(unsigned char)a;
	*(sparta_buffer+21)=(unsigned char)(a>>8);
	*(sparta_buffer+18)=(unsigned char)(a+0x21);
	*(sparta_buffer+19)=(unsigned char)((a+0x21)>>8);
	memcpy(sparta_buffer+22, DEFAULT_DISK_LABEL, 8);
	*(sparta_buffer+30)=1;
	*(sparta_buffer+31)=(unsigned char)GetSectorSize();
	*(sparta_buffer+32)=0x20;
	*(sparta_buffer+38)=0;
	*(sparta_buffer+39)=0xfa;
	*(sparta_buffer+40)=0;
	b=media_image->SaveSector(sparta_buffer, 1);
	if(b!=SECTOR_SINGLE) return false;
	b=4;
	while(b<a)
	{
		SetSectorState(b, SECTOR_FILL);
		++b;
	}
	disk_bit_map_cnt=0;
	return true;
}


bool SPARTA::CheckFS(void)
{
	media_image->ReadSector(sparta_buffer, 1);
	if(*(sparta_buffer+32)==0x20) return true;
	return false;
}

int SPARTA::LoadFile(unsigned char **buffer, char *filename)
{
	ATR_FILE file;
	unsigned int index=0;
	int a=0, b, c, sector;

	if(!FindFirstFile_Private(&file, filename)) return -1;
	sector=FindNextFileSector(file.FirstSector, &index);
	*buffer=new unsigned char[file.SizeInBytes];
	b=media_image->GetSectorSize();
	while(a<file.SizeInBytes)
	{
		c=file.SizeInBytes-a;
		if(c>=b)
		{
			media_image->ReadSector(*buffer+a, sector);
			++index;
			sector=FindNextFileSector(file.FirstSector, &index);
			a+=b;
		}
		else
		{
			media_image->ReadSector(sparta_buffer, sector);
			memcpy(*buffer+a, sparta_buffer, c);
			a+=c;
			break;
		}
	}
	return a;
}

int SPARTA::SaveFile(unsigned char *buffer, char *filename, unsigned int size)
{
	ATR_FILE file_info;
	tm *det_time;
	time_t cur_time;
	int map_sector, file_sector, saved_bytes, a, b, map_index=2, saved_sectors=0;
	char *temp_name;

	if((buffer==NULL) || (filename==NULL)) return -1;
	if(FindFirstFile_Private(&file_info, filename)) return -6;
	if((GetFreeSectors()*media_image->GetSectorSize())<=size) return -7;
	map_sector=FindFreeSector();
	if(SetSectorState(map_sector, SECTOR_FILL)==false) return -3;
	memset(sparta_buffer, 0, SECTOR_DOUBLE);
	media_image->SaveSector(sparta_buffer, map_sector);
	saved_bytes=0;
	while(size>saved_bytes)
	{
		if((size-saved_bytes)>GetSectorSize()) a=media_image->GetSectorSize();
		else a=size-saved_bytes;
		file_sector=FindFreeSector();
		memcpy(sparta_buffer, buffer+saved_bytes, a);
		if(media_image->SaveSector(sparta_buffer, file_sector)!=media_image->GetSectorSize()) return -2;
		saved_bytes+=a;
		++saved_sectors;
		if(SetSectorState(file_sector, SECTOR_FILL)==false) return -4;
		AllocateSector(map_sector, file_sector);
	}
	file_info.FirstSector=map_sector;
	file_info.SizeInBytes=size;
	file_info.SizeInSectors=saved_sectors;
	temp_name=PrepareName(filename);
	if(temp_name==NULL) return -5;
	memcpy(file_info.Name, temp_name, 8);
	file_info.Name[8]='\0';
	memcpy(file_info.Ext, temp_name+8, 3);
	file_info.Ext[3]='\0';
	delete[] temp_name;
	file_info.FileId=0;
	file_info.Attributes=0x08;
	cur_time=time(NULL);
	det_time=localtime(&cur_time);
	file_info.Year=det_time->tm_year-100;
	file_info.Month=det_time->tm_mon+1;
	file_info.Day=det_time->tm_mday;
	file_info.Hour=det_time->tm_hour;
	file_info.Minute=det_time->tm_min;
	file_info.Second=det_time->tm_sec;
	AddRowToDirectory(&file_info, dir_sector_map);
	IncSeqId();
	return 0;
}

bool SPARTA::DeleteFile(char *filename, bool clear_sector)
{
	unsigned int a, b, c, status_sector, status_index, map_sector, file_sector;
	unsigned int local_dir_sector_map_cnt, local_current_file, local_current_dir_index, local_current_dir_bsize;
	unsigned char dir_entry[DIR_ENTRY_SIZE];
	unsigned char *sector_buffer;
	char *dotted_filename;

	if(filename==NULL) return false;
	if(dir_sector_map==0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	}
	local_dir_sector_map_cnt=0;
	local_current_file=1;
	local_current_dir_index=local_current_file*DIR_ENTRY_SIZE;
	b=media_image->GetSectorSize();
	a=FindNextFileSector(dir_sector_map, &local_dir_sector_map_cnt);
	if(a==0) return false;
	if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
	local_current_dir_bsize=*(sparta_buffer+3)+(*(sparta_buffer+4)<<8)+(*(sparta_buffer+5)<<16);
	while((local_current_file*DIR_ENTRY_SIZE)<local_current_dir_bsize)
	{
		++local_current_file;
		status_sector=a;
		status_index=local_current_dir_index;
		if((local_current_dir_index+DIR_ENTRY_SIZE)<b)
		{
			memcpy(dir_entry, sparta_buffer+local_current_dir_index, DIR_ENTRY_SIZE);
			local_current_dir_index+=DIR_ENTRY_SIZE;
		}
		else
		{
			a=b-local_current_dir_index;
			++local_dir_sector_map_cnt;
			memcpy(dir_entry, sparta_buffer+local_current_dir_index, a);
			c=FindNextFileSector(dir_sector_map, &local_dir_sector_map_cnt);
			if(c==0) return false;
			if(b!=media_image->ReadSector(sparta_buffer, c)) return false;
			local_current_dir_index=DIR_ENTRY_SIZE-a;
			memcpy(dir_entry+a, sparta_buffer, local_current_dir_index);
		}
		if(dir_entry[0]==0) return false;
		if((dir_entry[0]&0x08)==0) continue;
		if((dir_entry[0]&0x20)>0) continue;
		dotted_filename=DottedFileName(&dir_entry[0]);
		if(!CheckFileName(dotted_filename, filename)) { delete[] dotted_filename; continue; }
		delete[] dotted_filename;
		map_sector=dir_entry[1]+(dir_entry[2]<<8);
		sector_buffer=new unsigned char[b];
		while(map_sector!=0)
		{
			media_image->ReadSector(sector_buffer, map_sector);
			SetSectorState(map_sector, SECTOR_EMPTY);
			if(clear_sector==true)
			{
				memset(sparta_buffer, 0, b);
				media_image->SaveSector(sparta_buffer, map_sector);
			}
			map_sector=*(sector_buffer)+(*(sector_buffer+1)<<8);
			c=4;
			while(c<b)
			{
				file_sector=*(sector_buffer+c)+(*(sector_buffer+c+1)<<8);
				c+=2;
				if(file_sector==0) continue;
				SetSectorState(file_sector, SECTOR_EMPTY);
				if(clear_sector==true)
				{
					memset(sparta_buffer, 0, b);
					media_image->SaveSector(sparta_buffer, file_sector);
				}
			}
		}
		delete[] sector_buffer;
		if(media_image->ReadSector(sparta_buffer, status_sector)!=b) return false;
		*(sparta_buffer+status_index)=*(sparta_buffer+status_index)&0xf7;
		*(sparta_buffer+status_index)=*(sparta_buffer+status_index)|0x10;
		if(media_image->SaveSector(sparta_buffer, status_sector)!=b) return false;
	}
	return true;
}

bool SPARTA::MakeDir(char *dirname)
{
	ATR_FILE file_info;
	tm *det_time;
	time_t cur_time;
	int map_sector, dir_sector, saved_bytes, a, b, map_index=2;
	char *temp_name;

	if(dirname==NULL) return false;
	if(FindFirstFile_Private(&file_info, dirname)) return false;
	map_sector=FindFreeDirSector();
	if(SetSectorState(map_sector, SECTOR_FILL)==false) return false;
	dir_sector=FindFreeSector();
	if(SetSectorState(dir_sector, SECTOR_FILL)==false) return false;
	memset(sparta_buffer, 0, SECTOR_DOUBLE);
	*(sparta_buffer+4)=(unsigned char)(dir_sector&0xff);
	*(sparta_buffer+5)=(unsigned char)(dir_sector>>8);
	media_image->SaveSector(sparta_buffer, map_sector);
	memset(sparta_buffer, 0, SECTOR_DOUBLE);
	*(sparta_buffer)=0xa8;
	*(sparta_buffer+1)=(unsigned char)(dir_sector_map&0xff);
	*(sparta_buffer+2)=(unsigned char)(dir_sector_map>>8);
	*(sparta_buffer+3)=DIR_ENTRY_SIZE;
	temp_name=PrepareName(dirname);
	if(temp_name==NULL) return -5;
	memcpy(sparta_buffer+6, temp_name, 11);
	memcpy(file_info.Name, temp_name, 8);
	file_info.Name[8]='\0';
	memcpy(file_info.Ext, temp_name+8, 3);
	file_info.Ext[3]='\0';
	delete[] temp_name;
	media_image->SaveSector(sparta_buffer, dir_sector);
	file_info.FirstSector=map_sector;
	file_info.SizeInBytes=DIR_ENTRY_SIZE;
	file_info.SizeInSectors=1;
	file_info.FileId=0;
	file_info.Attributes=0x28;
	cur_time=time(NULL);
	det_time=localtime(&cur_time);
	file_info.Year=det_time->tm_year-100;
	file_info.Month=det_time->tm_mon+1;
	file_info.Day=det_time->tm_mday;
	file_info.Hour=det_time->tm_hour;
	file_info.Minute=det_time->tm_min;
	file_info.Second=det_time->tm_sec;
	AddRowToDirectory(&file_info, dir_sector_map);
	IncSeqId();
	return true;
}

bool SPARTA::DeleteDir(char *dirname)
{
	list<int> sectors_to_free;
	list<int>::const_iterator sectors_to_free_ite;
	unsigned int a, b, c, status_sector, status_index, map_sector, dir_sector;
	unsigned int local_dir_sector_map_cnt, local_current_file, local_current_dir_index, local_current_dir_bsize;
	unsigned char dir_entry[DIR_ENTRY_SIZE];
	unsigned char *sector_buffer=NULL;
	char *dotted_dirname;
	bool result=false;

	if(dirname==NULL) return false;
	if(dir_sector_map==0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	}
	local_dir_sector_map_cnt=0;
	local_current_file=1;
	local_current_dir_index=local_current_file*DIR_ENTRY_SIZE;
	b=media_image->GetSectorSize();
	c=FindNextFileSector(dir_sector_map, &local_dir_sector_map_cnt);
	if(c==0) return false;
	if(b!=media_image->ReadSector(sparta_buffer, c)) return false;
	local_current_dir_bsize=*(sparta_buffer+3)+(*(sparta_buffer+4)<<8)+(*(sparta_buffer+5)<<16);
	while((local_current_file*DIR_ENTRY_SIZE)<local_current_dir_bsize)
	{
		++local_current_file;
		status_sector=c;
		status_index=local_current_dir_index;
		if((local_current_dir_index+DIR_ENTRY_SIZE)<b)
		{
			memcpy(dir_entry, sparta_buffer+local_current_dir_index, DIR_ENTRY_SIZE);
			local_current_dir_index+=DIR_ENTRY_SIZE;
		}
		else
		{
			a=b-local_current_dir_index;
			++local_dir_sector_map_cnt;
			memcpy(dir_entry, sparta_buffer+local_current_dir_index, a);
			c=FindNextFileSector(dir_sector_map, &local_dir_sector_map_cnt);
			if(c==0) return false;
			if(b!=media_image->ReadSector(sparta_buffer, c)) return false;
			local_current_dir_index=DIR_ENTRY_SIZE-a;
			memcpy(dir_entry+a, sparta_buffer, local_current_dir_index);
		}
		if(dir_entry[0]==0) return false;
		if((dir_entry[0]&0x20)==0) continue;
		dotted_dirname=DottedFileName(&dir_entry[0]);
		if(!CheckFileName(dotted_dirname, dirname)) { delete[] dotted_dirname; continue; }
		delete[] dotted_dirname;
		map_sector=dir_entry[1]+(dir_entry[2]<<8);
		sector_buffer=new unsigned char[b];
		sectors_to_free.clear();
		local_current_dir_index=DIR_ENTRY_SIZE;
		while(map_sector!=0)
		{
			result=true;
			media_image->ReadSector(sector_buffer, map_sector);
			sectors_to_free.push_back(map_sector);
			map_sector=(int)(*sector_buffer+(*(sector_buffer+1)<<8));
			c=4;
			while(c<b)
			{
				dir_sector=(int)(*(sector_buffer+c)+(*(sector_buffer+c+1)<<8));
				if(dir_sector==0) continue;
				c+=2;
				sectors_to_free.push_back(dir_sector);
				media_image->ReadSector(sparta_buffer, dir_sector);
				while(local_current_dir_index<b)
				{
					if(*(sparta_buffer+local_current_dir_index)==0) goto quit_func;
					if((*(sparta_buffer+local_current_dir_index)&0x10)==0)
					{
						sectors_to_free.clear();
						return false;
					}
					local_current_dir_index+=DIR_ENTRY_SIZE;
				}
				local_current_dir_index-=b;
			}
		}
	}
quit_func:
	if(sector_buffer!=NULL) delete[] sector_buffer;
	for(sectors_to_free_ite=sectors_to_free.begin();sectors_to_free_ite!=sectors_to_free.end();sectors_to_free_ite++)
	{
		SetSectorState(*sectors_to_free_ite, SECTOR_EMPTY);
	}
	sectors_to_free.clear();
	if(media_image->ReadSector(sparta_buffer, status_sector)!=b) return false;
	*(sparta_buffer+status_index)=*(sparta_buffer+status_index)&0xf7;
	*(sparta_buffer+status_index)=*(sparta_buffer+status_index)|0x10;
	if(media_image->SaveSector(sparta_buffer, status_sector)!=b) return false;
	IncSeqId();
	return result;
}

bool SPARTA::CD(char *directory)
{
	ATR_FILE file;
	int a, b;
	char *new_path;

	if(!FindFirstFile_Private(&file, directory)) return false;
	a=strlen(current_path);
	b=strlen(directory);
	new_path=new char[a+b+6];
	strcpy(new_path, current_path);
	strcpy(new_path+a, directory);
	*(new_path+a+b)=*(SPARTA_DIR_DELIM);
	*(new_path+a+b+1)='\0';
	delete[] current_path;
	current_path=new_path;
	dir_sector_map=file.FirstSector;
	dir_sector_map_cnt=0;
	return true;
}

int SPARTA::FindFreeSector()
{
	int sector, new_sector=0, a, b, c, d, seek, map_index, add_sector;

	sector=media_image->ReadSector(sparta_buffer, 1);
	if(sector!=SECTOR_SINGLE) return -1;
	sector=*(sparta_buffer+18)+(*(sparta_buffer+19)<<8);
	d=sector&0x07;
	if(media_image->GetSectorSize()==SECTOR_SINGLE)
	{
		map_index=sector>>10;
		seek=(sector>>3)&0x7f;
		add_sector=map_index*(SECTOR_SINGLE)<<3;
	}
	else
	{
		map_index=sector>>11;
		seek=(sector>>3)&0xff;
		add_sector=map_index*(SECTOR_DOUBLE)<<3;
	}
	b=media_image->ReadSector(sparta_buffer, disk_bit_map+map_index);
	while(new_sector==0)
	{
		for(a=seek;a<b;a++)
		{
			if(*(sparta_buffer+a)==0) { d=0; continue; }
			for(c=d;c<8;c++)
			{
				if((SPARTA_BIT_MASKS[c]&(*(sparta_buffer+a)))>0)
				{
					new_sector=(a<<3)+c+add_sector;
					goto quit_func;
				}
			}
			d=0;
		}
		seek=0;
		if(map_index==disk_bit_map_size) break;
		else b=media_image->ReadSector(sparta_buffer, disk_bit_map+(++map_index));
		add_sector+=(media_image->GetSectorSize()<<3);
	}
	map_index=0;
	add_sector=0;
	while(new_sector==0)
	{
		for(a=0;a<b;a++)
		{
			if(*(sparta_buffer+a)==0) continue;
			for(c=0;c<8;c++)
			{
				if((SPARTA_BIT_MASKS[c]&(*(sparta_buffer+a)))>0)
				{
					new_sector=(a<<3)+c+add_sector;
					goto quit_func;
				}
			}
		}
		if(map_index==disk_bit_map_size) break;
		else b=media_image->ReadSector(sparta_buffer, disk_bit_map+(++map_index));
		add_sector+=(media_image->GetSectorSize()<<3);
	}
quit_func:
	if(new_sector!=0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		*(sparta_buffer+18)=(unsigned char)(new_sector&0xff);
		*(sparta_buffer+19)=(unsigned char)(new_sector>>8);
		if(media_image->SaveSector(sparta_buffer, 1)!=SECTOR_SINGLE) return -2;
	}
	return new_sector;
}

int SPARTA::FindFreeDirSector()
{
	int sector, new_sector=0, a, b, c, d, seek, map_index, add_sector;

	sector=media_image->ReadSector(sparta_buffer, 1);
	if(sector!=SECTOR_SINGLE) return -1;
	sector=*(sparta_buffer+20)+(*(sparta_buffer+21)<<8);
	d=sector&0x07;
	if(media_image->GetSectorSize()==SECTOR_SINGLE)
	{
		map_index=sector>>10;
		seek=(sector>>3)&0x7f;
		add_sector=map_index*(SECTOR_SINGLE)<<3;
	}
	else
	{
		map_index=sector>>11;
		seek=(sector>>3)&0xff;
		add_sector=map_index*(SECTOR_DOUBLE)<<3;
	}
	b=media_image->ReadSector(sparta_buffer, disk_bit_map+map_index);
	while(new_sector==0)
	{
		for(a=seek;a<b;a++)
		{
			if(*(sparta_buffer+a)==0) { d=0; continue; }
			for(c=d;c<8;c++)
			{
				if((SPARTA_BIT_MASKS[c]&(*(sparta_buffer+a)))>0)
				{
					new_sector=(a<<3)+c+add_sector;
					goto quit_func;
				}
			}
			d=0;
		}
		seek=0;
		if(map_index==disk_bit_map_size) break;
		else b=media_image->ReadSector(sparta_buffer, disk_bit_map+(++map_index));
		add_sector+=(media_image->GetSectorSize()<<3);
	}
quit_func:
	if(new_sector!=0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		*(sparta_buffer+20)=(unsigned char)(new_sector&0xff);
		*(sparta_buffer+21)=(unsigned char)(new_sector>>8);
		if(media_image->SaveSector(sparta_buffer, 1)!=SECTOR_SINGLE) return -2;
	}
	return new_sector;
}

bool SPARTA::SetSectorState(unsigned int sector, bool state)
{
	int a, bitmap, mask;
	unsigned char bit_mask;
	char diff;

	if((sector<4) || (sector>media_image->GetTotalSectors())) return false;
	if(media_image->GetSectorSize()==SECTOR_SINGLE)
	{
		a=sector>>10;
		sector=sector&0x3ff;
	}
	else
	{
		a=sector>>11;
		sector=sector&0x7ff;
	}
	bitmap=disk_bit_map+a;
	a=media_image->ReadSector(sparta_buffer, bitmap);
	if(a!=GetSectorSize()) return false;
	if(state==SECTOR_FILL)
	{
		bit_mask=SPARTA_BIT_MASKS2[sector&0x07];
		*(sparta_buffer+(sector>>3))=*(sparta_buffer+(sector>>3))&bit_mask;
		diff=-1;
	}
	else
	{
		bit_mask=SPARTA_BIT_MASKS[sector&0x07];
		*(sparta_buffer+(sector>>3))=*(sparta_buffer+(sector>>3))|bit_mask;
		diff=1;
	}
	if(media_image->SaveSector(sparta_buffer, bitmap)!=media_image->GetSectorSize()) return false;
	a=media_image->ReadSector(sparta_buffer, 1);
	if(a!=SECTOR_SINGLE) return false;
	mask=*(sparta_buffer+13)+(*(sparta_buffer+14)<<8);
	mask+=diff;
	if(mask>(*(sparta_buffer+11)+(*(sparta_buffer+12)<<8))) return false;
	*(sparta_buffer+13)=(unsigned char)(mask&0xff);
	*(sparta_buffer+14)=(unsigned char)(mask>>8);
	if(media_image->SaveSector(sparta_buffer, 1)!=SECTOR_SINGLE) return false;
	return true;
}

bool SPARTA::PWD(char *result)
{
	strcpy(result, current_path);
	return true;
}

bool SPARTA::SetDiskLabel(char *label)
{
	int a;

	if(label==NULL) return false;
	if(media_image->ReadSector(sparta_buffer, 1)!=SECTOR_SINGLE) return false;
	a=strlen(label);
	if(a>8) a=8;
	memcpy(sparta_buffer+22, label, a);
	if(a<8)
	{
		while(a<8) *(sparta_buffer+22+a++)=' ';
	}
	if(media_image->SaveSector(sparta_buffer, 1)!=SECTOR_SINGLE) return false;
	return true;
}

bool SPARTA::ReadDiskLabel(char *result)
{
	int a;

	a=media_image->ReadSector(sparta_buffer, 1);
	if(a!=SECTOR_SINGLE) return false;
	strncpy(result, (char *)(sparta_buffer+22), 8);
	a=0;
	while((*(result+a)!=' ') && (a<8)) ++a;
	*(result+a)='\0';
	return true;
}

int SPARTA::GetFreeSectors(void)
{
	int ret;

	ret=media_image->ReadSector(sparta_buffer, 1);
	if(ret!=SECTOR_SINGLE) return -1;
	ret=*(sparta_buffer+13)+(*(sparta_buffer+14)<<8);
	return ret;
}

bool SPARTA::AddRowToDirectory(ATR_FILE *fileinfo, int dir_map)
{
	unsigned int a, b, c, d;
	unsigned char dir_row[DIR_ENTRY_SIZE];
	bool increase=false, loop=true;

	if(fileinfo==NULL) return false;
	dir_row[0]=fileinfo->Attributes;
	dir_row[1]=(unsigned char)(fileinfo->FirstSector&0xff);
	dir_row[2]=(unsigned char)(fileinfo->FirstSector>>8);
	dir_row[3]=(unsigned char)(fileinfo->SizeInBytes&0xff);
	dir_row[4]=(unsigned char)((fileinfo->SizeInBytes>>8)&0xff);
	dir_row[5]=(unsigned char)(fileinfo->SizeInBytes>>16);
	strcpy((char *)&dir_row[6], "           ");
	a=0;
	while(fileinfo->Name[a]!='\0') { dir_row[6+a]=fileinfo->Name[a]; ++a; }
	a=0;
	while(fileinfo->Ext[a]!='\0') { dir_row[14+a]=fileinfo->Ext[a]; ++a; }
	dir_row[17]=fileinfo->Day;
	dir_row[18]=fileinfo->Month;
	dir_row[19]=fileinfo->Year;
	dir_row[20]=fileinfo->Hour;
	dir_row[21]=fileinfo->Minute;
	dir_row[22]=fileinfo->Second;
	if(dir_sector_map==0)
	{
		media_image->ReadSector(sparta_buffer, 1);
		main_dir_sector_map=dir_sector_map=*(sparta_buffer+9)+(*(sparta_buffer+10)<<8);
	}
	dir_sector_map_cnt=0;
	current_dir_index=current_file*DIR_ENTRY_SIZE;
	b=media_image->GetSectorSize();
	a=d=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
	if(a==0) return false;
	if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
	current_dir_bsize=*(sparta_buffer+3)+(*(sparta_buffer+4)<<8)+(*(sparta_buffer+5)<<16);
	while(loop==true)
	{
		if((*(sparta_buffer+current_dir_index)&0x10)>0)
		{
			if((current_dir_index+DIR_ENTRY_SIZE)<b)
			{
				memcpy(sparta_buffer+current_dir_index, &dir_row[0], DIR_ENTRY_SIZE);
				if(b!=media_image->SaveSector(sparta_buffer, a)) return false;
				else return true;
			}
			else
			{
				c=b-current_dir_index;
				memcpy(sparta_buffer+current_dir_index, &dir_row[0], c);
				if(b!=media_image->SaveSector(sparta_buffer, a)) return false;
				a=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
				if(a==0) return false;
				if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
				memcpy(sparta_buffer, &dir_row[c], DIR_ENTRY_SIZE-c);
				if(b!=media_image->SaveSector(sparta_buffer, a)) return false;
				else return true;
			}
		}
		else if(*(sparta_buffer+current_dir_index)==0)
		{
			if((current_dir_index+DIR_ENTRY_SIZE)<b)
			{
				memcpy(sparta_buffer+current_dir_index, &dir_row[0], DIR_ENTRY_SIZE);
				if(b!=media_image->SaveSector(sparta_buffer, a)) return false;
				else
				{
					loop=false;
					increase=true;
					break;
				}
			}
			else
			{
				c=b-current_dir_index;
				memcpy(sparta_buffer+current_dir_index, &dir_row[0], c);
				if(b!=media_image->SaveSector(sparta_buffer, a)) return false;
				if(main_dir_sector_map==dir_sector_map) a=FindFreeDirSector();
				else a=FindFreeSector();
				if(a<1) return false;
				if(AllocateSector(dir_sector_map, a)==false) return false;
				SetSectorState(a, SECTOR_FILL);
				memset(sparta_buffer, 0, SECTOR_DOUBLE);
				memcpy(sparta_buffer, &dir_row[c], DIR_ENTRY_SIZE-c);
				if(b!=media_image->SaveSector(sparta_buffer, a)) return false;
				else
				{
					loop=false;
					increase=true;
					break;
				}
			}
		}
		if((current_dir_index+DIR_ENTRY_SIZE)>=b)
		{
			++dir_sector_map_cnt;
			a=FindNextFileSector(dir_sector_map, &dir_sector_map_cnt);
			if(a==0) return false;
			if(b!=media_image->ReadSector(sparta_buffer, a)) return false;
			current_dir_index-=b;
		}
		current_dir_index+=DIR_ENTRY_SIZE;
	}
	if(increase==true)
	{
		current_dir_bsize+=DIR_ENTRY_SIZE;
		media_image->ReadSector(sparta_buffer, d);
		sparta_buffer[3]=(unsigned char)(current_dir_bsize&0xff);
		sparta_buffer[4]=(unsigned char)((current_dir_bsize>>8)&0xff);
		sparta_buffer[5]=(unsigned char)(current_dir_bsize>>16);
		media_image->SaveSector(sparta_buffer, d);
	}
	return true;
}

bool SPARTA::AllocateSector(unsigned int sector_map, unsigned int new_sector)
{
	int a;
	unsigned int sector, next_sector;

	if(media_image->ReadSector(sparta_buffer, sector_map)!=media_image->GetSectorSize()) return false;
	next_sector=sector_map;
	sector=*(sparta_buffer)+(*(sparta_buffer+1)<<8);
	while(sector!=0)
	{
		if(media_image->ReadSector(sparta_buffer, sector)!=media_image->GetSectorSize()) return false;
		next_sector=sector;
		sector=*(sparta_buffer)+(*(sparta_buffer+1)<<8);
	}
	if(next_sector!=0) sector=next_sector;
	a=media_image->GetSectorSize()-2;
	a=4;
	while(((sparta_buffer[a]!=0) || (sparta_buffer[a+1]!=0)) && (a<=media_image->GetSectorSize())) a+=2;
	if(a>=media_image->GetSectorSize())
	{
		next_sector=FindFreeSector();
		SetSectorState(next_sector, SECTOR_FILL);
		if(media_image->ReadSector(sparta_buffer, sector)!=media_image->GetSectorSize()) return false;
		sparta_buffer[0]=(unsigned char)(next_sector&0xff);
		sparta_buffer[1]=(unsigned char)(next_sector>>8);
		if(media_image->SaveSector(sparta_buffer, sector)!=media_image->GetSectorSize()) return false;
		memset(sparta_buffer, 0, SECTOR_DOUBLE);
		sparta_buffer[2]=(unsigned char)(sector&0xff);
		sparta_buffer[3]=(unsigned char)(sector>>8);
		sector=next_sector;
		a=4;
	}
	sparta_buffer[a]=(unsigned char)(new_sector&0xff);
	sparta_buffer[a+1]=(unsigned char)(new_sector>>8);
	if(media_image->SaveSector(sparta_buffer, sector)!=media_image->GetSectorSize()) return false;
	return true;
}

bool SPARTA::IncSeqId(void)
{
	unsigned char a;

	if(media_image->ReadSector(sparta_buffer, 1)!=SECTOR_SINGLE) return false;
	a=*(sparta_buffer+38);
	++a;
	*(sparta_buffer+38)=a;
	if(media_image->SaveSector(sparta_buffer, 1)!=SECTOR_SINGLE) return false;
	return true;
}

char *SPARTA::DottedFileName(unsigned char *dir_entry)
{
	int a, b;
	char *filename;

	filename=new char[MAX_FILENAME_LENGTH];
	for(a=0;a<8;a++)
	{
		if(dir_entry[6+a]==' ') break;
		*(filename+a)=dir_entry[6+a];
	}
	*(filename+a++)='.';
	for(b=0;b<3;b++)
	{
		if(dir_entry[14+b]==' ') break;
		*(filename+a+b)=dir_entry[14+b];
	}
	*(filename+a+b)='\0';
	return filename;
}

bool SPARTA::DumpSector(int sector)
{
	if((sector<1) || (sector>media_image->GetTotalSectors())) return false;
	return media_image->DumpSector(sector);
}

char SPARTA::GetFSType(void)
{
	return FSTYPE_SPARTA2;
}

int SPARTA::GetSectorSize(void)
{
	return media_image->GetSectorSize();
}

int SPARTA::GetTotalSectors(void)
{
	return media_image->GetTotalSectors();
}

int SPARTA::GetDensity(void)
{
	return media_image->GetDensity();
}

int SPARTA::GetImageType(void)
{
	return media_image->GetImageType();
}

SPARTA::~SPARTA(void)
{
	delete[] sparta_buffer;
	if(current_path!=NULL) delete[] current_path;
}
