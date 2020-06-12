/* Franny 1.0
 * RAW disk access routines
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

#include "raw.h"

const char *IMAGE_MEDIA_RAW="RAW";

using namespace std;
using namespace types;
using namespace raw;

RAW::RAW(void)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image=NULL;
	total_sectors=0;
	raw_bios_buffer=new unsigned char[SECTOR_DOUBLE];
}

RAW::RAW(char *fname)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image=NULL;
	total_sectors=0;
	raw_bios_buffer=new unsigned char[SECTOR_DOUBLE];
	OpenImage(fname);
}

RAW::RAW(char *fname, unsigned int sectors, unsigned int bps)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image=NULL;
	total_sectors=sectors;
	raw_bios_buffer=new unsigned char[SECTOR_DOUBLE];
	NewImage(fname, sectors, bps);
}

bool RAW::OpenImage(char *fname)
{
	unsigned int image_size, a;

	if(image_load) CloseImage();
	try {
		image=new fstream(fname, fstream::in|fstream::out|fstream::binary);
	}
	catch(...)
	{ return false; }
	if(image->is_open()!=true) return false;
	image->seekg(0, ios::end);
	image_size=image->tellg();
	image->seekg(0, ios::beg);
	if(((image_size+SIZE_SUB)&0xff)==0)
	{
		bytes_per_sector=SECTOR_DOUBLE;
		boot_sector_size=SECTOR_SINGLE;
		total_sectors=(image_size+SIZE_SUB)>>8;
		(total_sectors==720)?density=DENSITY_DOUBLE:density=DENSITY_CUSTOM_256;
	}
	else if(((image_size&0xff)==0) && (image_size!=SIZE_MEDIUM) && (image_size!=SIZE_SINGLE))
	{
		bytes_per_sector=SECTOR_DOUBLE;
		boot_sector_size=SECTOR_DOUBLE;
		total_sectors=image_size>>8;
		(total_sectors==720)?density=DENSITY_DOUBLE:density=DENSITY_CUSTOM_256;
	}
	else if((image_size&0x7f)==0)
	{
		bytes_per_sector=SECTOR_SINGLE;
		boot_sector_size=SECTOR_SINGLE;
		total_sectors=image_size>>7;
		(total_sectors==720)?density=DENSITY_SINGLE:((total_sectors==1040)?density=DENSITY_MEDIUM:density=DENSITY_CUSTOM_128);
	}
	else
	{
		delete image;
		image_load=false;
		return false;
	}
	image_load=true;
	return true;
}

inline bool RAW::SetForceWrite(bool force)
{
	force_write=force;
	return true;
}

unsigned int RAW::ReadSector(unsigned char *buffer, unsigned int sector)
{
	unsigned int pos;

	if((image_load==false) || (sector==0) || (sector>total_sectors)) return 0;
	--sector;
	if(sector<3) pos=sector*boot_sector_size;
	else pos=3*boot_sector_size+(sector-3)*bytes_per_sector;
	image->seekg(pos, ios::beg);
	image->read((char *)buffer, bytes_per_sector);
	if(sector<3) return boot_sector_size;
	return bytes_per_sector;
}

unsigned int RAW::SaveSector(unsigned char *buffer, unsigned int sector)
{
	unsigned int pos;

	if((image_load==false) || (sector==0) || (sector>total_sectors)) return 0;
	--sector;
	if(sector<3) pos=sector*boot_sector_size;
	else pos=3*boot_sector_size+(sector-3)*bytes_per_sector;
	image->seekg(pos, ios::beg);
	image->write((char *)buffer, bytes_per_sector);
	if(sector<3) return boot_sector_size;
	return bytes_per_sector;
}

bool RAW::CloseImage(void)
{
	if(image_load==false) return false;
	delete image;
	image=NULL;
	image_load=false;
	total_sectors=0;
	return true;
}

bool RAW::NewImage(char *fname, unsigned int sectors, unsigned int bps)
{
	unsigned int len, a;

	if(image_load) CloseImage();
	if((bps!=SECTOR_SINGLE) && (bps!=SECTOR_DOUBLE)) return false;
	if(sectors>MAX_SECTORS) return false;
	try {
		image=new fstream(fname, fstream::out|fstream::in|fstream::trunc|fstream::binary);
	}
	catch(...)
	{
		throw exception();
	}
	memset(raw_bios_buffer, 0, bps);
	for(a=0;a<sectors;a++)
		{ (a<3)?image->write((char *)raw_bios_buffer, SECTOR_SINGLE):image->write((char *)raw_bios_buffer, bps); }
	image_load=true;
	corrupted=false;
	bytes_per_sector=bps;
	total_sectors=sectors;
	boot_sector_size=SECTOR_SINGLE;
	if(sectors==720) (bps==SECTOR_DOUBLE)?density=DENSITY_DOUBLE:density=DENSITY_SINGLE;
	else if((sectors==1040) && (bps==SECTOR_SINGLE)) density=DENSITY_MEDIUM;
	else (bps==SECTOR_DOUBLE)?density=DENSITY_CUSTOM_256:density=DENSITY_CUSTOM_128;
	return true;
}

unsigned char RAW::GetDensity(void)
{
	return density;
}

unsigned int RAW::GetSectorSize(void)
{
	return bytes_per_sector;
}

unsigned int RAW::GetTotalSectors(void)
{
	return total_sectors;
}

unsigned char RAW::GetImageType(void)
{
	return IMAGE_STANDARD;
}

bool RAW::SetImageType(unsigned char type)
{
	return false;
}

bool RAW::SetCorrupted(bool status)
{
	corrupted=status;
	return true;
}

bool RAW::DumpSector(unsigned int sector)
{
	unsigned int a, b;
	char sector_line[17];

	if((image_load==false) || (sector==0) || (sector>total_sectors)) return false;
	((sector>3) && (bytes_per_sector==SECTOR_DOUBLE))?a=SECTOR_DOUBLE:a=SECTOR_SINGLE;
	if(ReadSector(raw_bios_buffer, sector)>0)
	{
		cout<<hex;
		sector_line[16]='\0';
		for(b=0;b<a;b++)
		{
			if(!(b&0x0f)) (b<0x0f)?cout<<"0"<<b<<":":cout<<b<<":";
			(*(raw_bios_buffer+b)>=0x20)?sector_line[b&0x0f]=*(raw_bios_buffer+b):sector_line[b&0x0f]='.';
			(*(raw_bios_buffer+b)>0x0f)?cout<<" "<<(unsigned int)*(raw_bios_buffer+b):cout<<"0"<<(unsigned int)*(raw_bios_buffer+b);
			if((b&0x0f)==0x0f) cout<<" | "<<sector_line<<endl;
		}
		cout<<dec;
	}
	return true;
}

const char * RAW::GetMediaType(void)
{
	return IMAGE_MEDIA_RAW;
}

unsigned char RAW::GetMedia(void)
{
	return MEDIA_TYPE_RAW;
}

bool RAW::BuildHeader(unsigned int sectors, unsigned int bps)
{
	return true;
}

RAW::~RAW(void)
{
	if((image!=NULL) || (image_load==true)) delete image;
	delete[] raw_bios_buffer;
}
