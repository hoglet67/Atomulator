/*
	ff_emudir.h
*/

#include "integer.h"

#ifndef __FF_EMUDIR__

#define PATHSIZE	255
#define FNAMELEN	12

/* File attribute bits for directory entry */

#define	FAT_READONLY	0x01	/* Read only */
#define	FAT_HIDDEN		0x02	/* Hidden */
#define	FAT_SYSTEM		0x04	/* System */
#define	FAT_VOLUMELABEL	0x08	/* Volume label */
#define FAT_LFN			0x0F	/* LFN entry */
#define FAT_DIRECTORY	0x10	/* Directory */
#define FAT_ARCHIVE		0x20	/* Archive */
#define FAT_MASK		0x3F	/* Mask of defined bits */

typedef struct 
{
	DWORD	fsize;		/* File size */
	BYTE	fattrib;	/* Attribute */
	BYTE	filename[FNAMELEN+1];	
	BYTE	path[PATHSIZE+1];
} EMUDIR;

int findfirst(char		path[],
			  EMUDIR	*dir);
int findnext(EMUDIR	*dir);
int findclose(EMUDIR	*dir);

char *saferealpath(const char *path, char *resolved_path); 

BYTE get_fat_attribs(int	fid);

#define __FF_EMUDIR__
#endif