/* Franny 1.0
 * ATR disk access routines
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

#include "atr.h"

const char *IMAGE_MEDIA_ATR="ATR";

using namespace std;
using namespace types;
using namespace atr;

ATR::ATR(void)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image_type=IMAGE_UNKNOWN;
	image=NULL;
	total_sectors=0;
	atr_bios_buffer=new unsigned char[SECTOR_DOUBLE];
}

ATR::ATR(char *fname)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image_type=IMAGE_UNKNOWN;
	image=NULL;
	total_sectors=0;
	atr_bios_buffer=new unsigned char[SECTOR_DOUBLE];
	OpenImage(fname);
}

ATR::ATR(char *fname, unsigned int sectors, unsigned int bps)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image=NULL;
	total_sectors=sectors;
	atr_bios_buffer=new unsigned char[SECTOR_DOUBLE];
	NewImage(fname, sectors, bps);
}

bool ATR::OpenImage(char *fname)
{
	unsigned int real_image_size, a;

	if(image_load) CloseImage();
	try {
		image=new fstream(fname, fstream::in|fstream::out|fstream::binary);
	}
	catch(...)
	{ return false; }
	if(image->is_open()!=true) return false;
	image->seekg(0, ios::end);
	real_image_size=image->tellg();
	image->seekg(0, ios::beg);
	image->read((char *)atr_bios_buffer, HEADER_SIZE);
	if((*(atr_bios_buffer)!=0x96) && (*(atr_bios_buffer+1)!=0x02)) return 2;
	total_sectors=((*(atr_bios_buffer+6)<<16)+(*(atr_bios_buffer+3)<<8)+*(atr_bios_buffer+2))<<4;
	a=0;
	if((*(atr_bios_buffer+4)==(SECTOR_SINGLE&0xff)) && (*(atr_bios_buffer+5)==((SECTOR_SINGLE&0xff00)>>8))) a=7;
	if((*(atr_bios_buffer+4)==(SECTOR_DOUBLE&0xff)) && (*(atr_bios_buffer+5)==((SECTOR_DOUBLE&0xff00)>>8))) a=8;
	if(!a) return false;
	if((total_sectors+HEADER_SIZE)==real_image_size) { corrupted=false; image_type=IMAGE_STANDARD; }
	else
	{
		total_sectors=(((*(atr_bios_buffer+3)<<4)+(*(atr_bios_buffer+2)>>4))<<a);
		if(a==8) total_sectors-=3*SECTOR_SINGLE;
		if(((real_image_size-HEADER_SIZE)&0xfffff)==total_sectors)
		{
			corrupted=false;
			image_type=IMAGE_SIO2IDE;
			total_sectors=real_image_size-HEADER_SIZE;
		}
		else corrupted=true;
	}
	if(a==8)
	{
		bytes_per_sector=SECTOR_DOUBLE;
		total_sectors=(total_sectors+(3*SECTOR_SINGLE))>>8;
		(total_sectors==720)?density=DENSITY_DOUBLE:density=DENSITY_CUSTOM_256;
	}
	else
	{
		bytes_per_sector=SECTOR_SINGLE;
		total_sectors=total_sectors>>7;
		(total_sectors==720)?density=DENSITY_SINGLE:((total_sectors==1040)?density=DENSITY_MEDIUM:density=DENSITY_CUSTOM_128);
	}
	image_load=true;
	return true;
}

inline bool ATR::SetForceWrite(bool force)
{
	force_write=force;
	return true;
}

unsigned int ATR::ReadSector(unsigned char *buffer, unsigned int sector)
{
	unsigned int pos;

	if((image_load==false) || (sector==0) || (sector>total_sectors)) return 0;
	if((--sector>=3) && (bytes_per_sector==SECTOR_DOUBLE))
	{
		pos=3*SECTOR_SINGLE+((sector-3)*SECTOR_DOUBLE)+HEADER_SIZE;
		image->seekg(pos, ios::beg);
		image->read((char *)buffer, SECTOR_DOUBLE);
		return SECTOR_DOUBLE;
	}
	else
	{
		pos=sector*SECTOR_SINGLE+HEADER_SIZE;
		image->seekg(pos, ios::beg);
		image->read((char *)buffer, SECTOR_DOUBLE);
		return SECTOR_SINGLE;
	}
	return 0;
}

unsigned int ATR::SaveSector(unsigned char *buffer, unsigned int sector)
{
	unsigned int pos;

	if((image_load==false) || (sector==0) || (sector>total_sectors)) return 0;
	if((--sector>=3) && (bytes_per_sector==SECTOR_DOUBLE))
	{
		pos=3*SECTOR_SINGLE+((sector-3)*SECTOR_DOUBLE)+HEADER_SIZE;
		image->seekg(pos, ios::beg);
		image->write((char *)buffer, SECTOR_DOUBLE);
		return SECTOR_DOUBLE;
	}
	else
	{
		pos=sector*SECTOR_SINGLE+HEADER_SIZE;
		image->seekg(pos, ios::beg);
		image->write((char *)buffer, SECTOR_SINGLE);
		return SECTOR_SINGLE;
	}
	return 0;
}

bool ATR::CloseImage(void)
{
	if(image_load==false) return false;
	delete image;
	image=NULL;
	image_load=false;
	total_sectors=0;
	image_type=IMAGE_UNKNOWN;
	return true;
}

bool ATR::NewImage(char *fname, unsigned int sectors, unsigned int bps)
{
	unsigned int a;

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
	BuildHeader(sectors, bps);
	image->write((char *)atr_bios_buffer, HEADER_SIZE);
	memset(atr_bios_buffer, 0, bps);
	for(a=0;a<sectors;a++)
		{ (a<3)?image->write((char *)atr_bios_buffer, SECTOR_SINGLE):image->write((char *)atr_bios_buffer, bps); }
	image_load=true;
	corrupted=false;
	image_type=IMAGE_STANDARD;
	bytes_per_sector=bps;
	total_sectors=sectors;
	if(sectors==720) (bps==SECTOR_DOUBLE)?density=DENSITY_DOUBLE:density=DENSITY_SINGLE;
	else if((sectors==1040) && (bps==SECTOR_SINGLE)) density=DENSITY_MEDIUM;
	else (bps==SECTOR_DOUBLE)?density=DENSITY_CUSTOM_256:density=DENSITY_CUSTOM_128;
	return true;
}

unsigned char ATR::GetDensity(void)
{
	return density;
}

unsigned int ATR::GetSectorSize(void)
{
	return bytes_per_sector;
}

unsigned int ATR::GetTotalSectors(void)
{
	return total_sectors;
}

unsigned char ATR::GetImageType(void)
{
	return image_type;
}

bool ATR::SetImageType(unsigned char type)
{
	unsigned int len;

	if(type==IMAGE_UNKNOWN) return false;
	if(type==image_type) return true;
	memset(atr_bios_buffer, 0, HEADER_SIZE);
	*atr_bios_buffer=0x96;
	*(atr_bios_buffer+1)=0x02;
	*(atr_bios_buffer+4)=(char)(bytes_per_sector&0xff);
	*(atr_bios_buffer+5)=(char)((bytes_per_sector&0xff00)>>8);
	if(type==IMAGE_STANDARD)
	{
		len=((3*SECTOR_SINGLE)+(total_sectors-3)*bytes_per_sector)>>4;
		*(atr_bios_buffer+2)=(char)(len&0xff);
		*(atr_bios_buffer+3)=(char)((len&0xff00)>>8);
		*(atr_bios_buffer+6)=(char)((len&0xff0000)>>16);
	}
	else if(type==IMAGE_SIO2IDE)
	{
		if(bytes_per_sector==SECTOR_SINGLE) len=total_sectors>>1;
		else len=total_sectors;
		*(atr_bios_buffer+2)=(char)((len&0x0f)<<4);
		*(atr_bios_buffer+3)=(char)((len&0xff0)>>4);
	}
	image->seekg(0, ios::beg);
	image->write((char *)atr_bios_buffer, HEADER_SIZE);
	image_type=type;
	return true;
}

bool ATR::SetCorrupted(bool status)
{
	corrupted=status;
	return true;
}

bool ATR::DumpSector(unsigned int sector)
{
	unsigned int a, b;
	char sector_line[17];

	if((image_load==false) || (sector==0) || (sector>total_sectors)) return false;
	((sector>3) && (bytes_per_sector==SECTOR_DOUBLE))?a=SECTOR_DOUBLE:a=SECTOR_SINGLE;
	if(ReadSector(atr_bios_buffer, sector)>0)
	{
		cout<<hex;
		sector_line[16]='\0';
		for(b=0;b<a;b++)
		{
			if(!(b&0x0f)) (b<0x0f)?cout<<"0"<<b<<":":cout<<b<<":";
			(*(atr_bios_buffer+b)>=0x20)?sector_line[b&0x0f]=*(atr_bios_buffer+b):sector_line[b&0x0f]='.';
			(*(atr_bios_buffer+b)>0x0f)?cout<<" "<<(unsigned int)*(atr_bios_buffer+b):cout<<" 0"<<(unsigned int)*(atr_bios_buffer+b);
			if((b&0x0f)==0x0f) cout<<" | "<<sector_line<<endl;
		}
		cout<<dec;
	}
	return true;
}

const char * ATR::GetMediaType(void)
{
	return IMAGE_MEDIA_ATR;
}

unsigned char ATR::GetMedia(void)
{
	return MEDIA_TYPE_ATR;
}

bool ATR::BuildHeader(unsigned int sectors, unsigned int bps)
{	
	unsigned int len;

	memset(atr_bios_buffer, 0, HEADER_SIZE);
	len=((3*SECTOR_SINGLE)+(sectors-3)*bps)>>4;
	*atr_bios_buffer=0x96;
	*(atr_bios_buffer+1)=0x02;
	*(atr_bios_buffer+2)=(char)(len&0xff);
	*(atr_bios_buffer+3)=(char)((len&0xff00)>>8);
	*(atr_bios_buffer+4)=(char)(bps&0xff);
	*(atr_bios_buffer+5)=(char)((bps&0xff00)>>8);
	*(atr_bios_buffer+6)=(char)((len&0xff0000)>>16);
	return true;
}

ATR::~ATR(void)
{
	if((image!=NULL) || (image_load==true)) delete image;
	delete[] atr_bios_buffer;
}
