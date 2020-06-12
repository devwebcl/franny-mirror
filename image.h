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

#ifndef IMAGE_BIOS10
#define IMAGE_BIOS10

#include <iostream>
#include <fstream>
#include <string.h>

#include "types.h"

using namespace std;
namespace image
{

class IMAGE
{
	private:
		fstream *image;
		unsigned int total_sectors, density;
		unsigned char image_type;
		bool image_load, corrupted, force_write;

	public:
		IMAGE(void);
		IMAGE(char *);                                                 // fname
		IMAGE(char *, unsigned int, unsigned int);                     // fname, sectors, bytes/sector
		virtual bool OpenImage(char *);                             // fname
		virtual unsigned int ReadSector(unsigned char*, unsigned int);       // buffer, sector
		virtual unsigned int SaveSector(unsigned char*, unsigned int);               // buffer, sector
		virtual bool CloseImage(void);
		virtual bool NewImage(char *, unsigned int, unsigned int);   // fname, sectors, bytes/sector
		virtual bool SetForceWrite(bool);
		virtual bool SetImageType(unsigned char);                            // new image type
		virtual bool SetCorrupted(bool);
		virtual unsigned char GetImageType(void);
		virtual unsigned char GetDensity(void);
		virtual unsigned int GetSectorSize(void);
		virtual unsigned int GetTotalSectors(void);
		virtual bool DumpSector(unsigned int);                               // sector
		virtual const char * GetMediaType(void);
		virtual unsigned char GetMedia(void);
		virtual bool BuildHeader(unsigned int sectors, unsigned int bps);
		virtual ~IMAGE(void);
};

}	// image

#endif
