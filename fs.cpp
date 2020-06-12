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

#include "fs.h"

using namespace std;
using namespace types;
using namespace fs;

FS::FS(void)
{
	return;
}

FS::FS(IMAGE *fname)
{
	return;
}

FS::FS(char *fname, unsigned int sectors, unsigned int bps)
{
	return;
}

bool FS::Open(IMAGE *fname)
{
	return false;
}

bool FS::Format(void)
{
	return false;
}

bool FS::CheckFS(void)
{
	return false;
}

bool FS::FindFirstFile(ATR_FILE *, char *)
{
	return false;
}

bool FS::FindNextFile(ATR_FILE *)
{
	return false;
}

int FS::LoadFile(unsigned char **, char *)
{
	return 0;
}

int FS::SaveFile(unsigned char *, char *, unsigned int)
{
	return 0;
}

bool FS::DeleteFile(char *, bool)
{
	return false;
}

bool FS::MakeDir(char *)
{
	return false;
}

bool FS::DeleteDir(char *)
{
	return false;
}

bool FS::CD(char *)
{
	return false;
}

bool FS::PWD(char *)
{
	return false;
}

bool FS::SetDiskLabel(char *)
{
	return false;
}

bool FS::ReadDiskLabel(char *result)
{
	return false;
}

int FS::GetFreeSectors(void)
{
	return 0;
}

char FS::GetFSType(void)
{
	return FSTYPE_UNKNOWN;
}

char * FS::PrepareName(char *name)
{
	return NULL;
}

int FS::GetSectorSize(void)
{
	return 0;
}

int FS::GetTotalSectors(void)
{
	return 0;
}

int FS::GetDensity(void)
{
	return DENSITY_UNKNOWN;
}

int FS::GetImageType(void)
{
	return IMAGE_UNKNOWN;
}

bool FS::DumpSector(int sector)
{
	return false;
}

FS::~FS(void)
{
	return;
}
