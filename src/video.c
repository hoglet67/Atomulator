/*Atomulator v1.0 by Tom Walker
   6847 video emulation*/

#include <stdio.h>
#include <allegro.h>
#include <zlib.h>
#include "atom.h"

void saveframe();
void stopmovie();

int fullscreen = 0;
int winsizex = 512, winsizey = 384;

uint8_t fontdata[] =
{
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x02, 0x1a, 0x2a, 0x2a, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x14, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x12, 0x12, 0x1c, 0x12, 0x12, 0x3c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x20, 0x20, 0x20, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x12, 0x12, 0x12, 0x12, 0x12, 0x3c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x20, 0x20, 0x3c, 0x20, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x20, 0x20, 0x3c, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1e, 0x20, 0x20, 0x26, 0x22, 0x22, 0x1e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x3e, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x24, 0x28, 0x30, 0x28, 0x24, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x36, 0x2a, 0x2a, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x32, 0x2a, 0x26, 0x22, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x22, 0x22, 0x3c, 0x20, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x22, 0x2a, 0x24, 0x1a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3c, 0x22, 0x22, 0x3c, 0x28, 0x24, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x10, 0x08, 0x04, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x2a, 0x2a, 0x36, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x14, 0x22, 0x22, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x22, 0x22, 0x14, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x20, 0x20, 0x10, 0x08, 0x04, 0x02, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x1c, 0x2a, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x3e, 0x10, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x14, 0x14, 0x36, 0x00, 0x36, 0x14, 0x14, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x1e, 0x20, 0x1c, 0x02, 0x3c, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x32, 0x32, 0x04, 0x08, 0x10, 0x26, 0x26, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x28, 0x28, 0x10, 0x2a, 0x24, 0x1a, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x1c, 0x3e, 0x1c, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x3e, 0x08, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x10, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x18, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x02, 0x1c, 0x20, 0x20, 0x3e, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x02, 0x0c, 0x02, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x04, 0x0c, 0x14, 0x3e, 0x04, 0x04, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x20, 0x3c, 0x02, 0x02, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x20, 0x20, 0x3c, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x3e, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x1c, 0x22, 0x22, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1c, 0x22, 0x22, 0x1e, 0x02, 0x02, 0x1c, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x24, 0x04, 0x08, 0x08, 0x00, 0x08, 0x00, 0x00,
};

PALETTE atompal =
{
	{ 0,  0,  0  }, /*Black*/
	{ 0,  63, 0  }, /*Green*/
	{ 63, 63, 0  }, /*Yellow*/
	{ 0,  0,  63 }, /*Blue*/
	{ 63, 0,  0  }, /*Red*/
	{ 63, 63, 63 }, /*Buff*/
	{ 0,  63, 63 }, /*Cyan*/
	{ 63, 0,  63 }, /*Magenta*/

/* CHANGED FOR SP4 */

	{ 63, 32,  0  }, /*Orange - actually red on the Atom*/

/* END SP4 */

};

PALETTE monopal =
{
	{ 8,  8,  8  }, /*Black*/
	{ 55, 55, 55 }, /*Green*/
	{ 63, 63, 63 }, /*Yellow*/
	{ 32, 32, 32 }, /*Blue*/
	{ 32, 32, 32 }, /*Red*/
	{ 63, 63, 63 }, /*Buff*/
	{ 55, 55, 55 }, /*Cyan*/
	{ 55, 55, 55 }, /*Magenta*/
	{ 55, 55, 55 }, /*Orange - actually red on the Atom*/
};

BITMAP *b, *b2;
int depth;
void initvideo()
{
	depth = desktop_color_depth();
	set_color_depth(depth);

	#ifdef WIN32
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 2048, 2048, 0, 0);
	#else
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 512, 384, 0, 0);
	#endif

	#ifdef WIN32
	b2 = create_video_bitmap(256, 192);
	#else
	b2 = create_bitmap(256, 192);
	#endif

	set_color_depth(8);
	b = create_bitmap(256, 192);

//	if (colourboard)
//		set_palette(atompal);
//	else
//		set_palette(monopal);
	updatepal();
	set_color_depth(depth);

        /* Clear the sound stream buffer before we start filling it. */
        memset(sndstreambuf, 0, sizeof(sndstreambuf));
}

void updatepal()
{
	if (colourboard)
		set_palette(atompal);
	else
		set_palette(monopal);
}

uint8_t *ram;
int cy = 0, sy = 0;
int textcol[4] = { 0, 1, 0, 8 };
int semigrcol[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
int grcol[4] = { 0, 1, 0, 5 };
int tapeon;
int frmcount;
int fskipcount = 0;

char scrshotname[260];
int savescrshot = 0;

char moviename[260];
uint8_t sndstreambuf[626];
int sndstreamindex = 0;
int sndstreamcount = 0;
int wantmovieframe=0;
FILE *moviefile;
BITMAP *moviebitmap;

uint8_t fetcheddat[32];

void drawline(int line)
{
	int addr, chr, col;
	int x, xx;
	uint8_t temp;

	if (!line)
		vbl = cy = sy = 0;
		
	if (line < 192)
	{
		switch (gfxmode)
		{
		case 0: case 2: case 4: case 6:         /*Text mode*/
		case 8: case 10: case 12: case 14:
			addr = (cy << 5) + 0x8000;
			for (x = 0; x < 256; x += 8)
			{
				chr = fetcheddat[x >> 3];
				if (chr & 0x40)
				{
					temp = chr;
					chr <<= ((sy >> 2) << 1);
					chr = (chr >> 4) & 3;
					if (chr & 2)
						col = semigrcol[(temp >> 6) | (css << 1)];
					else
						col = 0;
					b->line[line][x] = b->line[line][x + 1] = b->line[line][x + 2] = b->line[line][x + 3] = col;
					if (chr & 1)
						col = semigrcol[(temp >> 6) | (css << 1)];
					else
						col = 0;
					b->line[line][x + 4] = b->line[line][x + 5] = b->line[line][x + 6] = b->line[line][x + 7] = col;
				}
				else
				{
					chr = ((chr & 0x3F) * 12) + sy;
					if (fetcheddat[x >> 3] & 0x80)
					{
						for (xx = 0; xx < 8; xx++)
						{
							b->line[line][x + xx] = textcol[(((fontdata[chr] >> (xx ^ 7)) & 1) ^ 1) | css];
						}
					}
					else
					{
						for (xx = 0; xx < 8; xx++)
						{
							b->line[line][x + xx] = textcol[((fontdata[chr] >> (xx ^ 7)) & 1) | css];
						}
					}
				}
			}
			sy++;
			if (sy == 12)
			{
				sy = 0;
				cy++;
			}
			addr = (cy << 5) + 0x8000;
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr++];
			break;
		
		/* Propper graphics modes */
		case 1:         /*64x64, 4 colours*/
			for (x = 0; x < 256; x += 16)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 16; xx += 4)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = b->line[line][x + xx + 2] = b->line[line][x + xx + 3] = semigrcol[(temp >> 6) | (css << 1)];
					temp <<= 2;
				}
			}
			
			addr = (((line + 1) / 3) << 4) | 0x8000;
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + (x >> 1)];
			
			break;
			
		case 3:         /*128x64, 2 colours*/
			for (x = 0; x < 256; x += 16)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 16; xx += 2)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = (temp & 0x80) ? grcol[css | 1] : grcol[css];
					temp <<= 1;
				}
			}
			
			addr = (((line + 1) / 3) << 4) | 0x8000;
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + (x >> 1)];
			
			break;

/* PATCH FOR CORRECT CLEAR2a */

		case 5: /*128x64, 4 colours*/
			for (x = 0; x < 256; x += 8)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 8; xx += 2)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = semigrcol[(temp >> 6) |(css << 1)];
					temp <<= 2;
				}
			}

			addr = (((line + 1) / 3) << 5) | 0x8000;
			for (x = 0; x < 32; x++)
			fetcheddat[x] = ram[addr + x];
			break;

/* PATCH CHANGES */

		case 7:         /*128x96, 2 colours*/
			for (x = 0; x < 256; x += 16)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 16; xx += 2)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = (temp & 0x80) ? grcol[css | 1] : grcol[css];
					temp <<= 1;
				}
			}

			addr = (((line + 1) >> 1) << 4) | 0x8000;
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + (x >> 1)];

			break;

		case 9:         /*128x96, 4 colours*/
			for (x = 0; x < 256; x += 8)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 8; xx += 2)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = semigrcol[(temp >> 6) | (css << 1)];
					temp <<= 2;
				}
			}

			addr = (((line + 1) >> 1) << 5) | 0x8000;

			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + x];

			break;

		case 11:         /*128x192, 2 colours*/
			for (x = 0; x < 256; x += 16)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 16; xx += 2)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = (temp & 0x80) ? grcol[css | 1] : grcol[css];
					temp <<= 1;
				}
			}

			addr = ((line + 1) << 4) | 0x8000;
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + (x >> 1)];

			break;

		case 13:         /*128x192, 4 colours*/
			for (x = 0; x < 256; x += 8)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 8; xx += 2)
				{
					b->line[line][x + xx] = b->line[line][x + xx + 1] = semigrcol[(temp >> 6) | (css << 1)];
					temp <<= 2;
				}
			}

			addr = ((line + 1) << 5) | 0x8000;

			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + x];

			break;

		case 15:         /*256x192, 2 colours*/
			for (x = 0; x < 256; x += 8)
			{
				temp = fetcheddat[x >> 3];
				for (xx = 0; xx < 8; xx++)
				{
					b->line[line][x + xx] = (temp & 0x80) ? grcol[css | 1] : grcol[css];
					temp <<= 1;
				}
			}

			addr = ((line + 1) << 5) | 0x8000;
			// rpclog("addr=%04X\n",addr);
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[addr + x];

			break;

//                        default:
//                        printf("Bad GFX mode %i\n",gfxmode);
//                        dumpregs();
//                        dumpram();
//                        exit(-1);
		}
	}

	if (line == 192)
	{
		startblit();
		frmcount++;
		fskipcount++;

		if (savescrshot)
		{
			savescrshot = 0;
			if (colourboard)
				save_bmp(scrshotname, b, atompal);
			else
				save_bmp(scrshotname, b, monopal);
		}

		if ((!(tapeon && fasttape) && fskipcount >= fskipmax) || frmcount == 60)
		{
			fskipcount = 0;
			blit(b, b2, 0, 0, 0, 0, 256, 192);
			if (fullscreen)
			{
				stretch_blit(b2, screen, 0, 0, 256, 192, 0, 0, 1024, 768);
				if (tapeon)
					rectfill(screen, 1000, 0, 1023, 8, makecol(255, 0, 0));
			}
			else
			{
				stretch_blit(b2, screen, 0, 0, 256, 192, 0, 0, winsizex, winsizey-1);
				if (tapeon)
					rectfill(screen, winsizex - 12, 0, winsizex, 4, makecol(255, 0, 0));
			}

			frmcount = 0;
		}
                if (wantmovieframe) saveframe();
		endblit();
	}

	if (line == 192)
		vbl = 1;

	if (line == 224)
		vbl = 0;

	if ((line == 261 && !palnotntsc) || line == 311)
	{
		switch (gfxmode)
		{

/* PATCH FOR CORRECT CLEAR2a */

		case 0: case 2: case 4: case 6:         /*Text mode*/
		case 8: case 10: case 12: case 14:
		case 5: case 9: case 13: case 15:
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[0x8000 + x];
			break;
/* END PATCH */

		case 1: case 3: case 7: case 11:         /*16-byte per line*/
			for (x = 0; x < 32; x++)
				fetcheddat[x] = ram[0x8000 + (x >> 1)];
			break;

		}
	}

//        sndbuffer[line]=(speaker)?255:0;
}

/*void mixaudio(uint8_t *p)
   {
        memcpy(p,sndbuffer,262);
   }*/

void enterfullscreen()
{
/*	if (opengl)
        {
                rpclog("Enter fullscreen start\n");
                openglreinit();
                rpclog("Enter fullscreen end\n");
                return;
        }*/
	#ifdef WIN32
	destroy_bitmap(b2);
	#endif

	set_color_depth(depth);
	set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, 1024, 768, 0, 0);

	#ifdef WIN32
	b2 = create_video_bitmap(256, 192);
	#endif

	set_color_depth(8);
	updatepal();
//	if (colourboard)
//		set_palette(atompal);
//	else
//		set_palette(monopal);
}

void leavefullscreen()
{
/*	if (opengl)
        {
                openglreinit();
                return;
        }*/
	#ifdef WIN32
	destroy_bitmap(b2);
	#endif

	set_color_depth(depth);

#ifdef WIN32
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 2048, 2048, 0, 0);
#else
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 512, 384, 0, 0);
#endif

	#ifdef WIN32
	b2 = create_video_bitmap(256, 192);
	#endif

	set_color_depth(8);
	updatepal();
//	if (colourboard)
//		set_palette(atompal);
//	else
//		set_palette(monopal);
	updatewindowsize(512, 384);
}

void startmovie()
{
    stopmovie();

    wantmovieframe = 1;
    moviefile = fopen(moviename, "wb");
    if (moviefile == NULL)
        return;

    moviebitmap = create_bitmap_ex(8, 512, 384);
    sndstreamindex = 0;
    sndstreamcount = 0;
}

void stopmovie()
{
    wantmovieframe = 0;
    if (moviefile != NULL) {
        fclose(moviefile);
        destroy_bitmap(moviebitmap);
        moviefile = NULL;
    }
}

#define DEFLATE_CHUNK_SIZE 262144

int deflate_bitmap(int level)
{
    unsigned int have;
    z_stream strm;
    unsigned char in[DEFLATE_CHUNK_SIZE];
    unsigned char out[DEFLATE_CHUNK_SIZE];

    /* Allocate the deflate state. */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    if (deflateInit(&strm, level) != Z_OK)
        return Z_ERRNO;

    /* Compress the bitmap buffer. */
    strm.avail_in = 512*384;
    strm.next_in = moviebitmap->dat;

    /* Run deflate() on the bitmap buffer, finishing the compression. */
    strm.avail_out = DEFLATE_CHUNK_SIZE;
    strm.next_out = out;
    if (deflate(&strm, Z_FINISH) == Z_STREAM_ERROR)
        return Z_ERRNO;

    /* Write the length of the data. */
    have = DEFLATE_CHUNK_SIZE - strm.avail_out;
    fwrite(&have, sizeof(unsigned int), 1, moviefile);

    if (fwrite(out, 1, have, moviefile) != have || ferror(moviefile)) {
        deflateEnd(&strm);
        return Z_ERRNO;
    }

    /* clean up and return */
    deflateEnd(&strm);
    return Z_OK;
}

void saveframe()
{
    if (moviefile == NULL)
        return;

    int start;
    if (sndstreamcount == 624) {
        /* Take the last 625 samples. */
        start = (sndstreamindex + 1) % sizeof(sndstreambuf);
    } else if (sndstreamcount == 626) {
        /* Take the first 625 samples from the 626 obtained and leave the last
           one for the next frame. */
        start = sndstreamindex;
    }

    stretch_blit(b, moviebitmap, 0, 0, 256, 192, 0, 0, 512, 384);

    if (deflate_bitmap(6) != Z_OK) {
        stopmovie();
        return;
    }

    int remaining = sizeof(sndstreambuf) - start;
    if (remaining >= 625)
        fwrite(&sndstreambuf[start], 1, 625, moviefile);
    else {
        fwrite(&sndstreambuf[start], 1, remaining, moviefile);
        fwrite(sndstreambuf, 1, 625 - remaining, moviefile);
    }

    sndstreamcount = 0;
}

void clearscreen()
{
	clear(screen);
//        clear(b);
	clear(b2);
}
