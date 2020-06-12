/* Franny 1.0
 * Filesystem core routines
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

#ifndef FS_BIOS10
#define FS_BIOS10

#include <iostream>
#include <fstream>
#include <string.h>

#include "types.h"
#include "image.h"

using namespace std;
using namespace types;
using namespace image;

namespace fs
{

class FS
{
	private:
		IMAGE *image;

	public:
		FS(void);
		FS(IMAGE *);                                                   // fname
		FS(char *, unsigned int, unsigned int);                       // fname, sectors, bytes/sector
		virtual bool Open(IMAGE *);                                    // fname
		virtual bool Format(void);
		virtual bool CheckFS(void);
		virtual bool FindFirstFile(ATR_FILE *, char *);               // file's data, file mask
		virtual bool FindNextFile(ATR_FILE *);
		virtual int LoadFile(unsigned char **, char *);                // buffer, fname
		virtual int SaveFile(unsigned char *, char *, unsigned int);  // buffer, fname, size
		virtual bool DeleteFile(char *, bool=false);                  // fname, clean sectors
		virtual bool MakeDir(char *);                                 // dir name
		virtual bool DeleteDir(char *);                               // dir name
		virtual bool CD(char *);                                      // path
		virtual bool PWD(char *);                                     // place for result
		virtual bool SetDiskLabel(char *);                            // disk name
		virtual bool ReadDiskLabel(char *);                           // place for result
		virtual bool DumpSector(int);
		virtual int GetFreeSectors(void);
		virtual char GetFSType(void);
		virtual char *PrepareName(char *);                            // name to prepare
		virtual int GetSectorSize(void);
		virtual int GetTotalSectors(void);
		virtual int GetDensity(void);
		virtual int GetImageType(void);
		virtual ~FS(void);
};

}	// FS

#endif
