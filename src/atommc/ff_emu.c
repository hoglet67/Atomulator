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

#define SHORT_NAME_LEN	12

DIR 	dj;
EMUDIR	emu;
char	MMCPath[PATHSIZE+1];
char	BaseMMCPath[PATHSIZE+1];
FIL		*openfil;
char	openname[SHORT_NAME_LEN+1];

#ifdef DEBUGFF
void HexDump(const void 	*Buff, 
					   int 		Length); 

void HexDumpHead(const void 	*Buff, 
					   int 		Length); 

#endif

static BYTE file_exists(char	name[])
{
	struct stat statbuf;

rpclog("file_exists(%s)\n",name);

	if(0==stat(name,&statbuf))
		return FR_OK;
	else
		return FR_NO_PATH;
}

static void update_FIL(FIL	*fil,
					   int	fp,
					   int	updatefp)
{
	struct stat statbuf;
	
	if((fp) || (0==updatefp))
	{
		if(updatefp)
			fil->fs=(FATFS *)fp;
		else
			fp=(int)fil->fs;
		
		if(0==fstat(fp,&statbuf))
		{
			fil->fsize=statbuf.st_size;
		}
		fil->fptr=lseek(fp, 0, SEEK_CUR);
	}
}	

static FRESULT get_result(int	err_no)
{
	debuglog("get_result errno=%d [%04X]\n",err_no,err_no);
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
	FRESULT	result = FR_NO_PATH;
	
	// add new directory to current
	snprintf(newpath,PATHSIZE,"%s/%s",MMCPath,path);
	
	// Resolve the newpath 
	if(NULL!=saferealpath(newpath,fullpath))
	{
		// Check that the new path is BELOW the base mmcpath
		if(0==strncmp(BaseMMCPath,fullpath,strlen(BaseMMCPath)))
		{
			// And that it exists
			if(0==access(fullpath,F_OK))
			{
				strcpy(MMCPath,fullpath);
				result=FR_OK;
			}
		}
	}

	//rpclog("f_chdir(path)\nnewpath=%s\nfullpath=%s\nbasepath=%s\n",path,newpath,fullpath,BaseMMCPath);
	
	return result;
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
	snprintf(open_path,PATHSIZE,"%s/%s",MMCPath,path);
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
	
	ptrpos=fp->fptr;
	*br=read((int)fp->fs,buff,btr);
	
//	rpclog("f_read(%d) offset=%d[%04X],result=%d\n",btr,ptrpos,ptrpos,*br);
//	HexDumpHead(buff,btr);
	
	update_FIL(fp,0,0);
	
	if(*br>=0)
	{
		return FR_OK;
	}
}

FRESULT f_write (
	FIL *fp,			/* Pointer to the file object */
	const void *buff,	/* Pointer to the data to be written */
	UINT btw,			/* Number of bytes to write */
	UINT *bw			/* Pointer to number of bytes written */
)
{
	DWORD	ptrpos;
	int		written;
	int 	error;
	
	ptrpos=fp->fptr;
	written=write((int)fp->fs,buff,btw);
	*bw=written;

	debuglog("f_write(%d) offset=%d[%04X],result=%d\n",btw,ptrpos,ptrpos,written);
//	HexDumpHead(buff,btw);
	
	if(written<0)
	{
		error=errno;
		debuglog("errno: %s [%d]\n",strerror(error),error);
	}
	
	if(*bw>=0)
	{
		update_FIL(fp,0,0);
		
		return FR_OK;
	}
}

FRESULT f_close (
	FIL *fp		/* Pointer to the file object to be closed */
)
{
	int result=0;
	
	if(0!=(int)fp->fs)
		result=close((int)fp->fs);
	
	//debuglog("f_close():result=%d\n",result);
	
	fp->fs=NULL;
	openfil=NULL;
	
	return FR_OK;
}

FRESULT f_unlink (
	const XCHAR *path		/* Pointer to the file or directory path */
)
{
	char del_path[PATHSIZE+1];
	int open_mode=0;

/* CHANGED FOR SP4 */

	int result=0;

	int newfile;
	
	// Get real path of file and check to see if it exists
	snprintf(del_path,PATHSIZE,"%s/%s",MMCPath,path);

	//debuglog("f_unlink(%s)\n",del_path);

      result=unlink(del_path);

      if (result == 0)
         return FR_OK;
      else

      return get_result(errno);

/* END SP4*/

}

FRESULT f_opendir (
	DIR *dj,			/* Pointer to directory object to create */
	const XCHAR *path	/* Pointer to the directory path */
)
{
	char	newpath[PATHSIZE+1];
	
	rpclog("f_opendir(%s)\n",path);
	
	snprintf(newpath,PATHSIZE,"%s/%s",MMCPath,path);
	
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
	lseek((int)fp->fs,ofs,SEEK_SET);
	
	return FR_OK;
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

rpclog("get_fileinfo_special()\n");
	if(NULL!=openfil)
	{
		
		fno->fsize	= openfil->fsize;
//		fno->fptr	= openfil->fptr;
		fno->fdate	= 0;
		fno->ftime	= 0;
		fno->fattrib= get_fat_attribs((int)openfil->fs);
rpclog("size=%d, attr=%d\n",fno->fsize,fno->fattrib);
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