/* Franny 1.0
 * Const values and types
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

#ifndef TYPES10
#define TYPES10

namespace types
{

static const char *DEFAULT_DISK_LABEL="UNKNOWN    ";

const int SIZE_SINGLE=92160;
const int SIZE_MEDIUM=133120;
const int SIZE_DOUBLE=183926;
const int SIZE_SUB=384;		/* 3*128 */
const unsigned char MEDIA_TYPE_UNKNOWN=0;
const unsigned char MEDIA_TYPE_ATR=1;
const unsigned char MEDIA_TYPE_RAW=2;
const unsigned int MAX_SECTORS=0x10000;
const unsigned short int SECTOR_SINGLE=0x80;
const unsigned short int SECTOR_DOUBLE=0x100;
const unsigned char HEADER_SIZE=0x10;
const unsigned char DENSITY_SINGLE=0;
const unsigned char DENSITY_MEDIUM=1;
const unsigned char DENSITY_CUSTOM_128=2;
const unsigned char DENSITY_DOUBLE=10;
const unsigned char DENSITY_CUSTOM_256=11;
const unsigned char DENSITY_UNKNOWN=255;
const char IMAGE_UNKNOWN=0;
const char IMAGE_STANDARD=1;
const char IMAGE_SIO2IDE=2;
const char FSTYPE_UNKNOWN=0;
const char FSTYPE_DOS2=1;
const char FSTYPE_SPARTA2=2;
const char MAX_FILENAME_LENGTH=13;
const bool SECTOR_FILL=false;
const bool SECTOR_EMPTY=true;

struct ATR_FILE {
	unsigned int FirstSector;
	int SizeInBytes;
	int SizeInSectors;
	char Name[9];
	char Ext[4];
	unsigned char FileId;
	unsigned char Attributes;
	unsigned char Year;
	unsigned char Month;
	unsigned char Day;
	unsigned char Hour;
	unsigned char Minute;
	unsigned char Second;
};

}	// types

#endif
