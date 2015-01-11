/*Atomulator v1.0 by Tom Walker
   CSW file handling*/

#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include "atom.h"

void findfilenamescsw();
int cswena = 0;
int cintone = 1, cindat = 0, datbits = 0, enddat = 0;
FILE *cswf;
char csws[256];
FILE *cswlog;
int cswrate;
uint8_t *cswdat;
int cswpoint;
uint8_t cswhead[0x34];
int tapelcount, tapellatch, pps;
int cswskip = 0;
int tapespeed;
float cswmul;

void opencsw(char *fn)
{
	int end, c;
	uint32_t destlen = 8 * 1024 * 1024;
	uint8_t *tempin;

	if (cswf)
		fclose(cswf);
	if (cswdat)
		free(cswdat);
	cswena = 1;
	/*Allocate buffer*/
	cswdat = malloc(8 * 1024 * 1024);
	/*Open file and get size*/
	cswf = fopen(fn, "rb");
	fseek(cswf, -1, SEEK_END);
	end = ftell(cswf);
	fseek(cswf, 0, SEEK_SET);
	/*Read header*/
	fread(cswhead, 0x34, 1, cswf);
	for (c = 0; c < cswhead[0x23]; c++)
		getc(cswf);
	cswrate = cswhead[0x19] | (cswhead[0x1A] << 8) | (cswhead[0x1B] << 16) | (cswhead[0x1C] << 24);
//        rpclog("CSWRATE %i\n",cswrate);
	cswmul = 4000000.0f / (float)cswrate;
//        cswmul*=4;
	/*Allocate temporary memory and read file into memory*/
	end -= ftell(cswf);
	tempin = malloc(end);
	fread(tempin, end, 1, cswf);
	fclose(cswf);
	/*Decompress*/
	uncompress(cswdat, (uLongf*)&destlen, tempin, end);
	free(tempin);
	/*Reset data pointer*/
	cswpoint = 0;
	dcd();
	tapellatch = (1000000 / (1200 / 10)) / 64;
	tapelcount = 0;
	pps = 120;
//        clearcatwindow();
	findfilenamescsw();
//        cataddname("No catalogue for CSW files yet, sorry!");
//        getcswfnames();

}

void closecsw()
{
	if (cswf)
		fclose(cswf);
	if (cswdat)
		free(cswdat);
	cswf = (FILE*)NULL;
	cswdat = (uint8_t*)NULL;
}

int getcsw()
{
	while (!cswdat[cswpoint])
	{
		cswpoint++;
		if (cswpoint >= (8 * 1024 * 1024))
		{
			cswpoint = 0;
			break;
		}
	}
//        rpclog("%i %i\n",cswdat[cswpoint],(int)((float)cswdat[cswpoint]*cswmul));
	return (int)((float)cswdat[cswpoint++] * cswmul);
}

int fpoint = 0;
int header, cinbyte, cnextbit;
int cover;
int output;

uint8_t findcswbyte()
{
	int d, c = 0;
	int count = 0;
	uint8_t byte = 0;
	int bitsleft = 0;

//        rpclog("Getting fnames\n");
	while (fpoint < (8 * 1024 * 1024))
	{
		d = (int)((float)cswdat[fpoint++] * cswmul);
//                if (output) rpclog("%i - %i %02X  %i %i %i\n",fpoint,d,byte,c,bitsleft,count);
		if (header < 1000)
		{
			if (d > 1200)
				header = 0;
			else
				header++;
//                        if (header==1000) rpclog("2 seconds of header found!\n");
		}
		else if (!cinbyte)
		{
			if (d > 1200) /*0*/
			{
//                                rpclog("Start bit!\n");
//                                output=500;
				cinbyte = 1;
				bitsleft = 9;
				count = 0;
				c = 1;
				count = d;
			}
			else if (cnextbit)
			{
				c++;
				if (c >= 16)
				{
					cnextbit = 0;
					header = 0;
//                                        rpclog("Back to header\n");
				}
			}
		}
		else
		{
			c++;
			count += d;
			if (count >= 12500 /*13056*//*(3333*4)*/)
			{
				count = 0;
				if (c >= 12)
					byte = (byte >> 1) | 0x80;
				else
					byte >>= 1;
				c = 0;
				bitsleft--;
				if (!bitsleft)
				{
//                                        rpclog("Found byte %02X %c",byte,byte);
//                                        rpclog("\n");
					cinbyte = 0;
					c = 0;
					return byte;
//                                        nextbit=1;
/*                                        fbuffer[0]=fbuffer[1];
                                        fbuffer[1]=fbuffer[2];
                                        fbuffer[2]=fbuffer[3];
                                        fbuffer[3]=byte;
                                        if (fbuffer[0]==0x2A && fbuffer[1]==0x2A && fbuffer[2]==0x2A && fbuffer[3]==0x2A)
                                        {
                                                rpclog("Found file header!\n");
                                        }*/
				}
			}
		}
	}
	cover = 1;
	return 0;
}

#define getcswbyte() fdat = findcswbyte(); if (cover) return

void findfilenamescsw()
{
	uint8_t fbuffer[4];
	uint8_t fdat, status;
	char ffilename[16];
	int c;
	int tb, offset, run, load;
	char s[100];
	int skip;

//        clearcatwindow();
	cover = 0;
	header = 0;
	cinbyte = cnextbit = 0;
	fpoint = 0;
	while (!cover)
	{
		fbuffer[0] = fbuffer[1];
		fbuffer[1] = fbuffer[2];
		fbuffer[2] = fbuffer[3];
		fbuffer[3] = findcswbyte();
		if (fbuffer[0] == 0x2A && fbuffer[1] == 0x2A && fbuffer[2] == 0x2A && fbuffer[3] == 0x2A)
		{
//                        rpclog("Found file header!\n");
			//output=1;
			fbuffer[3] = 0;
			c = 0;
//                        printf("Found file!\n");
//                        printf("Filename : ");
			do
			{
				getcswbyte();
				ffilename[c++] = fdat;
			}
			while (fdat != 0xD && c < 15);
			c--;
			while (c < 13)
				ffilename[c++] = 32;
			ffilename[c] = 0;
//                        rpclog("Filename %s\n",ffilename);
			getcswbyte();
			status = fdat;
//                        rpclog("Next byte %02X\n",fdat);
//                                printf("%s ",ffilename);
			getcswbyte();
			tb = fdat;
			getcswbyte();
//                                printf("Block %02X%02X ",tb,fdat);
			c = ((tb * 256) + fdat) * 256;
			offset = ((tb * 256) + fdat) * 256;
			getcswbyte();
			c += fdat;
			skip = fdat;
//                                printf("Size %04X ",c);
			getcswbyte();
			tb = fdat;
			getcswbyte();
//                                printf("Run %02X%02X ",tb,fdat);
			run = (tb << 8) | fdat;
			getcswbyte();
			tb = fdat;
			getcswbyte();
//                                printf("Load %02X%02X\n",tb,fdat);
			load = (tb << 8) | fdat;
			load -= offset;
			if (!(status & 0x80))
			{
				sprintf(s, "%s Size %04X Load %04X Run %04X", ffilename, c, load, run);
				cataddname(s);
			}
			for (c = 0; c < skip; c++)
				getcswbyte();

			output = 0;
		}
	}
}
