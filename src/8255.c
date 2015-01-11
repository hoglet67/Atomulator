/*Atomulator v1.0 by Tom Walker
   8255 PIA + keyboard + sound emulation*/
#include <string.h>
#include <allegro.h>
#include "atom.h"

int16_t sndbuffer[312 * 2 * 5];
int sndpos = 0;

/*SWARM - CTRL=LEFT, ADJ=RIGHT, REPT=FIRE*/
/*PINBALL - SHIFT,REPT*/
int output;
int tapecyc;
int inchunk;
int intone = 0, tapedat, hightone = 0;
int bytevalid = 0, bitvalid = 0;
int tapeon = 0;
uint16_t databyte;

uint16_t pc;

int keyl[128];
int keys[16][6] =
{
	{ 0,		  KEY_3,	 KEY_MINUS,	KEY_G,	    KEY_Q, KEY_ESC },
	{ 0,		  KEY_2,	 KEY_COMMA,	KEY_F,	    KEY_P, KEY_Z   },
	{ KEY_UP,	  KEY_1,	 KEY_SEMICOLON, KEY_E,	    KEY_O, KEY_Y   },
	{ KEY_RIGHT,	  KEY_0,	 KEY_QUOTE,	KEY_D,	    KEY_N, KEY_X   },
	{ KEY_CAPSLOCK,	  KEY_BACKSPACE, KEY_9,		KEY_C,	    KEY_M, KEY_W   },
	{ KEY_TAB,	  KEY_END,	 KEY_8,		KEY_B,	    KEY_L, KEY_V   },
	{ KEY_CLOSEBRACE, KEY_ENTER,	 KEY_7,		KEY_A,	    KEY_K, KEY_U   },
	{ KEY_BACKSLASH,  0,		 KEY_6,		KEY_EQUALS, KEY_J, KEY_T   },
	{ KEY_OPENBRACE,  0,		 KEY_5,		KEY_SLASH,  KEY_I, KEY_S   },
	{ KEY_SPACE,	  0,		 KEY_4,		KEY_STOP,   KEY_H, KEY_R   }
};

void init8255()
{
	int c, d;

	memset(keyl, 0, sizeof(keyl));
	for (c = 0; c < 16; c++)
	{
		for (d = 0; d < 6; d++)
		{
			keyl[keys[c][d]] = c | (d << 4) | 0x80;
		}
	}
}

/*int keys2[16][6]=
   {
        {0,KEY_3,KEY_MINUS,KEY_G,KEY_Q,KEY_ESC},
        {0,KEY_2,KEY_COMMA,KEY_F,KEY_P,KEY_Z},
        {KEY_DEL,KEY_1,KEY_COLON,KEY_E,KEY_O,KEY_Y},
        {KEY_RIGHT,KEY_0,KEY_QUOTE,KEY_D,KEY_N,KEY_X},
        {KEY_CAPSLOCK,KEY_BACKSPACE,KEY_9,KEY_C,KEY_M,KEY_W},
        {KEY_TAB,KEY_END,KEY_8,KEY_B,KEY_L,KEY_V},
        {KEY_CLOSEBRACE,KEY_ENTER,KEY_7,KEY_A,KEY_K,KEY_U},
        {KEY_BACKSLASH,0,KEY_6,KEY_EQUALS,KEY_J,KEY_T},
        {KEY_OPENBRACE,0,KEY_5,KEY_SLASH,KEY_I,KEY_S},
        {KEY_SPACE,0,KEY_4,KEY_STOP,KEY_H,KEY_R}
   };*/


int keyrow;
void write8255(uint16_t addr, uint8_t val)
{
	int oldgfx = gfxmode;
	
	switch (addr & 0xF)
	{
	case 0:
		keyrow = val & 0xF;
//                if (gfxmode!=(val>>4)) printf("GFX mode now %02X %04X\n",val,pc);
		gfxmode = (val >> 4) & 0x0F;
		if(gfxmode!=oldgfx)
			debuglog("gfxmode changed at PC=%04X from %02X to %02X\n",pc,oldgfx,gfxmode);
//                printf("GFX mode now %02X %04X\n",val,pc);
//                printf("Keyrow now %i %04X\n",keyrow,pc);
		break;
	case 2:
		css = (val & 8) >> 2;
		speaker = val & 4;
//		rpclog("Speaker %i\n", (val & 4) >> 2);
		break;
	case 3:
		switch (val & 0xE)
		{
		case 0x4: 
			speaker = val & 1;                 
			//rpclog("Speaker %i\n", (val & 4) >> 2); 
			break;
			
		case 0x6: 
			css = (val & 1) ? 2 : 0; 
			break;
		}
		break;
//                        rpclog("8255 port 3 %02X\n",val);
	}
//        printf("Write 8255 %04X %02X\n",addr,val);
}

uint8_t read8255(uint16_t addr)
{
	uint8_t temp = 0xFF;
	int c;

//        printf("Read 8255 %04X %04X\n",addr,pc);
	switch (addr & 3)
	{
	case 0:
		return (keyrow & 0x0F) | ((gfxmode << 4) & 0xF0);
	case 1:
		for (c = 0; c < 128; c++)
		{
			if (key[c] && keyl[keylookup[c]] & 0x80 && keyrow == (keyl[keylookup[c]] & 0xF))
				temp &= ~(1 << ((keyl[keylookup[c]] & 0x70) >> 4));
			if (key[c] && keylookup[c] == KEY_LCONTROL)
				temp &= ~0x40;
			if (key[c] && (keylookup[c] == KEY_LSHIFT || keylookup[c] == KEY_RSHIFT))
				temp &= ~0x80;
		}
/*                if (key[keys[keyrow][0]]) temp&=~1;
                if (key[keys[keyrow][1]]) temp&=~2;
                if (key[keys[keyrow][2]]) temp&=~4;
                if (key[keys[keyrow][3]]) temp&=~8;
                if (key[keys[keyrow][4]]) temp&=~0x10;
                if (key[keys[keyrow][5]]) temp&=~0x20;
                if (key[keys2[keyrow][0]]) temp&=~1;
                if (key[keys2[keyrow][1]]) temp&=~2;
                if (key[keys2[keyrow][2]]) temp&=~4;
                if (key[keys2[keyrow][3]]) temp&=~8;
                if (key[keys2[keyrow][4]]) temp&=~0x10;
                if (key[keys2[keyrow][5]]) temp&=~0x20;
   //                rpclog("temp=%02X\n",temp);*/
//                if (key[keylookup[KEY_LCONTROL]] || key[keylookup[KEY_RCONTROL]]) temp&=~0x40;
//                if (key[keylookup[KEY_LSHIFT]] || key[keylookup[KEY_RSHIFT]]) temp&=~0x80;
		return temp;
	case 2:
		if (vbl)
			temp &= ~0x80;
		if (key[KEY_ALT] || key[KEY_ALTGR])
			temp &= ~0x40;
		if (!css)
			temp &= ~8;
		if (!speaker)
			temp &= ~4;
		if (!intone)
			temp &= ~0x10;
		if (!tapedat)
			temp &= ~0x20;
//                printf("VBL %i %04X\n",vbl,pc);
		return temp;
//                default:
//                printf("Read 8255 %04X\n",addr);
	}
	return 0;
}

void polltape()
{
	if (cswena)
	{
		if (tapeon)
		{
			tapecyc += getcsw();
			tapedat = !tapedat;
		}
	}
	else
	{
		tapecyc += 794;
		intone ^= 0x10;
		if (tapeon)
		{
			if (hightone)
			{
				hightone--;
				tapedat = hightone & 1;
				inchunk = 0;
				if (!hightone)
					polluef();
			}
			else if (bytevalid)
			{
				if (databyte & 1)
					tapedat = bitvalid & 1;
				else
					tapedat = bitvalid & 2;
				bitvalid--;
				if (!bitvalid)
				{
					bytevalid--;
					databyte >>= 1;
					if (!bytevalid)
						polluef();
					else
					{
						bitvalid = 16;
					}
				}
			}
			else
				polluef();
		}
	}
}

void dcdlow()
{
	hightone = 0;
//        printf("High tone off\n");
}

void dcd()
{
	hightone = 15000;
//        printf("High tone on\n");
}

void receive(uint8_t dat)
{
	bytevalid = 10;
	bitvalid = 16;
	databyte = (dat << 1) | 0x200;
//        rpclog("Recieved byte %02X - first bit %i\n",dat,databyte&1);
	lastdat = dat;
}

void pollsound()
{
	int16_t temp = 0;

	if (sndatomsid)
	{
//		sid_fillbuf(&sndbuffer[sndpos << 1],2);
		sid_fillbuf(&temp,2);
	}

	if (spon)
		temp += (speaker) ? 4095 : -4096;
		
	if (tpon)
		temp += (tapedat) ? 2047 : -2048;

	if (0!=temp)
	{
		sndbuffer[sndpos++] = temp;
		sndbuffer[sndpos++] = temp;
	}

	if (sndpos >= (312 * 2 * 5))
	{
		sndpos = 0;
		givealbuffer(sndbuffer);
	}
}
