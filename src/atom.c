/*Atomulator v1.0 by Tom Walker
   Main loop*/

#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include "atom.h"
#include "atommc.h"
#include "1770.h"

char exedir[MAXPATH+1];

int palnotntsc = 0;
int colourboard = 1;
int sndddnoise = 1;

// Flag to force autoboot on power on break
int autoboot = 0;
extern unsigned char configByte;

// SID
int sndatomsid=1;
int cursid=0;
int sidmethod=0;

// SP3 FOR JOYSTICK SUPPORT

int joyst = 0;

// END SP3

// SP10 FOR KEYBOARDJOYSTICK SUPPORT

int keyjoyst = 0;

// END SP10

// GDOS2015
int fdc1770 = 0;
int GD_bank		= 0;	// Which of the 16 banks is jumpered.
// end GDOS2015

// RAM config
int main_ramflag	= 3;	// 25K by default (when RAMROM *DISABLED!*).
int vid_ramflag		= 7;	// 8K by default
int vid_top;			//= ((vid_ramflag+1)*0x0400)+0x8000;	// Last video RAM address.
// RAM config

int ramrom_enable = 1;

int fasttape = 0;
char discfn[260];

FILE *rlog;
void rpclog(char *format, ...)
{
	char buf[256];

//   return;
	
	sprintf(buf,"%s/rlog.txt",exedir);
	
	if (!rlog)
		rlog = fopen(buf, "wt");
//turn;
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	fputs(buf, rlog);
	fflush(rlog);
}

/*SP7 CHANGES*/

FILE *plog;
void prtbuf(char *format, ...)
{
	char buf[256];

//   return;
	
	sprintf(buf,"%s/pbuf.txt",exedir);
	
	if (!plog)
		plog = fopen(buf, "wt");
//turn;
	va_list ap;
	va_start(ap, format);
	vsprintf(buf, format, ap);
	va_end(ap);
	fputs(buf, plog);
	fflush(plog);
}

/*END SP7*/

int tapeon;
uint8_t *ram;
char filelist[256][17];
int filelistsize;
int filepos[256];

int drawscr;
int ddframes = 0;

void scrupdate()
{
	ddframes++;
	drawscr++;
}

void atom_reset(int power_on)
{
	debuglog("atom_reset(%d)\n",power_on);
//	memset(ram, 0, 0x10000);
	if(power_on)
	{
		ram[8] = rand();
		ram[9] = rand();
		ram[10] = rand();
		ram[11] = rand();
		
		// Clear BBC basic workspace.
		// if(bbcmode)
		//	memset(&ram[0], 0, 0x10000);
	}
	resetvia();
	sid_reset();
	debuglog("exedir=%s\n",exedir);
	InitMMC();

	// If autoboot has been set (via the command line)
	// clear bit 6 of the configByte
	if (power_on && autoboot) {
		configByte &= ~0x40;
	}

	if(fdc1770)
		reset1770();
	else
		reset8271();
		
	gfxmode=0;
	reset6502();
	debuglog("atom_reset():done\n");

      // SP3 FOR JOYSTICK SUPPORT

      install_joystick(JOY_TYPE_AUTODETECT);

      // END SP3
}

void atom_init(int argc, char **argv)
{
	int c;
	int tapenext = 0, discnext = 0;

	for (c = 1; c < argc; c++)
	{
		if (!strcasecmp(argv[c], "--help"))
		{
			printf("%s command line options :\n\n",ATOMULATOR_VERSION);
			printf("-disc disc.ssd  - load disc.ssd into drives :0/:2\n");
			printf("-disc1 disc.ssd - load disc.ssd into drives :1/:3\n");
			printf("-tape tape.uef  - load tape.uef\n");
			printf("-fasttape       - set tape speed to fast\n");
			printf("-autoboot       - cause atommc to auto boot on power on reset\n");
			printf("-debug          - start debugger\n");
			exit(-1);
		}
		else
		if (!strcasecmp(argv[c], "-tape"))
		{
			tapenext = 2;
		}
		else if (!strcasecmp(argv[c], "-disc") || !strcasecmp(argv[c], "-disk"))
		{
			discnext = 1;
		}
		else if (!strcasecmp(argv[c], "-disc1"))
		{
			discnext = 2;
		}
		else if (!strcasecmp(argv[c], "-fasttape"))
		{
			fasttape = 1;
		}
		else if (!strcasecmp(argv[c], "-autoboot"))
		{
			autoboot = 1;
		}
		else if (!strcasecmp(argv[c], "-debug"))
		{
			debug = debugon = 1;
		}
		else if (tapenext)
			strcpy(tapefn, argv[c]);
		else if (discnext)
		{
			strcpy(discfns[discnext - 1], argv[c]);
			discnext = 0;
		}
		else
		{
			strcpy(discfns[0], argv[c]);
			discnext = 0;
		}
		if (tapenext)
			tapenext--;
	}
	initalmain(0, NULL);
	inituef();
	initmem();
	loadroms();
	reset6502();
	initvideo();
	init8255();
	disc_reset();
	reset8271();
	resetvia();
	#ifndef WIN32
	install_keyboard();
	#endif
	//install_mouse();
	install_timer();
	install_int_ex(scrupdate, BPS_TO_TIMER(300));
	inital();
	sid_init();
	sid_settype(sidmethod, cursid);
	loaddiscsamps();
	loaddisc(0, discfns[0]);
	loaddisc(1, discfns[1]);
	atom_reset(1);
}

void changetimerspeed(int i)
{
	remove_int(scrupdate);
	install_int_ex(scrupdate, BPS_TO_TIMER(i * 6));
}

int oldf12 = 0;
void atom_run()
{
	if ((drawscr > 0) || (tapeon && fasttape))
	{
		if (palnotntsc)
			exec6502(312, 64);
		else
			exec6502(262, 64);
			
		poll_keyboard();

// SP10 FOR KEYBOARDJOYSTICK SUPPORT

                poll_joystick();

//SP10 END

		if (tapeon && fasttape)
			drawscr = 0;
		else
			drawscr -= (palnotntsc) ? 6 : 5;
		
		if (drawscr > 25)
			drawscr = 0;
		
		if (ddframes >= 25)
		{
			ddframes -= 25;
			mixddnoise();
		}
		
		if (key[KEY_F12] && !oldf12)
			atom_reset(0);
		
		oldf12 = key[KEY_F12];
	}
	else
		rest(1);
}

void atom_exit()
{
	stopmovie();
	saveconfig();
	closeddnoise();
	FinalizeMMC();
//        dumpregs();
//        dumpram();
}

