/*
	ff_emu.c

	Functions to emulate the FatFilesystem routines as used by AtoMMC.

	Note this is by no means a complete emulation of FATFS, but
	replicates / emulates enough of the functionality to allow emulation
	of the AtoMMC interface firmware.

	I had to split the emulation over two files because of a clash of
	structure names used by fatfs calls and the underlying os.

	2012-06-12, Phill Harvey-Smith.
*/

#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>  // Required for OSX and lseek()

#include "integer.h"
#include "ff.h"
#include "ff_emudir.h"
#include "../atom.h"

// This sequence of defines is needed as MinGW specifically needs binary
// files to be opened with O_BINARY, which does not exist on other platforms.
#ifndef O_BINARY
# ifdef _O_BINARY
#  define O_BINARY  _O_BINARY
# else
#  define O_BINARY  0
# endif
#endif

DIR 	dj;
EMUDIR	emu;
char	MMCPath[PATHSIZE+1];
char	BaseMMCPath[PATHSIZE+1];
FIL		*openfil;

#ifdef DEBUGFF
void HexDump(const void 	*Buff,
					   int 		Length);

void HexDumpHead(const void 	*Buff,
					   int 		Length);

#endif

// This function is used to convert an path sent by AtoMMC
// to a filesystem path.
//
// Relative paths are appended to the curent directory (MMCPath)
//
// Absolute paths are appended to the "MMC" root directory (BaseMMCPath)
//
// Note: Atomulator does not securely sandbox the MMC directory. For example
// *CAT /.. will access the parent folder (containing the Atomulator exe file)
// *DELETE /..atom.cfg will delete the atom.cfg file (!!!)
//
// This is not a regression; it has always been the case.
//
// The f_chdir (change directory) function does try to prevent the user moveing
// to a directory that is above BaseMMCPath. It does a reasonable job of this.
// On linux, realpath() is used, for example. This approach does not generalize
// to the other f_ functions, because realpath() only works if the file/
// directory exists. So it could not, for example, be used by f_open().
//
// We should revisit this!
//
// AtoMMC emulation allows access to filesystem objects outside of the MMC path
// https://github.com/hoglet67/Atomulator/issues/13

static FRESULT build_absolute_path(const XCHAR *xpath, char *newpath, int validate_name)
{
	// Make a copy of the path, so we can normalize the path seperators
	char path[PATHSIZE+1];
	strncpy(path, xpath, PATHSIZE);

	// Normalize the path seperators (linux treats \ as a valid name character)
	for (int i = 0; i < strlen(path); i++) {
		if (path[i] == '\\') {
			path[i] = '/';
		}
	}

	// For most functions, we need to make sure the name part is a valid 8.3 name
	// (the exception is f_chdir where we allow things like .. and / and dir/)

	if (validate_name) {

		// Find the last element of the path
		char *name = rindex(path, '/');
		if (name == NULL) {
			name = path;
		} else {
			name++;
		}

		// Find the suffix
		char *suffix = index(name, '.');

		// Validate the name part
		int name_len = (suffix != NULL) ? (suffix - name) : strlen(name);
		if (name_len < 1 || name_len > 8) {
			// Name not between 1 and 8 characters
			return FR_INVALID_NAME;
		}

		// Validate the optional suffix part
		if (suffix) {
			// Skip past the period
			suffix++;
			// Reject multiple suffixes
			if (index(suffix, '.') != NULL) {
				// More than one suffix
				return FR_INVALID_NAME;
			}
			int suffix_len = strlen(suffix);
			if (suffix_len == 0) {
				// Remove a dangling suffix
				*(suffix - 1) = '\0';
			} else if (suffix_len > 3) {
				// Suffix too long
				return FR_INVALID_NAME;
			}
		}
	}

	// Make the path absolute
	if (*path == '/') {
		// absolute: append the path to the root directory path
		snprintf(newpath, PATHSIZE, "%s/%s", BaseMMCPath, path);
	} else {
		// relative: append the path to current directory path
		snprintf(newpath, PATHSIZE, "%s/%s", MMCPath, path);
	}
	return F_OK;
}

static BYTE file_exists(char *name)
{
	struct stat statbuf;

	//rpclog("file_exists(%s)\n",name);

	if(0==stat(name,&statbuf) && !S_ISDIR(statbuf.st_mode) )
		return FR_OK;
	else
		return FR_NO_PATH;
}

static BYTE dir_exists(char *name)
{
	struct stat statbuf;

	//rpclog("dir_exists(%s)\n",name);

	if(0==stat(name,&statbuf) && S_ISDIR(statbuf.st_mode) )
		return FR_OK;
	else
		return FR_NO_PATH;
}

static FRESULT validate(FIL *fp)
{
	if (!fp || !fp->fs) {
		return FR_INVALID_OBJECT;
	}
	return FR_OK;
}

static void update_FIL(FIL	*fil,
					   int	fp,
					   int	updatefp)
{
	struct stat statbuf;

	if((fp) || (0==updatefp))
	{
		if(updatefp)
			fil->fs=(FATFS *)(intptr_t)fp;
		else
			fp=(intptr_t)fil->fs;

		if(0==fstat(fp,&statbuf))
		{
			fil->fsize=statbuf.st_size;
		}
		fil->fptr=lseek(fp, 0, SEEK_CUR);
	}
}

static FRESULT get_result(int	err_no)
{
	//debuglog("get_result errno=%d [%04X]\n",err_no,err_no);
	switch(err_no)
	{
		case ENOENT:
			return FR_NO_PATH;
		case EACCES :
			return FR_DENIED;
		case EBUSY :
			return FR_DENIED;
		case EFAULT :
		case EIO :
			return FR_DISK_ERR;

		default :
			return FR_OK;
	}
}
FRESULT f_chdrive (
	BYTE drv		/* Drive number */
)
{
	return FR_OK;
}

FRESULT f_mount (
	BYTE vol,		/* Logical drive number to be mounted/unmounted */
	FATFS *fs		/* Pointer to new file system object (NULL for unmount)*/
)
{
	return FR_OK;
}

FRESULT f_chdir (
	const XCHAR *path	/* Pointer to the directory path */
)
{
	char	newpath[PATHSIZE+1];
	char	fullpath[PATHSIZE+1];

	FRESULT	res;
   	if ((res = build_absolute_path(path, newpath, 0)) != FR_OK) {
		return res;
	}

	// Resolve the newpath
	if(NULL!=saferealpath(newpath,fullpath))
	{
		// Check that the new path is BELOW the base mmcpath
		if(0==strncmp(BaseMMCPath,fullpath,strlen(BaseMMCPath)))
		{
			// And that it exists and is a directory
			if(dir_exists(fullpath) == FR_OK)
			{
				strcpy(MMCPath,fullpath);
				return FR_OK;
			}
		}
	}

	//rpclog("f_chdir(path)\nnewpath=%s\nfullpath=%s\nbasepath=%s\n",path,newpath,fullpath,BaseMMCPath);

	return FR_NO_PATH;
}

FRESULT f_open (
	FIL *fp,			/* Pointer to the blank file object */
	const XCHAR *path,	/* Pointer to the file name */
	BYTE mode			/* Access mode and file open mode flags */
)
{
	BYTE exists;
	char open_path[PATHSIZE+1];
	int open_mode=0;
	int newfile;

	// Get real path of file and check to see if it exists
	FRESULT res;
	if ((res = build_absolute_path(path, open_path, 1)) != FR_OK) {
		return res;
	}

	exists=file_exists(open_path);

	//debuglog("f_open(%s,%02X):exists=%d\n",open_path,mode,exists);

	mode &= (FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW);

	if(FR_OK==exists)
	{
		if(mode & FA_CREATE_NEW)
			return FR_EXIST;

		if(mode & FA_CREATE_ALWAYS)
			open_mode=O_CREAT;

		if(mode & (FA_READ | FA_WRITE))
		{
			if(mode & FA_WRITE)
				open_mode |= O_RDWR;
			else
				open_mode |= O_RDONLY;
		}
	}
	else
	{
		if(mode & (FA_OPEN_ALWAYS | FA_CREATE_NEW | FA_CREATE_ALWAYS))
			open_mode=O_CREAT | O_RDWR;
		else
			return FR_NO_FILE;
	}

	// To prevent leaking of file descriptors, leading to
	// files that cannot be deleted on Windows
	if (fp->fs) {
		f_close(fp);
	}

	newfile=open(open_path,open_mode | O_BINARY,S_IRWXU);
	update_FIL(fp,newfile,1);

	//debuglog("Openmode:%04X\n",open_mode);

	if (newfile>0)
	{
		openfil=fp;
		return FR_OK;
	}
	else
		return FR_INVALID_NAME;
}

FRESULT f_read (
	FIL *fp, 		/* Pointer to the file object */
	void *buff,		/* Pointer to data buffer */
	UINT btr,		/* Number of bytes to read */
	UINT *br		/* Pointer to number of bytes read */
)
{
	DWORD	ptrpos;
	int	bytesread;
	int	error;

	/* Check validity of the file object */
	FRESULT res;
	if ((res = validate(fp)) != FR_OK) {
		return res;
	}

	ptrpos=fp->fptr;
	bytesread=read((intptr_t)fp->fs,buff,btr);
	*br=bytesread;

	//debuglog("f_read(%d) offset=%d[%04X],result=%d\n",btr,ptrpos,ptrpos,*br);
//	HexDumpHead(buff,btr);

	update_FIL(fp,0,0);

	if(bytesread<0)
	{
		error=errno;
		return error;
	}

	return FR_OK;
}

FRESULT f_write (
	FIL *fp,			/* Pointer to the file object */
	const void *buff,		/* Pointer to the data to be written */
	UINT btw,			/* Number of bytes to write */
	UINT *bw			/* Pointer to number of bytes written */
)
{
	DWORD	ptrpos;
	int		written;
	int 	error;

	/* Check validity of the file object */
	FRESULT res;
	if ((res = validate(fp)) != FR_OK) {
		return res;
	}

	ptrpos=fp->fptr;

// SP9 START

	written=write((intptr_t)fp->fs,buff,btw);
	*bw=written;

	//debuglog("f_write(%d) offset=%d[%04X],result=%d\n",btw,ptrpos,ptrpos,written);
	//HexDumpHead(buff,btw);

	if(written<0)
	{
		error=errno;
		//debuglog("errno: %s [%d]\n",strerror(error),error);
		return error;		/* Return correct error for RAF */
	}

	update_FIL(fp,0,0);
	return FR_OK;
}

// SP9 END

FRESULT f_close (
	FIL *fp		/* Pointer to the file object to be closed */
)
{
	int result=0;

	if(fp->fs)
		result=close((intptr_t)fp->fs);

	//debuglog("f_close():result=%d\n",result);

	fp->fs=NULL;
	openfil=NULL;

	return FR_OK;
}

FRESULT f_unlink (
	const XCHAR *path		/* Pointer to the file or directory path */
)
{
	struct stat statbuf;
	char del_path[PATHSIZE+1];

/* CHANGED FOR SP4 */

	// Get real path of file and check to see if it exists
	FRESULT res;
	if ((res = build_absolute_path(path, del_path, 1)) != FR_OK) {
		return res;
	}

	// Check that path exists
	if (stat(del_path, &statbuf)) {
		return FR_NO_PATH;
	}

	// Try to delete it
	if (S_ISDIR(statbuf.st_mode)) {
      rmdir(del_path);
	} else {
      unlink(del_path);
	}

	// Check that path no longer exists
	if (stat(del_path, &statbuf)) {
		return FR_OK;
	} else {
		return FR_DENIED;
	}

/* END SP4*/

}

FRESULT f_opendir (
	DIR *dj,			/* Pointer to directory object to create */
	const XCHAR *path	/* Pointer to the directory path */
)
{
	char	newpath[PATHSIZE+1];

	//rpclog("f_opendir(%s)\n",path);

	FRESULT res;
	if ((res = build_absolute_path(path, newpath, 1)) != FR_OK) {
		return res;
	}

	if(findfirst(newpath,&emu))
		return FR_OK;
	else
		return FR_NO_PATH;
}

FRESULT f_readdir (
	DIR *dj,			/* Pointer to the open directory object */
	FILINFO *fno		/* Pointer to file information to return */
)
{
	// If a file found copy it's details, else set size to 0 and filename to ''
	if(findnext(&emu))
	{
		fno->fsize=emu.fsize;
		fno->fattrib=emu.fattrib;
		strncpy(fno->fname,emu.filename,FNAMELEN);
	}
	else
	{
		fno->fsize=0;
		fno->fname[0]=0;
	}

	return FR_OK;
}


FRESULT f_lseek (
	FIL *fp,		/* Pointer to the file object */
	DWORD ofs		/* File pointer from top of file */
)
{
	/* Check validity of the file object */
	FRESULT res;
	if ((res = validate(fp)) != FR_OK) {
		return res;
	}
	lseek((intptr_t)fp->fs,ofs,SEEK_SET);
	update_FIL(fp,0,0);
	return FR_OK;
}


FRESULT f_sync (
    FIL* fp
)
{
   // There is no need to call fflush here because f_write uses write()
   // which is unbuffered.
}

FRESULT f_rename (
	const XCHAR *path_old,	/* Pointer to the old name */
	const XCHAR *path_new	/* Pointer to the new name */
)
{
   char full_path_old[PATHSIZE+1];
   char full_path_new[PATHSIZE+1];

   FRESULT res;
   if ((res = build_absolute_path(path_old, full_path_old, 1)) != FR_OK) {
	   return res;
   }
   if ((res = build_absolute_path(path_new, full_path_new, 1)) != FR_OK) {
	   return res;
   }
   //rpclog("f_rename(%s, %s)\n", full_path_old, full_path_new);
   if (rename(full_path_old, full_path_new) == 0) {
      return FR_OK;
   } else {
      return get_result(errno);
   }
}

FRESULT f_mkdir (
	const XCHAR *path		/* Pointer to the directory path */
)
{
   char full_path[PATHSIZE+1];

   FRESULT res;
   if ((res = build_absolute_path(path, full_path, 1)) != FR_OK) {
	   return res;
   }

   //rpclog("f_mkdir(%s)\n", full_path);

   if (file_exists(full_path) == FR_OK) {
      return FR_EXIST;
   }
   if (dir_exists(full_path) == FR_OK) {
      return FR_EXIST;
   }

#ifdef WIN32
   if (mkdir(full_path) == 0) {
#else
   if (mkdir(full_path, 0755) == 0) {
#endif
      return FR_OK;
   } else {
      return get_result(errno);
   }
}

static
void get_fileinfo (		/* No return code */
	DIR *dj,			/* Pointer to the directory object */
	FILINFO *fno	 	/* Pointer to the file information to be filled */
)
{
}

void get_fileinfo_special(FILINFO *fno)
{
//   get_fileinfo(&dj, fno);

	//rpclog("get_fileinfo_special()\n");
	if(NULL!=openfil)
	{

		fno->fsize	= openfil->fsize;
//		fno->fptr	= openfil->fptr;
		fno->fdate	= 0;
		fno->ftime	= 0;
		fno->fattrib= get_fat_attribs((intptr_t)openfil->fs);
		//rpclog("size=%d, attr=%d\n",fno->fsize,fno->fattrib);
	}
}

#ifdef DEBUGFF
void HexDump(const void 	*Buff,
				   int 		Length)
{
	char		LineBuff[80];
	char		*LineBuffPos;
	int			LineOffset;
	int			CharOffset;
	BYTE		*BuffPtr;

	BuffPtr=Buff;

	for(LineOffset=0;LineOffset<Length;LineOffset+=16, BuffPtr+=16)
	{
		LineBuffPos=LineBuff;
		LineBuffPos+=sprintf(LineBuffPos,"%4.4X ",LineOffset);

		for(CharOffset=0;CharOffset<16;CharOffset++)
		{
			if((LineOffset+CharOffset)<Length)
				LineBuffPos+=sprintf(LineBuffPos,"%02X ",BuffPtr[CharOffset]);
			else
			    LineBuffPos+=sprintf(LineBuffPos,"   ");
		}

		for(CharOffset=0;CharOffset<16;CharOffset++)
		{
			if((LineOffset+CharOffset)<Length)
			{
				if(isprint(BuffPtr[CharOffset]))
					LineBuffPos+=sprintf(LineBuffPos,"%c",BuffPtr[CharOffset]);
				else
					LineBuffPos+=sprintf(LineBuffPos," ");
			}
			else
				LineBuffPos+=sprintf(LineBuffPos,".");
		}
		rpclog("%s\n",LineBuff);
	}
	rpclog("\n\n");
}

void HexDumpHead(const void 	*Buff,
					   int 		Length)
{
	rpclog("Addr 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ASCII\n");
	rpclog("----------------------------------------------------------\n");

	HexDump(Buff,Length);
};
#endif
