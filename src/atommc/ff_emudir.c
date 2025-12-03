/*
	ff_emudir.c

	Note this code uses static directory / file entries.

	2012-06-12, Phill Harvey-Smith.
*/

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include "ff_emudir.h"
#include "../atom.h"

static DIR *dirptr;
static struct dirent *entry;
static int error;

void StripTrailingSlash(char	*path);

int findfirst(char		path[],
			  EMUDIR	*dir)
{
	findclose(dir);

	StripTrailingSlash(path);
	//rpclog("findfirst(%s)\n",path);

	dirptr=opendir(path);
	if(dirptr!=NULL)
	{
		strcpy(dir->path,path);
	}

	return (dirptr!=NULL);
}

int validSFN(char *filename)
{
	int	result = 1;
	int dotpos = -1;
	int beforedot = 0;
	int afterdot = 0;
	int CharNo = 0;

	for(CharNo=0; CharNo<strlen(filename); CharNo++)
	{
		if((dotpos<0) && (filename[CharNo]=='.'))
			dotpos=CharNo;

		if(dotpos<0)
			beforedot++;

		if((dotpos>=0) && (dotpos!=CharNo))
			afterdot++;
	}

	// invalid, more than 8 characters in filename
	if(beforedot>8)
		result = 0;

	// invalid extension > 3 characters
	if(afterdot>3)
		result = 0;

	return result;
}

int findnext(EMUDIR	*dir)
{
	char filename[PATHSIZE*2];
	struct stat statbuf;

	//rpclog("findnext()\n");

	do
	{
		entry=readdir(dirptr);
	} while ((entry!=NULL) && (!validSFN(entry->d_name)));

	if(entry!=NULL)
	{
		strncpy(dir->filename,entry->d_name,FNAMELEN);
		snprintf(filename,PATHSIZE*2,"%s/%s",dir->path,dir->filename);

		//debuglog("%s\n",filename);
		if(0==stat(filename, &statbuf))
		{
			dir->fsize=statbuf.st_size;

			if(S_ISDIR(statbuf.st_mode))
				dir->fattrib=FAT_DIRECTORY;
			else
				dir->fattrib=0;
		}
		//rpclog("Filename:%s, Size:%d, Attr:%02X\n",dir->filename,dir->fsize,dir->fattrib);
	}

	return (entry!=NULL);
}

void findclose(EMUDIR	*dir)
{
	//rpclog("findclose()\n");

	closedir(dirptr);
}

void StripTrailingSlash(char	*path)
{
	int PathLen = strlen(path)-1;

	if (PathLen>=0)
		if((path[PathLen]=='/') || (path[PathLen]=='\\'))
			path[PathLen]=0x00;
}

/**
 * MinGW doesn't have realpath, so use fallback implementation in that case,
 * otherwise this just calls through to realpath
 *
 */
char *saferealpath(const char *path, char *resolved_path)
{
#ifdef __WIN32__
	char	buf[MAXPATH];
	char* 	basename;
	int		len;
	size_t	idx;

	_fullpath(buf,path,MAXPATH);

	len=strlen(buf);

	if (len == 0 || len > MAXPATH - 1)
		strcpy(resolved_path, path);
	else
		strcpy(resolved_path, buf);

	// Replace backslashes with forward slashes so the
	// rest of the code behaves correctly.
	for (idx = 0; idx < strlen(resolved_path); idx++)
	{
		if (resolved_path[idx] == '\\')
			resolved_path[idx] = '/';
	}

	// Strip off any trailing slashes
	idx = strlen(resolved_path) - 1; // start at the index of the last character
	while (idx > 1 && resolved_path[idx] == '/')
	{
		resolved_path[idx] = 0; // shorten then string by one character
		idx--;
	}

	return resolved_path;
#else
	return realpath(path, resolved_path);
#endif
}

BYTE get_fat_attribs(int	fid)
{
	struct stat statbuf;
	BYTE		result = 0;

	if(0==fstat(fid,&statbuf))
	{
		if(S_ISDIR(statbuf.st_mode))
			result |= FAT_DIRECTORY;
	}
	return result;
}
