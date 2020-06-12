/* Franny 1.0
 * core disk access routines
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

#include "image.h"

using namespace std;
using namespace types;
using namespace image;

const char *IMAGE_MEDIA_UNKNOWN="unknown";

IMAGE::IMAGE(void)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image_type=IMAGE_UNKNOWN;
	image=NULL;
	total_sectors=0;
}

IMAGE::IMAGE(char *fname)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image_type=IMAGE_UNKNOWN;
	image=NULL;
	total_sectors=0;
}

IMAGE::IMAGE(char *fname, unsigned int sectors, unsigned int bps)
{
	image_load=false;
	corrupted=false;
	force_write=false;
	image=NULL;
	total_sectors=0;
}

bool IMAGE::OpenImage(char *fname)
{
	return false;
}

inline bool IMAGE::SetForceWrite(bool force)
{
	return false;
}

unsigned int IMAGE::ReadSector(unsigned char *buffer, unsigned int sector)
{
	return 0;
}

unsigned int IMAGE::SaveSector(unsigned char *buffer, unsigned int sector)
{
	return 0;
}

bool IMAGE::CloseImage(void)
{
	return false;
}

bool IMAGE::NewImage(char *fname, unsigned int sectors, unsigned int bps)
{
	return false;
}

unsigned char IMAGE::GetDensity(void)
{
	return DENSITY_UNKNOWN;
}

unsigned int IMAGE::GetSectorSize(void)
{
	return 0;
}

unsigned int IMAGE::GetTotalSectors(void)
{
	return 0;
}

unsigned char IMAGE::GetImageType(void)
{
	return IMAGE_UNKNOWN;
}

bool IMAGE::SetImageType(unsigned char type)
{
	return false;
}

bool IMAGE::SetCorrupted(bool status)
{
	return false;
}

bool IMAGE::DumpSector(unsigned int sector)
{
	return false;
}

const char * IMAGE::GetMediaType(void)
{
	return IMAGE_MEDIA_UNKNOWN;
}

unsigned char IMAGE::GetMedia(void)
{
	return MEDIA_TYPE_UNKNOWN;
}

bool IMAGE::BuildHeader(unsigned int sector, unsigned int bps)
{
	return false;
}

IMAGE::~IMAGE(void)
{
	return;
}
