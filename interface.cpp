/* Franny 1.0
 * Interface 'images<->user's application' routines
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

#include "interface.h"

using namespace image;
using namespace atr;
using namespace raw;
using namespace fs;
using namespace atariii;
using namespace sparta;
using namespace types;
using namespace interface;

const char *DATE_SEPARATOR="-";
const char *TIME_SEPARATOR=":";
const char *DEFAULT_LIST_FORMAT="A F.E S B D T";
const char *DIR_DELIM=">/\\";

INTERFACE::INTERFACE(void)
{
	media_image=NULL;
	filesystem_image=NULL;
	image_filename=NULL;
	return;
}

INTERFACE::INTERFACE(char *filename, char media_type, char fs_type)
{
	image_filename=new char[strlen(filename)+1];
	strcpy(image_filename, filename);
	try {
		if(media_type==MEDIA_TYPE_UNKNOWN) media_image=DetectMedia();
		else media_image=AllocMedia(media_type);
		if(fs_type==FSTYPE_UNKNOWN) filesystem_image=DetectFS(media_image);
		else filesystem_image=AllocFS(media_image, fs_type);
	}
	catch(...)
	{
		throw exception();
	}
	return;
}

INTERFACE::~INTERFACE(void)
{
	if(media_image!=NULL) delete media_image;
	if(filesystem_image!=NULL) delete filesystem_image;
	if(image_filename!=NULL) delete[] image_filename;
	return;
}

bool INTERFACE::ShowName(ATR_FILE *file, bool align)
{
	int a, b;

	if(file==NULL) return false;
	cout<<file->Name;
	if(align==true)
	{
		a=strlen(file->Name);
		for(b=a;b<8;b++) cout<<" ";
	}
	return true;
}

bool INTERFACE::ShowExt(ATR_FILE *file, bool align)
{
	int a, b;

	if(file==NULL) return false;
	cout<<file->Ext;
	if(align==true)
	{
		a=strlen(file->Ext);
		for(b=a;b<3;b++) cout<<" ";
	}
	return true;
}

bool INTERFACE::ShowSizeInBytes(ATR_FILE *file)
{
	if(file==NULL) return false;
	if(file->SizeInBytes>99999) cout<<file->SizeInBytes;
	else if(file->SizeInBytes>9999) cout<<" "<<file->SizeInBytes;
	else if(file->SizeInBytes>999) cout<<"  "<<file->SizeInBytes;
	else if(file->SizeInBytes>99) cout<<"   "<<file->SizeInBytes;
	else if(file->SizeInBytes>9) cout<<"    "<<file->SizeInBytes;
	else cout<<"      "<<file->SizeInBytes;
	return true;
}

bool INTERFACE::ShowSizeInSectors(ATR_FILE *file)
{
	if(file==NULL) return false;
	if(file->SizeInSectors>999) cout<<file->SizeInSectors;
	else if(file->SizeInSectors>99) cout<<" "<<file->SizeInSectors;
	else if(file->SizeInSectors>9) cout<<"  "<<file->SizeInSectors;
	else cout<<"   "<<file->SizeInSectors;
	return true;
}

bool INTERFACE::ShowAttributes(ATR_FILE *file)
{
	if(file==NULL) return false;
	switch(filesystem_image->GetFSType())
	{
		case FSTYPE_DOS2:
			((file->Attributes&0x20)>0)?cout<<"R":cout<<"-";
			cout<<"--";
			break;
		case FSTYPE_SPARTA2:
			((file->Attributes&0x01)>0)?cout<<"R":cout<<"-";
			((file->Attributes&0x02)>0)?cout<<"H":cout<<"-";
			((file->Attributes&0x20)>0)?cout<<"D":cout<<"-";
			break;
		default:
			cout<<"---"<<endl;
			break;
	}
	return true;
}

bool INTERFACE::ShowDate(ATR_FILE *file)
{
	if(file==NULL) return false;
	(file->Day>9)?cout<<(int)file->Day:cout<<"0"<<(int)file->Day;
	cout<<DATE_SEPARATOR;
	(file->Month>9)?cout<<(int)file->Month:cout<<"0"<<(int)file->Month;
	cout<<DATE_SEPARATOR;
	cout<<(int)(file->Year+2000);
	return true;
}

bool INTERFACE::ShowTime(ATR_FILE *file)
{
	if(file==NULL) return false;
	(file->Hour>9)?cout<<(int)file->Hour:cout<<"0"<<(int)file->Hour;
	cout<<TIME_SEPARATOR;
	(file->Minute>9)?cout<<(int)file->Minute:cout<<"0"<<(int)file->Minute;
	cout<<TIME_SEPARATOR;
	(file->Second>9)?cout<<(int)file->Second:cout<<"0"<<(int)file->Second;
	return true;
}

bool INTERFACE::UpString(char *string)
{
	int a=0;

	if(string==NULL) return false;
	while(*(string+a)!='\0')
	{
		if((*(string+a)>='a') && (*(string+a)<='z')) *(string+a)-=0x20;
		++a;
	}
	return true;
}

int INTERFACE::FindChar(char *string, char what)
{
	int a=0;
	
	while(*(string+a)!='\0')
	{
		if(*(string+a)==what) return a;
		++a;
	}
	return -1;
}

bool INTERFACE::Create(char *filename, int sectors, int bps, int media_type, int fs_type)
{
	int density=DENSITY_UNKNOWN;

	if(filename==NULL) return false;
	if((sectors==720) && (bps==SECTOR_SINGLE)) density=DENSITY_SINGLE;
	else if((sectors==1040) && (bps==SECTOR_SINGLE)) density=DENSITY_MEDIUM;
	else if((sectors==720) && (bps==SECTOR_DOUBLE)) density=DENSITY_DOUBLE;
	try
	{
		media_image=AllocMedia(media_type);
		if(media_image->NewImage(filename, sectors, bps)==false) throw exception();
		filesystem_image=AllocFS(media_image, fs_type);
		if((media_image==NULL) || (filesystem_image==NULL)) throw exception();
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool INTERFACE::Format(void)
{
	if(filesystem_image==NULL) return false;
	return filesystem_image->Format();
}

bool INTERFACE::Open(char *filename, char media_type, char fs_type)
{
	if(image_filename!=NULL) delete[] image_filename;
	if(filesystem_image!=NULL) delete filesystem_image;
	if(media_image!=NULL) delete media_image;
	image_filename=new char[strlen(filename)+1];
	strcpy(image_filename, filename);
	if(media_type==MEDIA_TYPE_UNKNOWN) media_image=DetectMedia();
	else media_image=AllocMedia(media_type);
	if(media_image==NULL) return false;
	if(fs_type==FSTYPE_UNKNOWN) filesystem_image=DetectFS(media_image);
	else filesystem_image=AllocFS(media_image, fs_type);
	if(filesystem_image==NULL) return false;
	return true;
}

bool INTERFACE::List(char *file_mask, char *format)
{
	ATR_FILE file;
	int a, cnt=0;
	char *local_format;
	bool status=true;

	if(file_mask==NULL) return false;
	if(format==NULL) local_format=(char *)DEFAULT_LIST_FORMAT;
	else local_format=format;
	if(filesystem_image->FindFirstFile(&file, file_mask))
	{
		do {
			if(status==false) continue;
			for(a=0;a<strlen(local_format);a++)
			{
				switch(*(local_format+a))
				{
					case 'A':
					case 'a':
						ShowAttributes(&file);
						break;
					case 'B':
					case 'b':
						ShowSizeInBytes(&file);
						break;
					case 'D':
					case 'd':
						ShowDate(&file);
						break;
					case 'E':
					case 'e':
						if(*(local_format+a+1)=='+')
						{
							++a;
							ShowExt(&file, false);
						}
						else ShowExt(&file, true);
						break;
					case 'F':
					case 'f':
						if(*(local_format+a+1)=='+')
						{
							++a;
							ShowName(&file, false);
						}
						else ShowName(&file, true);
						break;
					case 'S':
					case 's':
						ShowSizeInSectors(&file);
						break;
					case 'T':
					case 't':
						ShowTime(&file);
						break;
					default:
						cout<<(char)*(local_format+a);
				}
			}
			cout<<endl;
			++cnt;
		} while(status=filesystem_image->FindNextFile(&file));
	}
	(cnt==1)?cout<<"Total: 1 file."<<endl:cout<<"Total: "<<cnt<<" files."<<endl;
	return true;
}

bool INTERFACE::DumpSector(int sector)
{
	return media_image->DumpSector(sector);
}

bool INTERFACE::DumpFile(char *src_filename, char *dest_filename)
{
	fstream *outfile;
	int size;
	unsigned char *buffer;
	char *temp_name;

	if((src_filename==NULL) || (dest_filename==NULL)) return false;
	if(UpString(src_filename)==false) return false;
	if(strchr(src_filename, '.')==NULL)
	{
		temp_name=new char[strlen(src_filename)+2];
		strcpy(temp_name, src_filename);
		strcat(temp_name, ".");
		size=filesystem_image->LoadFile(&buffer, temp_name);
		delete[] temp_name;
	}
	else size=filesystem_image->LoadFile(&buffer, src_filename);
	if(size<0) return false;
	outfile=new fstream(dest_filename, fstream::out);
	if(outfile->is_open()==false) { delete outfile; delete[] buffer; return false; }
	outfile->write((const char *)buffer, size);
	delete outfile;
	delete[] buffer;
	return true;
}

bool INTERFACE::InsertFile(char *src_filename, char *dest_filename)
{
	fstream *infile;
	int size;
	unsigned char *buffer;
	bool ret=true;
	char *temp_name;

	if((src_filename==NULL) || (dest_filename==NULL)) return false;
	if(UpString(dest_filename)==false) return false;
	if((*src_filename=='.') && ((*(src_filename+1)=='.') || (*(src_filename+1)=='\0'))) return false;
	infile=new fstream((const char *)src_filename, fstream::in);
	if(infile->is_open()==false) { delete infile; return false; }
	infile->seekg(0, fstream::end);
	size=infile->tellg();
	if(size==0) { delete infile; return false; }
	infile->seekg(0, fstream::beg);
	buffer=new unsigned char[size];
	infile->read((char *)buffer, size);
	delete infile;
	if(strchr(dest_filename, '.')==NULL)
	{
		temp_name=new char[strlen(dest_filename)+2];
		strcpy(temp_name, dest_filename);
		strcat(temp_name, ".");
		if(filesystem_image->SaveFile(buffer, temp_name, size)!=0) ret=false;
		delete[] temp_name;
	}
	else { if(filesystem_image->SaveFile(buffer, dest_filename, size)!=0) ret=false; }
	delete[] buffer;
	return ret;
}

bool INTERFACE::DeleteFile(char *filename)
{
	char *temp_name;
	bool result;

	if(filename==NULL) return false;
	if(strchr(filename, '.')==NULL)
	{
		temp_name=new char[strlen(filename)+2];
		strcpy(temp_name, filename);
		strcat(temp_name, ".");
		result=filesystem_image->DeleteFile(temp_name);
		delete[] temp_name;
	}
	else result=filesystem_image->DeleteFile(filename);
	return result;
}

bool INTERFACE::MakeDir(char *dirname)
{
	char *temp_name;
	bool result;

	if(dirname==NULL) return false;
	if(UpString(dirname)==false) return false;
	if(strchr(dirname, '.')==NULL)
	{
		temp_name=new char[strlen(dirname)+2];
		strcpy(temp_name, dirname);
		strcat(temp_name, ".");
		result=filesystem_image->MakeDir(temp_name);
		delete[] temp_name;
	}
	else result=filesystem_image->MakeDir(dirname);
	return result;
}

bool INTERFACE::RemoveDir(char *dirname)
{
	char *temp_name;
	bool result;

	if(dirname==NULL) return false;
	if(UpString(dirname)==false) return false;
	if(strchr(dirname, '.')==NULL)
	{
		temp_name=new char[strlen(dirname)+2];
		strcpy(temp_name, dirname);
		strcat(temp_name, ".");
		result=filesystem_image->DeleteDir(temp_name);
		delete[] temp_name;
	}
	else result=filesystem_image->DeleteDir(dirname);
	return result;
}

void INTERFACE::Status(void)
{
	char tmp[16];

	filesystem_image->ReadDiskLabel((char *)&tmp);
	cout<<"Disk label:       "<<tmp<<endl;
	cout<<"File system type: ";
	switch(filesystem_image->GetFSType())
	{
		case FSTYPE_DOS2:
			cout<<"Atari DOS II";
			break;
		case FSTYPE_SPARTA2:
			cout<<"Sparta 2";
			break;
		default:
			cout<<"unknown";
			break;
	}
	cout<<endl<<"Density:          ";
	switch(filesystem_image->GetDensity())
	{
		case DENSITY_SINGLE:
			cout<<"single";
			break;
		case DENSITY_MEDIUM:
			cout<<"medium";
			break;
		case DENSITY_DOUBLE:
			cout<<"double";
			break;
		case DENSITY_CUSTOM_128:
			cout<<"custom (128 bytes per sector)";
			break;
		case DENSITY_CUSTOM_256:
			cout<<"custom (256 bytes per sector)";
			break;
		default:
			cout<<"unknown";
			break;
	}
	cout<<endl<<"Total Sectors:    "<<filesystem_image->GetTotalSectors()<<" ("<<filesystem_image->GetSectorSize()*filesystem_image->GetTotalSectors()<<" bytes)"<<endl;
	if(filesystem_image->GetFSType()==FSTYPE_DOS2) cout<<"Free Sectors:     "<<filesystem_image->GetFreeSectors()<<" ("<<(filesystem_image->GetSectorSize()-3)*filesystem_image->GetFreeSectors()<<" bytes)"<<endl;
	else cout<<"Free Sectors:     "<<filesystem_image->GetFreeSectors()<<" ("<<filesystem_image->GetSectorSize()*filesystem_image->GetFreeSectors()<<" bytes)"<<endl;
	cout<<"Image Type:       ";
	switch(filesystem_image->GetImageType())
	{
		case IMAGE_STANDARD:
		    cout<<"standard";
		    break;
		case IMAGE_SIO2IDE:
		    cout<<"Sio2Ide";
		    break;
		default:
		    cout<<"unknown";
		    break;
	}
	cout<<endl;
	cout<<"Image Media:      "<<media_image->GetMediaType()<<endl;
	return;
}

bool INTERFACE::ChangeImageType(char type)
{
	if(media_image->SetImageType(type)==true) return true;
	return false;
}

int INTERFACE::ChangeDirectory(char *path)
{
	int a=0, b=0, c=0;
	char *tmp_name;

	if(path==NULL) return -1;
	tmp_name=new char[strlen(path)+1];
	if((*path==DIR_DELIM[0]) || (*path==DIR_DELIM[1]) || (*path==DIR_DELIM[2])) ++a;
	while(*(path+a)!='\0')
	{
		if((*(path+a)==DIR_DELIM[0]) || (*(path+a)==DIR_DELIM[1]) || (*(path+a)==DIR_DELIM[2]))
		{
			if(b>0)
			{
				tmp_name[b]='\0';
				if(strchr(tmp_name, '.')==NULL) strcat(tmp_name, ".");
				if(filesystem_image->CD(tmp_name)==false) { delete[] tmp_name; return -2; }
				b=0;
				++a;
				c=a;
				continue;
			}
			else return -3;
		}
		tmp_name[b]=*(path+a);
		++b;
		++a;
	}
	delete[] tmp_name;
	return c;
}

FS * INTERFACE::DetectFS(IMAGE *media)
{
	FS *result=NULL;
	unsigned char *sector;
	
	sector=new unsigned char[SECTOR_DOUBLE];
	if(media->ReadSector(sector, 0x168)!=0)
	{
		if(*sector==0x02) result=new ATARI_II(media);
		else
		{
			if(media->ReadSector(sector, 1)!=0)
			{
				if(*(sector+32)==0x20) result=new SPARTA(media);
			}
			else result=new FS(media);
		}
	}
	delete[] sector;
	return result;
}

FS * INTERFACE::AllocFS(IMAGE *media, char fs_type)
{
	FS *result=NULL;

	switch(fs_type)
	{
	case FSTYPE_DOS2:
		result=new ATARI_II(media);
		break;
	case FSTYPE_SPARTA2:
		result=new SPARTA(media);
		break;
	default:
		result=new FS(media);
		break;
	}
	return result;
}

IMAGE * INTERFACE::DetectMedia(void)
{
	IMAGE *result;
	ifstream *media;
	unsigned int size;
	unsigned char buffer[2];

	media=new ifstream(image_filename);
	media->seekg(0, ios::end);
	size=media->tellg();
	media->seekg(0, ios::beg);
	media->read((char *)buffer, 2);
	delete media;
	if((buffer[0]==0x96) && (buffer[1]==0x02)) result=new ATR(image_filename);
	else if((size&0x7f)==0) result=new RAW(image_filename);
	else result=new IMAGE(image_filename);
	return result;
}

IMAGE * INTERFACE::AllocMedia(char media_type)
{
	IMAGE *result;

	switch(media_type)
	{
	case MEDIA_TYPE_ATR:
		result=new ATR(image_filename);
		break;
	case MEDIA_TYPE_RAW:
		result=new RAW(image_filename);
		break;
	default:
		result=new IMAGE(image_filename);
		break;
	}
	return result;
}

bool INTERFACE::SetVolumeName(char *volume)
{
	if(volume==NULL) return false;
	return filesystem_image->SetDiskLabel(volume);
}

bool INTERFACE::ConvertImage(unsigned int media, char *new_image)
{
	IMAGE *local_image;
	unsigned int sectors;
	unsigned int bps;
	unsigned char sector_buffer[SECTOR_DOUBLE];

	if(media_image==NULL) return false;
	sectors=media_image->GetTotalSectors();
	bps=media_image->GetSectorSize();
	local_image=AllocMedia(media);
	if(local_image->NewImage(new_image, sectors, bps)==false)
	{
		delete local_image;
		return false;
	}
	for(int a=0;a<sectors;a++)
	{
		media_image->ReadSector(sector_buffer, a+1);
		local_image->SaveSector(sector_buffer, a+1);
	}
	local_image->CloseImage();
	delete local_image;
	return true;
}
