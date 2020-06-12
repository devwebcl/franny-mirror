/* Franny 1.0
 * Atari DOS II disk format routines
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

#include "atari_ii.h"

using namespace std;
using namespace types;
using namespace atariii;

ATARI_II::ATARI_II(void)
{
	media_image=NULL;
	strcpy(current_dir, "root");
	strcpy(file_mask, "*.*");
	strcpy(disk_label, "AtariDOS");
	atari_ii_buffer=new unsigned char[SECTOR_DOUBLE];
	force_atari_ii=set_mask=false;
	root_dir_sector=first_dir_sector=0x169;
	dir_length=8;
}

ATARI_II::ATARI_II(IMAGE *media)
{
	media_image=media;
	strcpy(current_dir, "root");
	strcpy(file_mask, "*.*");
	strcpy(disk_label, "AtariDOS");
	atari_ii_buffer=new unsigned char[SECTOR_DOUBLE];
	force_atari_ii=set_mask=false;
	root_dir_sector=first_dir_sector=0x169;
	dir_length=8;
}

bool ATARI_II::Open(IMAGE *media)
{
	media_image=media;
	force_atari_ii=set_mask=false;
	root_dir_sector=first_dir_sector=0x169;
	return true;
}

bool ATARI_II::Format(void)
{
	unsigned int total_sectors, bps, free_sectors, tfree_sectors, den;

	bps=media_image->GetSectorSize();
	total_sectors=media_image->GetTotalSectors();
	if(bps==SECTOR_DOUBLE)
	{
		if(total_sectors==720)
		{
			free_sectors=tfree_sectors=707;
			den=DENSITY_DOUBLE;
		}
		else return false;
	}
	else if(bps==SECTOR_SINGLE)
	{
		if(total_sectors==720)
		{
			free_sectors=tfree_sectors=707;
			den=DENSITY_SINGLE;
		}
		else if(total_sectors==1040)
		{
			free_sectors=707;
			tfree_sectors=1010;
			den=DENSITY_MEDIUM;
		}
		else return false;
	}
	else return false;
	memset(atari_ii_buffer, 0, SECTOR_DOUBLE);
	for(bps=0;bps<8;bps++) media_image->SaveSector(atari_ii_buffer, 361+bps);
	*(atari_ii_buffer)=2;
	*(atari_ii_buffer+1)=(unsigned char)(tfree_sectors&0xff);
	*(atari_ii_buffer+2)=(unsigned char)((tfree_sectors&0xff00)>>8);
	*(atari_ii_buffer+3)=(unsigned char)(free_sectors&0xff);
	*(atari_ii_buffer+4)=(unsigned char)((free_sectors&0xff00)>>8);
	for(bps=11;bps<100;bps++) *(atari_ii_buffer+bps)=0xff;
	*(atari_ii_buffer+0x0a)=0x0f;
	*(atari_ii_buffer+0x37)=0;
	*(atari_ii_buffer+0x38)=0x7f;
	media_image->SaveSector(atari_ii_buffer, 0x168);
	if(den==DENSITY_MEDIUM)
	{
		memset(atari_ii_buffer, 0xff, SECTOR_SINGLE);
		*(atari_ii_buffer+0x27)=0;
		*(atari_ii_buffer+0x28)=0x7f;
		*(atari_ii_buffer+0x7a)=0x2f;
		*(atari_ii_buffer+0x7b)=1;
		memset(atari_ii_buffer+0x7c, 0, 4);
		media_image->SaveSector(atari_ii_buffer, 0x400);
	}
	return true;
}

bool ATARI_II::CheckFS(void)
{
	media_image->ReadSector(atari_ii_buffer, 0x168);
	if(*(atari_ii_buffer)==0x02)
	{
		return true;
	}
	return false;
}

bool ATARI_II::FindFirstFile(ATR_FILE *file, char *mask)
{
	unsigned int a, b, c;
	char filename[12];

	memset(file, 0, sizeof(ATR_FILE));
	if((!force_atari_ii) && (!CheckFS())) return false;
	current_file=0;
	strcpy(file_mask, mask);
	set_mask=true;
	for(a=first_dir_sector;a<(first_dir_sector+dir_length);a++)
	{
		b=media_image->ReadSector(atari_ii_buffer, a);
		if((b!=SECTOR_SINGLE) && (b!=SECTOR_DOUBLE)) return false;
		for(b=0;b<8;b++)
		{
			++current_file;
			if(*(atari_ii_buffer+(b<<4))==0x80) continue;
			if(*(atari_ii_buffer+(b<<4))==0) return false;
			for(c=0;c<8;c++)
			{
				if(*(atari_ii_buffer+(b<<4)+5+c)==' ') break;
				*(file->Name+c)=*(atari_ii_buffer+(b<<4)+5+c);
			}
			*(file->Name+c)='\0';
			for(c=0;c<3;c++)
			{
				if (*(atari_ii_buffer+(b<<4)+5+8+c)==' ') break;
				*(file->Ext+c)=*(atari_ii_buffer+(b<<4)+5+8+c);
			}
			*(file->Ext+c)='\0';
			strcpy(filename, file->Name);
			strcat(filename, ".");
			strcat(filename, file->Ext);
			if(!CheckFileName(filename, mask)) continue;
			file->Attributes=*(atari_ii_buffer+(b<<4));
			file->SizeInSectors=*(atari_ii_buffer+1+(b<<4))+(*(atari_ii_buffer+2+(b<<4))<<8);
			file->FirstSector=*(atari_ii_buffer+3+(b<<4))+(*(atari_ii_buffer+4+(b<<4))<<8);
			file->FileId=current_file-1;
			c=*(atari_ii_buffer+3+(b<<4))+(*(atari_ii_buffer+4+(b<<4))<<8);
			file->SizeInBytes=FileSize(c, file->FileId);
			if((file->SizeInSectors!=0) && (file->SizeInBytes==0)) return false;
			return true;
		}
	}
	return false;
}

bool ATARI_II::FindNextFile(ATR_FILE *file)
{
	int a, b, c, d, e;
	char filename[12];

	if(((!force_atari_ii) && (!CheckFS())) || (!set_mask)) return false;
	if(current_file>=64) return false;
	memset(file, 0, sizeof(ATR_FILE));
	a=(current_file>>3)+first_dir_sector;
	for(d=a;d<(first_dir_sector+dir_length);d++)
	{
		b=media_image->ReadSector(atari_ii_buffer, d);
		if((b!=SECTOR_SINGLE) && (b!=SECTOR_DOUBLE)) return false;
		c=current_file&0x07;
		for(b=c;b<8;b++)
		{
			++current_file;
			if(*(atari_ii_buffer+(b<<4))==0x80) continue;
			if(*(atari_ii_buffer+(b<<4))==0) return false;
			for(e=0;e<8;e++)
			{
				if(*(atari_ii_buffer+(b<<4)+5+e)==' ') break;
				*(file->Name+e)=*(atari_ii_buffer+(b<<4)+5+e);
			}
			*(file->Name+e)='\0';
			for(e=0;e<3;e++)
			{
				if(*(atari_ii_buffer+(b<<4)+5+8+e)==' ') break;
				*(file->Ext+e)=*(atari_ii_buffer+(b<<4)+5+8+e);
			}
			*(file->Ext+e)='\0';
			strcpy(filename, file->Name);
			strcat(filename, ".");
			strcat(filename, file->Ext);
			if(!CheckFileName(filename, file_mask)) continue;
			file->Attributes=*(atari_ii_buffer+(b<<4));
			file->SizeInSectors=*(atari_ii_buffer+1+(b<<4))+(*(atari_ii_buffer+2+(b<<4))<<8);
			file->FirstSector=*(atari_ii_buffer+3+(b<<4))+(*(atari_ii_buffer+4+(b<<4))<<8);
			file->FileId=current_file-1;
			e=*(atari_ii_buffer+3+(b<<4))+(*(atari_ii_buffer+4+(b<<4))<<8);
			file->SizeInBytes=FileSize(e, file->FileId);
			if ((file->SizeInSectors!=0) && (file->SizeInBytes==0)) return false;
			return true;
		}
	}
	return false;
}


int ATARI_II::FileSize(unsigned int sector, unsigned char file_id)
{
	int len=0, a;

	while(sector)
	{
		a=media_image->ReadSector(atari_ii_buffer, sector);
		sector=*(atari_ii_buffer+a-2)+((*(atari_ii_buffer+a-3)&0x03)<<8);
		len+=*(atari_ii_buffer+a-1);
		if((*(atari_ii_buffer+a-3)>>2)!=file_id) { sector=0; len=-1; }
	}
	return len;
}

bool ATARI_II::CheckFileName(char *filename, char *mask)
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

bool ATARI_II::SetSectorState(unsigned int sector, bool status)
{
	unsigned int index, free_sectors;

	if(sector>720) return false;
	media_image->ReadSector(atari_ii_buffer, 0x168);
	if(sector==0x2d0) sector=0;
	index=(sector>>3)+10;
	free_sectors=*(atari_ii_buffer+3)+(*(atari_ii_buffer+4)<<8);
	if(status==SECTOR_EMPTY)
	{
		free_sectors++;
		*(atari_ii_buffer+index)=*(atari_ii_buffer+index)|ATARI_II_BIT_MASKS[sector&0x07];
	}
	else
	{
		--free_sectors;
		*(atari_ii_buffer+index)=*(atari_ii_buffer+index)&ATARI_II_BIT_MASKS2[sector&0x07];
	}
	*(atari_ii_buffer+3)=(unsigned char)(free_sectors&0xff);
	*(atari_ii_buffer+4)=(unsigned char)(free_sectors>>8);
	media_image->SaveSector(atari_ii_buffer, 0x168);
	return true;
}

int ATARI_II::FindFreeSector(void)
{
	unsigned char a, b;
	unsigned int val;
	
	media_image->ReadSector(atari_ii_buffer, 0x168);
	for(a=10;a<100;a++)
	{
		if(*(atari_ii_buffer+a))
		{
			for(b=0;b<8;b++)
			{
				if(*(atari_ii_buffer+a)&ATARI_II_BIT_MASKS[b])
				{
					val=((a-10)<<3)+b;
					if(val==0) val=0x2d0;
					return val;
				}
			}
		}
	}
	return -1;
}

int ATARI_II::LoadFile(unsigned char **buffer, char *fname)
{
	ATR_FILE file;
	unsigned int a, sector, index;
	unsigned char file_id;

	if(!FindFirstFile_Private(&file, fname)) return -1;
	file_id=file.FileId;
	sector=file.FirstSector;
	index=0;
	*buffer=new unsigned char[file.SizeInBytes];
	while(sector)
	{
		a=media_image->ReadSector(atari_ii_buffer, sector);
		if((*(atari_ii_buffer+a-3)>>2)!=file_id) return -2;
		sector=*(atari_ii_buffer+a-2)+((*(atari_ii_buffer+a-3)&0x03)<<8);
		if(sector) { memcpy(*buffer+index, atari_ii_buffer, a-3); index+=(a-3); }
		else { memcpy(*buffer+index, atari_ii_buffer, *(atari_ii_buffer+a-1)); index+=*(atari_ii_buffer+a-1); }
	}
	return index;
}

int ATARI_II::SaveFile(unsigned char *buffer, char *fname, unsigned int size)
{
	ATR_FILE file;
	int file_id;
	unsigned int saved_size, sector, next_sector, sector_size, file_size, first_sector;
	char *temp_name;

	if(size==0) return 0;
	if(FindFirstFile_Private(&file, fname)) return -2;
	if((GetFreeSectors()*(GetSectorSize()-3))<=size) return -7;
	file_id=FindFreeFileId();
	if(file_id<0) return -1;
	saved_size=file_size=0;
	sector_size=media_image->GetSectorSize()-3;
	first_sector=sector=FindFreeSector();
	while(size>saved_size)
	{
		if(SetSectorState(sector, SECTOR_FILL)==false) return -3;
		if(size>(saved_size+sector_size))
		{
			next_sector=FindFreeSector();
			memcpy(atari_ii_buffer, buffer+saved_size, sector_size);
			*(atari_ii_buffer+media_image->GetSectorSize()-3)=(file_id<<2)|((next_sector&0x3ff)>>8);
			*(atari_ii_buffer+media_image->GetSectorSize()-2)=next_sector&0xff;
			(media_image->GetSectorSize()==SECTOR_SINGLE)?*(atari_ii_buffer+media_image->GetSectorSize()-1)=0x7d:*(atari_ii_buffer+media_image->GetSectorSize()-1)=0xfd;
			saved_size+=sector_size;
		}
		else
		{
			memset(atari_ii_buffer, 0, media_image->GetSectorSize());
			memcpy(atari_ii_buffer, buffer+saved_size, size-saved_size);
			*(atari_ii_buffer+media_image->GetSectorSize()-3)=file_id<<2;
			*(atari_ii_buffer+media_image->GetSectorSize()-1)=size-saved_size;
			saved_size+=(size-saved_size);
		}
		media_image->SaveSector(atari_ii_buffer, sector);
		++file_size;
		sector=next_sector;
	}
	if(!media_image->ReadSector(atari_ii_buffer, first_dir_sector+(file_id>>3))) return -5;
	temp_name=PrepareName(fname);
	if(temp_name==NULL) return -4;
	next_sector=(file_id&7)<<4;
	*(atari_ii_buffer+next_sector)=0x42;
	*(atari_ii_buffer+next_sector+1)=(unsigned char)(file_size&0xff);
	*(atari_ii_buffer+next_sector+2)=(unsigned char)(file_size>>8);
	*(atari_ii_buffer+next_sector+3)=(unsigned char)(first_sector&0xff);
	*(atari_ii_buffer+next_sector+4)=(unsigned char)(first_sector>>8);
	memcpy(atari_ii_buffer+next_sector+5, temp_name, 11);
	delete[] temp_name;
	if(!media_image->SaveSector(atari_ii_buffer, first_dir_sector+(file_id>>3))) return -6;
	return 0;
}

char * ATARI_II::PrepareName(char *name)
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

char ATARI_II::FindFreeFileId(void)
{
	unsigned int sector;
	unsigned char a, b;

	sector=first_dir_sector;
	for(a=0;a<8;a++)
	{
		if(media_image->ReadSector(atari_ii_buffer, sector+a)==0) return -2;
		for(b=0;b<8;b++)
		{
			if((*(atari_ii_buffer+(b<<4))==0x80) || (*(atari_ii_buffer+(b<<4))==0))
			{
				return (a<<3)+b;
			}
		}
	}
	return -1;
}

bool ATARI_II::DeleteFile(char *filename, bool clear_sector)
{
	ATR_FILE file;
	int sector, next_sector, a, file_id;

	if(!FindFirstFile_Private(&file, filename)) return false;
	file_id=file.FileId;
	sector=file.FirstSector;
	while(sector)
	{
		a=media_image->ReadSector(atari_ii_buffer, sector);
		next_sector=*(atari_ii_buffer+a-2)+((*(atari_ii_buffer+a-3)&0x03)<<8);
		if((*(atari_ii_buffer+a-3)>>2)!=file_id) return false;
		SetSectorState(sector, SECTOR_EMPTY);
		if(clear_sector==true)
		{
			memset(atari_ii_buffer, 0, SECTOR_DOUBLE);
			media_image->SaveSector(atari_ii_buffer, sector);
		}
		sector=next_sector;
	}
	media_image->ReadSector(atari_ii_buffer, first_dir_sector+(file_id>>3));
	*(atari_ii_buffer+((file_id&7)<<4))=0x80;
	media_image->SaveSector(atari_ii_buffer, first_dir_sector+(file_id>>3));
	return true;
}

bool ATARI_II::FindFirstFile_Private(ATR_FILE *file, char *mask)
{
	unsigned int a, b, c;
	char filename[12];

	memset(file, 0, sizeof(ATR_FILE));
	if((!force_atari_ii) && (!CheckFS())) return false;
	current_file=0;
	for(a=first_dir_sector;a<(first_dir_sector+dir_length);a++)
	{
		b=media_image->ReadSector(atari_ii_buffer, a);
		if((b!=SECTOR_SINGLE) && (b!=SECTOR_DOUBLE)) return false;
		for(b=0;b<8;b++)
		{
			++current_file;
			if(*(atari_ii_buffer+(b<<4))==0x80) continue;
			if(*(atari_ii_buffer+(b<<4))==0) return false;
			for(c=0;c<8;c++)
			{
				if(*(atari_ii_buffer+(b<<4)+5+c)==' ') break;
				*(file->Name+c)=*(atari_ii_buffer+(b<<4)+5+c);
			}
			*(file->Name+c)='\0';
			for(c=0;c<3;c++)
			{
				if (*(atari_ii_buffer+(b<<4)+5+8+c)==' ') break;
				*(file->Ext+c)=*(atari_ii_buffer+(b<<4)+5+8+c);
			}
			*(file->Ext+c)='\0';
			strcpy(filename, file->Name);
			strcat(filename, ".");
			strcat(filename, file->Ext);
			if(!CheckFileName(filename, mask)) continue;
			file->Attributes=*(atari_ii_buffer+(b<<4));
			file->SizeInSectors=*(atari_ii_buffer+1+(b<<4))+(*(atari_ii_buffer+2+(b<<4))<<8);
			file->FirstSector=*(atari_ii_buffer+3+(b<<4))+(*(atari_ii_buffer+4+(b<<4))<<8);
			file->FileId=current_file-1;
			c=*(atari_ii_buffer+3+(b<<4))+(*(atari_ii_buffer+4+(b<<4))<<8);
			file->SizeInBytes=FileSize(c, file->FileId);
			if((file->SizeInSectors!=0) && (file->SizeInBytes==0)) return false;
			return true;
		}
	}
	return false;
}

bool ATARI_II::CD(char *path)
{
	return false;
}

bool ATARI_II::PWD(char *path)
{
	strcpy(path, current_dir);
	return true;
}

bool ATARI_II::SetDiskLabel(char *name)
{
	return false;
}

bool ATARI_II::ReadDiskLabel(char *name)
{
	strcpy(name, disk_label);
	return true;
}

int ATARI_II::GetFreeSectors(void)
{
	int ret, a;

	ret=media_image->ReadSector(atari_ii_buffer, 0x168);
	if((ret!=SECTOR_SINGLE) && (ret!=SECTOR_DOUBLE)) return -1;
	ret=*(atari_ii_buffer+3)+(*(atari_ii_buffer+4)<<8);
	if(media_image->GetDensity()==DENSITY_MEDIUM)
	{
		a=media_image->ReadSector(atari_ii_buffer, 0x400);
		if((a!=SECTOR_SINGLE) && (a!=SECTOR_DOUBLE)) return -2;
		ret+=*(atari_ii_buffer+0x7a)+(*(atari_ii_buffer+0x7b)<<8);
	}
	return ret;
}

char ATARI_II::GetFSType(void)
{
	return FSTYPE_DOS2;
}

bool ATARI_II::MakeDir(char *)
{
	return false;
}

bool ATARI_II::DeleteDir(char *)
{
	return false;
}

int ATARI_II::GetSectorSize(void)
{
	return media_image->GetSectorSize();
}

int ATARI_II::GetTotalSectors(void)
{
	return media_image->GetTotalSectors();
}

int ATARI_II::GetDensity(void)
{
	return media_image->GetDensity();
}

int ATARI_II::GetImageType(void)
{
	return media_image->GetImageType();
}

ATARI_II::~ATARI_II(void)
{
	delete[] atari_ii_buffer;
}
