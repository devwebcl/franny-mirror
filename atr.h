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

#ifndef ATR_BIOS10
#define ATR_BIOS10

#include <iostream>
#include <fstream>
#include <string.h>

#include "image.h"
#include "types.h"

using namespace std;
using namespace image;
namespace atr
{

class ATR:public IMAGE
{
	private:
		fstream *image;
		unsigned int total_sectors, density;
		unsigned char *atr_bios_buffer, image_type;
		bool image_load, corrupted, force_write;

	protected:
		unsigned int bytes_per_sector;

	public:
		ATR(void);
		ATR(char *);                                                 // fname
		ATR(char *, unsigned int, unsigned int);                     // fname, sectors, bytes/sector
		bool OpenImage(char *);                                      // fname
		unsigned int ReadSector(unsigned char*, unsigned int);       // buffer, sector
		unsigned int SaveSector(unsigned char*, unsigned int);               // buffer, sector
		bool CloseImage(void);
		bool NewImage(char *, unsigned int, unsigned int);           // fname, sectors, bytes/sector
		bool SetForceWrite(bool);
		bool SetImageType(unsigned char);                            // new image type
		bool SetCorrupted(bool);
		unsigned char GetImageType(void);
		unsigned char GetDensity(void);
		unsigned int GetSectorSize(void);
		unsigned int GetTotalSectors(void);
		bool DumpSector(unsigned int);                               // sector
		const char * GetMediaType(void);
		unsigned char GetMedia(void);
		bool BuildHeader(unsigned int sectors, unsigned int bps);
		~ATR(void);
};

}	// atrbios

#endif
