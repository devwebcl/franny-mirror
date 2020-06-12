/* Franny 1.0
 * User's application
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

#include <iostream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include "atari_ii.h"
#include "franny.h"
#include "interface.h"

using namespace std;
using namespace interface;

const char *DEFAULT_LIST_MASK="*.*";

bool checkFileType(char *file)
{
	struct stat file_info;

	if(file==NULL) return false;
	if(stat(file, &file_info)!=0) return false;
	if(file_info.st_mode&S_IFREG) return true;
	return false;
}

int main(int argcnt, char *argvec[])
{
	INTERFACE interface;
	string new_image;
	int opt, sectors=720, sector_size=256, total_commands=0, param1=0, name_seek=0, result=0;
	char *atr_name=NULL, command=COMMAND_NONE, fs_type=FSTYPE_UNKNOWN, *name=NULL, *dest_name=NULL;
	char *list_format=NULL;
	char media_type=MEDIA_TYPE_UNKNOWN, new_media_type;
	int image_type=IMAGE_STANDARD;
	bool force=false;

	if(argcnt==1)
	{
		cout<<PROGRAM_NAME<<": Try: "<<argvec[0]<<" -h"<<endl;
		return 3;
	}
	while((opt=getopt(argcnt, argvec, "ACd:f:Fhi:Il:L::M:m:N:O:o:R:s:St:T:U:vV:"))!=-1)
	{
		switch(opt)
		{
			case 'A':
				command=COMMAND_ADD;
				++total_commands;
				break;
			case 'C':
				if(command!=COMMAND_TYPE) ++total_commands;
				command=COMMAND_CREATE;
				break;
			case 'd':
				if(strlen(optarg)>1) { cerr<<PROGRAM_NAME<<": Bad -d parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(*optarg=='s') sector_size=128;
				else if(*optarg=='d') sector_size=256;
				else { cerr<<PROGRAM_NAME<<": Bad -d parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				break;
			case 'f':
				if(strlen(optarg)>1) { cerr<<PROGRAM_NAME<<": Bad -f parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(*optarg=='a') fs_type=FSTYPE_DOS2;
				else if(*optarg=='s') fs_type=FSTYPE_SPARTA2;
				else { cerr<<PROGRAM_NAME<<": Bad -f parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				break;
			case 'F':
				command=COMMAND_FORMAT;
				++total_commands;
				break;
			case 'h':
				cout<<"Usage: "<<PROGRAM_NAME<<" options imagename.atr [newimagename.atr]"<<endl<<endl;;
				cout<<"-A\t\tadd file to image (parameters must be added with -i and -o),"<<endl;
				cout<<"-C\t\tcreate new image,"<<endl;
				cout<<"-d [s|d]\tsector's size [single|double]; default: double,"<<endl;
				cout<<"-f [a|s]\tfile system type [AtariDOS II|Sparta 2.0]; default: Sparta 2.0,"<<endl;
				cout<<"-F\t\tformat,"<<endl;
				cout<<"-h\t\thelp,"<<endl;
				cout<<"-i filename\tinput filename (source file for -A and -S),"<<endl;
				cout<<"-I\t\timage information,"<<endl;
				cout<<"-l format\tlist format,"<<endl;
				cout<<"-Lmask\t\tlist directory,"<<endl;
				cout<<"-M dirname\tcreate dir,"<<endl;
				cout<<"-m [a|r]\tmedia type (atr|raw); default: atr,"<<endl;
				cout<<"-N name\t\tvolume name,"<<endl;
				cout<<"-o filename\toutput filename (destination file for -A and -S),"<<endl;
				cout<<"-O [a|r]\tnew media type; new image file name, must be specified after image name,"<<endl;
				cout<<"-R dirname\tremove dir,"<<endl;
				cout<<"-s sectors\ttotal sectors; default: 720,"<<endl;
				cout<<"-S\t\tsave (extract) filename from image (parameters must be added with -i and -o),"<<endl;
				cout<<"-t [s|m|d|f|F]\timage templates [single|medium|double|full/Full(65535 sectors, 128/256 bytes each)],"<<endl;
				cout<<"-T [d|s]\tchange image type (default|sio2ide),"<<endl;
				cout<<"-U filename\tunlink(remove) file,"<<endl;
				cout<<"-v\t\tversion,"<<endl;
				cout<<"-V sector\tview (dump) sector."<<endl<<endl;
				cout<<"Report bugs to 'zooey-devel@lists.sourceforge.net'."<<endl;
				cout<<"Copyright (C) 2000-2006, Rafa³ 'Bob_er' Ciepiela."<<endl;
				cout<<"This is free software; for details see attached 'copying' file or source files."<<endl;
				return 4;
				break;
			case 'i':
				if(name!=NULL) delete[] name;
				name=new char[strlen(optarg)+1];
				strcpy(name, optarg);
				break;
			case 'I':
				command=COMMAND_INFO;
				++total_commands;
				break;
			case 'l':
				if(list_format!=NULL) delete[] list_format;
				list_format=new char[strlen(optarg)+1];
				strcpy(list_format, optarg);
				break;
			case 'L':
				if(name!=NULL) delete[] name;
				if(optarg==NULL)
				{
					name=new char[strlen(DEFAULT_LIST_MASK)+1];
					strcpy(name, DEFAULT_LIST_MASK);
				}
				else
				{
					name=new char[strlen(optarg)+1];
					strcpy(name, optarg);
				}
				command=COMMAND_LIST;
				++total_commands;
				break;
			case 'M':
				if(name!=NULL) delete[] name;
				name=new char[strlen(optarg)+1];
				strcpy(name, optarg);
				command=COMMAND_MKDIR;
				++total_commands;
				break;
			case 'm':
				if(strlen(optarg)>1) { cerr<<PROGRAM_NAME<<": Bad -m parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(*optarg=='a') media_type=MEDIA_TYPE_ATR;
				else if(*optarg=='r') media_type=MEDIA_TYPE_RAW;
				else { cerr<<PROGRAM_NAME<<": Bad -m parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				break;
			case 'N':
				if(name!=NULL) delete[] name;
				name=new char[strlen(optarg)+1];
				strcpy(name, optarg);
				command=COMMAND_NAME;
				++total_commands;
				break;
			case 'o':
				if(dest_name!=NULL) delete[] dest_name;
				dest_name=new char[strlen(optarg)+1];
				strcpy(dest_name, optarg);
				break;
			case 'O':
				command=COMMAND_CONV;
				if(strlen(optarg)>1) { cerr<<PROGRAM_NAME<<": Bad -O parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(*optarg=='a') new_media_type=MEDIA_TYPE_ATR;
				else if(*optarg=='r') new_media_type=MEDIA_TYPE_RAW;
				else { cerr<<PROGRAM_NAME<<": Bad -O parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				++total_commands;
				break;
			case 'R':
				if(name!=NULL) delete[] name;
				name=new char[strlen(optarg)+2];
				strcpy(name, optarg);
				if(strchr(name, '.')==NULL) strcat(name, ".");
				command=COMMAND_RMDIR;
				++total_commands;
				break;
			case 's':
				sectors=atoi(optarg);
				if((sectors>65535) || (sectors<1))
					{ cerr<<PROGRAM_NAME<<": Bad -s parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				break;
			case 'S':
				command=COMMAND_SAVE;
				++total_commands;
				break;
			case 't':
				if(strlen(optarg)>1) { cerr<<PROGRAM_NAME<<": Bad -t parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(*optarg=='s') { sector_size=128; sectors=720; }
				else if(*optarg=='m') { sector_size=128; sectors=1040; }
				else if(*optarg=='d') { sector_size=256; sectors=720; }
				else if(*optarg=='f') { sector_size=128; sectors=65535; }
				else if(*optarg=='F') { sector_size=256; sectors=65535; }
				else { cerr<<PROGRAM_NAME<<": Bad -t parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				break;
			case 'T':
				if(strlen(optarg)>1) { cerr<<PROGRAM_NAME<<": Bad -T parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(*optarg=='d') image_type=IMAGE_STANDARD;
				else if(*optarg=='s') image_type=IMAGE_SIO2IDE;
				else { cerr<<PROGRAM_NAME<<": Bad -T parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				if(command!=COMMAND_CREATE)
				{
					command=COMMAND_TYPE;
					++total_commands;
				}
				break;
			case 'U':
				if(name!=NULL) delete[] name;
				name=new char[strlen(optarg)+1];
				strcpy(name, optarg);
				command=COMMAND_UNLINK;
				++total_commands;
				break;
			case 'v':
				cout<<PROGRAM_NAME<<" "<<VERSION<<endl;
				return 4;
				break;
			case 'V':
				param1=atoi(optarg);
				if((param1>65535) || (param1<1))
					{ cerr<<PROGRAM_NAME<<": Bad -V parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl; return 3; }
				command=COMMAND_DUMP;
				++total_commands;
				break;
			default:
				cerr<<PROGRAM_NAME<<": Unknown option or bad parameter. Try: "<<PROGRAM_NAME<<" -h!"<<endl;
				return 3;
		}
	}
	if((argvec[optind]!=NULL) && (atr_name==NULL))
	{
		atr_name=new char[strlen(argvec[optind])+1];
		strcpy(atr_name, argvec[optind]);
	}
	if(argvec[optind+1]!=NULL)
	{
		new_image=argvec[optind+1];
	}
	if(command==COMMAND_NONE)
	{
			cerr<<PROGRAM_NAME<<": No action specified."<<endl;
			return 3;
	}
	if(total_commands!=1)
	{
			cerr<<PROGRAM_NAME<<": You can specify only one command."<<endl;
			return 3;
	}
	if(atr_name==NULL)
	{
		cerr<<PROGRAM_NAME<<": No image specified."<<endl;
		return 3;
	}
	if((command!=COMMAND_CREATE) && (checkFileType(atr_name)==false))
	{
		cerr<<PROGRAM_NAME<<": Specified file is not regular file."<<endl;
		return 3;
	}
	if(command!=COMMAND_CREATE) if(interface.Open(atr_name, media_type, fs_type)==false) { cout<<PROGRAM_NAME<<": Cannot open image '"<<atr_name<<"'."<<endl; return 3; }
	if((command==COMMAND_SAVE) || (command==COMMAND_ADD))		// dla 5 i 6 musza byc podane oba argumenty.
	{
		if((name==NULL) || (dest_name==NULL))
		{
			cerr<<PROGRAM_NAME<<": You must specify input and output filename."<<endl;
			return 3;
		}
	}
	if(name!=NULL)
	{
		if(command==COMMAND_ADD)
		{
			if(checkFileType(name)==false)
			{
				cerr<<PROGRAM_NAME<<": Specified file is not regular file."<<endl;
				return 3;
			}
			name_seek=interface.ChangeDirectory(dest_name);
		}
		else if(command!=COMMAND_NAME) name_seek=interface.ChangeDirectory(name);
		if(name_seek<0)
		{
				cerr<<PROGRAM_NAME<<": Cannot change directory."<<endl;
				return 3;
		}
	}
	switch(command)
	{
		case COMMAND_CREATE:
			if(fs_type==FSTYPE_UNKNOWN) fs_type=FSTYPE_SPARTA2;
			if(media_type==MEDIA_TYPE_UNKNOWN) media_type=MEDIA_TYPE_ATR;
			if((sectors>0) && (sector_size>0)) 
			{
				if(interface.Create(atr_name, sectors, sector_size, media_type, fs_type)==false)
					cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				else if(interface.Format()==false)
				{
					cerr<<PROGRAM_NAME<<": Cannot format image."<<endl;
					result=1;
				}
				if((result==0) && (media_type==MEDIA_TYPE_ATR))
				{
					if(interface.ChangeImageType(image_type)==false)
					{
						cerr<<PROGRAM_NAME<<": Cannot set image type."<<endl;
						result=1;
					}
				}
			}
			else cout<<PROGRAM_NAME<<": Bad -C parameters. Use '-d -s' or '-t'."<<endl;
			break;
		case COMMAND_LIST:
			if(interface.List(name+name_seek, list_format)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_INFO:
			interface.Status();
			break;
		case COMMAND_DUMP:
			interface.DumpSector(param1);
			break;
		case COMMAND_SAVE:
			if(interface.DumpFile(name+name_seek, dest_name)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_ADD:
			if(interface.InsertFile(name, dest_name+name_seek)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_UNLINK:
			if(interface.DeleteFile(name+name_seek)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_TYPE:
			if(interface.ChangeImageType(image_type)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_NAME:
			if(interface.SetVolumeName(name)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_MKDIR:
			if(interface.MakeDir(name+name_seek)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_RMDIR:
			if(interface.RemoveDir(name+name_seek)==false)
			{
				cerr<<PROGRAM_NAME<<": Command failed."<<endl;
				result=1;
			}
			break;
		case COMMAND_FORMAT:
			if(interface.Format()==false)
			{
				cerr<<PROGRAM_NAME<<": Cannot format image."<<endl;
				result=1;
			}
			break;
		case COMMAND_CONV:
			if(interface.ConvertImage(new_media_type, (char *)new_image.c_str())==false)
			{
				cerr<<PROGRAM_NAME<<": Cannot convert image."<<endl;
				result=1;
			}
			break;
		default:
			cerr<<PROGRAM_NAME<<": Illegal operation "<<(int)command<<" (Not implemented yet - please report!)"<<endl;
			return 3;
	}
	if(atr_name!=NULL) delete[] atr_name;
	if(name!=NULL) delete[] name;
	if(dest_name!=NULL) delete[] dest_name;
	if(list_format!=NULL) delete[] list_format;
	return result;
}
