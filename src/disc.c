/*Atomulator v1.0 by Tom Walker
   Disc support (also some tape)*/

#include <allegro.h>
#include <string.h>
#include <stdio.h>
#include "atom.h"

int motoron = 0, fdctime = 0, motorspin = 0, curdrive = 0;

struct
{
	char *ext;
	void (*load)(int drive, char *fn);
	void (*close)(int drive);
	int size;
}
loaders[] =
{
	{ "DSK", ssd_load, ssd_close, 40 * 10 * 256	},
	{ "SSD", ssd_load, ssd_close, 80 * 10 * 256	},
	{ "DSD", dsd_load, ssd_close, 2 * 80 * 10 * 256 },
	{ "FDI", fdi_load, fdi_close, -1		},
	{ 0,	 0,	   0 }
};

int driveloaders[2];

void loaddisc(int drive, char *fn)
{
	int c = 0;
	char *p;
	FILE *f;

	setejecttext(drive, "");
	if (!fn)
		return;
	p = get_extension(fn);
	if (!p)
		return;
	setejecttext(drive, fn);
	rpclog("Loading :%i %s %s\n", drive, fn, p);
	while (loaders[c].ext)
	{
		if (!strcasecmp(p, loaders[c].ext))
		{
			driveloaders[drive] = c;
			loaders[c].load(drive, fn);
			return;
		}
		c++;
	}
//        printf("Couldn't load %s %s\n",fn,p);
	/*No extension match, so guess based on image size*/
	f = fopen(fn, "rb");
	if (!f)
		return;
	fseek(f, -1, SEEK_END);
	c = ftell(f) + 1;
	fclose(f);
	rpclog("Size %i\n", c);
	if (c <= (200 * 1024)) /*200k DFS - 80*1*10*256*/
	{
		driveloaders[drive] = 0;
		loaders[0].load(drive, fn);
		return;
	}
	if (c <= (400 * 1024)) /*400k DFS - 80*2*10*256*/
	{
		driveloaders[drive] = 1;
		loaders[1].load(drive, fn);
		return;
	}
}

void newdisc(int drive, char *fn)
{
	int c = 0, d;
	FILE *f;
	char *p = get_extension(fn);

	while (loaders[c].ext)
	{
		if (!strcasecmp(p, loaders[c].ext) && loaders[c].size != -1)
		{
			f = fopen(fn, "wb");
			for (d = 0; d < loaders[c].size; d++)
			{
				if (d == 0x106)
					putc(1, f);
				else if (d == 0x107)
					putc(0x90, f);
				else
					putc(0, f);
			}
			fclose(f);
			loaddisc(drive, fn);
			return;
		}
		c++;
	}
}

void closedisc(int drive)
{
	if (loaders[driveloaders[drive]].close)
		loaders[driveloaders[drive]].close(drive);
}

int disc_notfound = 0;

void disc_reset()
{
	drives[0].poll = drives[1].poll = 0;
	drives[0].seek = drives[1].seek = 0;
	drives[0].readsector = drives[1].readsector = 0;
	curdrive = 0;
}

void disc_poll()
{
	if (drives[curdrive].poll)
		drives[curdrive].poll();
	if (disc_notfound)
	{
		disc_notfound--;
		if (!disc_notfound)
			fdcnotfound();
	}
}

int oldtrack[2] = { 0, 0 };
void disc_seek(int drive, int track)
{
	if (drives[drive].seek)
		drives[drive].seek(drive, track);
	ddnoise_seek(track - oldtrack[drive]);
	oldtrack[drive] = track;
}

void disc_readsector(int drive, int sector, int track, int side, int density)
{
//		rpclog("disc_readsector(%d,%d,%d,%d,%d)\n",drive, sector, track, side, density);

	if (drives[drive].readsector)
		drives[drive].readsector(drive, sector, track, side, density);
	else
		disc_notfound = 10000;
}

void disc_writesector(int drive, int sector, int track, int side, int density)
{
	if (drives[drive].writesector)
		drives[drive].writesector(drive, sector, track, side, density);
	else
		disc_notfound = 10000;
}

void disc_readaddress(int drive, int track, int side, int density)
{
	if (drives[drive].readaddress)
		drives[drive].readaddress(drive, track, side, density);
	else
		disc_notfound = 10000;
}

void disc_format(int drive, int track, int side, int density)
{
	if (drives[drive].format)
		drives[drive].format(drive, track, side, density);
	else
		disc_notfound = 10000;
}


void loadtape(char *fn)
{
	char *p;

//        rpclog("loadtape\n");
	if (!fn)
		return;
//        rpclog("get_extension\n");
//        if (fn[0]==0) return;
	p = get_extension(fn);
	if (!p)
		return;
//        rpclog("do load %c\n",p[0]);
	if (p[0] == 'u' || p[0] == 'U')
		openuef(fn);
	else
		opencsw(fn);
}

