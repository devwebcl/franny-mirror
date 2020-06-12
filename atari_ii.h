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

#ifndef ATARI2_10
#define ATARI2_10

#include <fstream>
#include <stdlib.h>

#include "fs.h"
#include "types.h"

using namespace std;
using namespace types;
using namespace fs;

namespace atariii
{

const unsigned char ATARI_II_BIT_MASKS[8]={ 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

const unsigned char ATARI_II_BIT_MASKS2[8]={ 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

class ATARI_II:public FS
{
	private:
		IMAGE *media_image;
		unsigned int current_sector, first_dir_sector, root_dir_sector;
		unsigned char current_file, *atari_ii_buffer, dir_length;
		char file_mask[16], disk_label[9], current_dir[5];
		bool force_atari_ii, set_mask;

		bool CheckFileName(char *, char *);                   // fname, mask;
		int FileSize(unsigned int, unsigned char);            // sector, file_id
		int FindFreeSector(void);
		char FindFreeFileId(void);
		bool SetSectorState(unsigned int, bool);              // sector, state (0 - fill, 1 - free)
		bool FindFirstFile_Private(ATR_FILE *, char *);       // file's data, file mask
		char * PrepareName(char *);                           // filename to prepare

	public:
		ATARI_II(void);
		ATARI_II(IMAGE *);                                     // fname
		bool Open(IMAGE *);			                             // fname
		bool Format(void);
		bool CheckFS(void);
		bool FindFirstFile(ATR_FILE *, char *);               // file's data, file mask
		bool FindNextFile(ATR_FILE *);
		int LoadFile(unsigned char **, char *);                // buffer, fname
		int SaveFile(unsigned char *, char *, unsigned int);  // buffer, fname, size
		bool DeleteFile(char *, bool=false);                  // fname, clean sectors
		bool MakeDir(char *);                                 // dir name
		bool DeleteDir(char *);                               // dir name
		bool CD(char *);                                      // path
		bool PWD(char *);                                     // place for result
		bool SetDiskLabel(char *);                            // disk name
		bool ReadDiskLabel(char *);                           // place for result
		int GetFreeSectors(void);
		char GetFSType(void);
		int GetSectorSize(void);
		int GetTotalSectors(void);
		int GetDensity(void);
		int GetImageType(void);
		~ATARI_II(void);
};

} // atariii

#endif
