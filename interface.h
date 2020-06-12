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

#ifndef INTERFACE_10
#define INTERFACE_10

#include <string.h>

#include "image.h"
#include "atr.h"
#include "raw.h"
#include "fs.h"
#include "atari_ii.h"
#include "sparta.h"
#include "types.h"

using namespace image;
using namespace fs;
using namespace types;

namespace interface
{

class INTERFACE
{
	private:
		IMAGE *media_image;
		FS *filesystem_image;
		char *image_filename;

		bool ShowName(ATR_FILE *, bool align=true);
		bool ShowExt(ATR_FILE *, bool align=true);
		bool ShowSizeInBytes(ATR_FILE *);
		bool ShowSizeInSectors(ATR_FILE *);
		bool ShowAttributes(ATR_FILE *);
		bool ShowDate(ATR_FILE *);
		bool ShowTime(ATR_FILE *);
		bool UpString(char *);
		int FindChar(char *, char);
		FS * DetectFS(IMAGE *);
		FS * AllocFS(IMAGE *, char);
		IMAGE * DetectMedia(void);
		IMAGE * AllocMedia(char);

	public:
		INTERFACE(void);
		INTERFACE(char *, char media_type=MEDIA_TYPE_UNKNOWN, char fs_type=FSTYPE_UNKNOWN);
		bool Create(char *, int, int, int, int);              // filename, sectors, bps, media_type
		bool Open(char *, char media_type=MEDIA_TYPE_UNKNOWN, char fs_type=FSTYPE_UNKNOWN); // filename, file system type
		bool Format(void);
		bool List(char *, char *format=NULL);           // file_mask, format
		void Status(void);
		bool DumpSector(int);                           // sector
		bool DumpFile(char *, char *);                  // src filename, dest filename
		bool InsertFile(char *, char *);                // src filename, dest filename
		bool DeleteFile(char *);                        // filename
		bool MakeDir(char *);                           // dirname
		bool RemoveDir(char *);                         // dirname
		bool ChangeImageType(char);                     // image type
		int ChangeDirectory(char *);                    // path
		bool SetVolumeName(char *);                     // volume name
		bool ConvertImage(unsigned int media, char *new_image);
		~INTERFACE(void);
};

}

#endif	/* INTERFACE_10 */
