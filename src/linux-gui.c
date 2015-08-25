/*Atomulator v1.0 by Tom Walker
   Linux GUI*/

#ifndef WIN32
#include <string.h>
#include <allegro.h>
#include <stdio.h>
//#include <alleggl.h>
#include "atom.h"
#include "roms.h"
#include "resources.h"
#include "sidtypes.h"

#undef printf

int oldf11 = 1;
int resetit;
int palit;

int timerspeeds[] 	= { 5, 12, 25, 38, 50, 75, 85, 100, 150, 200, 250 };
int frameskips[] 	= { 0,  0,  0,  0,  0,  0,  0,   1,   2,   3,   4 };
int emuspeed = 4;
int fskipmax = 0;

char ejecttext[2][260] = { "Eject disc :0/2", "Eject disc :1/3" };

void setejecttext(int drive, char *fn)
{
	if (fn[0])
		sprintf(ejecttext[drive], "Eject drive :%i/%i - %s", drive, drive + 2, get_filename(fn));
	else
		sprintf(ejecttext[drive], "Eject drive :%i/%i", drive, drive + 2);
}

void setquit()
{
}

extern int fullscreen;
extern int quited;
int windx = 512, windy = 384;
extern int dcol;
extern int ddtype, ddvol, sndddnoise;

MENU filemenu[4];
MENU discmenu[8];
MENU tapespdmenu[3];
MENU tapemenu[5];
MENU videomenu[5];
MENU ddtypemenu[3];
MENU ddvolmenu[4];
MENU soundmenu[8];
MENU keymenu[3];
MENU hardmenu[2];
MENU settingsmenu[7];
MENU miscmenu[3];
MENU speedmenu[11];
MENU mainmenu[6];

MENU sidtypemenu[15];
MENU methodmenu[3];
MENU ramrommenu[2];

void updatelinuxgui()
{
	int x;

	discmenu[4].flags = (writeprot[0]) ? D_SELECTED : 0;
	discmenu[5].flags = (writeprot[1]) ? D_SELECTED : 0;
	discmenu[6].flags = (defaultwriteprot) ? D_SELECTED : 0;

	tapespdmenu[0].flags = (!fasttape) ? D_SELECTED : 0;
	tapespdmenu[1].flags = (fasttape) ? D_SELECTED : 0;

	videomenu[0].flags = (fullscreen) ? D_SELECTED : 0;
	videomenu[1].flags = (snow) ? D_SELECTED : 0;
	videomenu[2].flags = (colourboard) ? D_SELECTED : 0;
	videomenu[3].flags = (palnotntsc) ? D_SELECTED : 0;

	soundmenu[0].flags = (spon) ? D_SELECTED : 0;
	soundmenu[1].flags = (sndatomsid)?D_SELECTED:0;
	soundmenu[2].flags = (tpon) ? D_SELECTED : 0;
	soundmenu[3].flags = (sndddnoise) ? D_SELECTED : 0;

	hardmenu[0].flags = (RR_jumpers & RAMROM_FLAG_BBCMODE) ? D_SELECTED : 0;

	ddtypemenu[0].flags = (!ddtype) ? D_SELECTED : 0;
	ddtypemenu[1].flags = (ddtype) ? D_SELECTED : 0;
	for (x = 0; x < 3; x++)
		ddvolmenu[x].flags = (ddvol == (int)ddvolmenu[x].dp) ? D_SELECTED : 0;

	for (x = 0; x < 10; x++)
		speedmenu[x].flags = (emuspeed == (int)speedmenu[x].dp) ? D_SELECTED : 0;

	for (x=0;x<14;x++) 
		sidtypemenu[x].flags=(cursid==(int)sidtypemenu[x].dp)?D_SELECTED:0;

	settingsmenu[5].flags = joyst ? D_SELECTED : 0;

	methodmenu[0].flags=(!sidmethod)?D_SELECTED:0;
    methodmenu[1].flags=(sidmethod)?D_SELECTED:0;
	
	ramrommenu[0].flags=(ramrom_enable) ? D_SELECTED : 0;
	ramrommenu[1].flags=(RR_jumpers & RAMROM_FLAG_DISKROM) ? D_SELECTED : 0;	
}

int gui_keydefine();

int gui_return()
{
	return D_CLOSE;
}

int gui_reset()
{
	resetit = 1;
	return D_O_K;
}

int gui_exit()
{
	quited = 1;
	return D_CLOSE;
}

MENU filemenu[4] =
{
	{ "&Return",	 gui_return, NULL, 0, NULL },
	{ "&Hard reset", gui_reset,  NULL, 0, NULL },
	{ "&Exit",	 gui_exit,   NULL, 0, NULL },
	{ NULL,		 NULL,	     NULL, 0, NULL }
};

int gui_load0()
{
	char tempname[260];
	int ret;
	int xsize = windx - 32, ysize = windy - 16;

	memcpy(tempname, discfns[0], 260);
	ret = file_select_ex("Please choose a disc image", tempname, "DSK;SSD;DSD;IMG;FDI", 260, xsize, ysize);
	if (ret)
	{
		closedisc(0);
		memcpy(discfns[0], tempname, 260);
		loaddisc(0, discfns[0]);
		if (defaultwriteprot)
			writeprot[0] = 1;
	}
	updatelinuxgui();
	return D_O_K;
}

int gui_load1()
{
	char tempname[260];
	int ret;
	int xsize = windx - 32, ysize = windy - 16;

	memcpy(tempname, discfns[1], 260);
	ret = file_select_ex("Please choose a disc image", tempname, "DSK;SSD;DSD;IMG;FDI", 260, xsize, ysize);
	if (ret)
	{
		closedisc(1);
		memcpy(discfns[1], tempname, 260);
		loaddisc(1, discfns[1]);
		if (defaultwriteprot)
			writeprot[1] = 1;
	}
	updatelinuxgui();
	return D_O_K;
}

int gui_eject0()
{
	closedisc(0);
	discfns[0][0] = 0;
	return D_O_K;
}
int gui_eject1()
{
	closedisc(1);
	discfns[1][0] = 0;
	return D_O_K;
}

int gui_wprot0()
{
	writeprot[0] = !writeprot[0];
	if (fwriteprot[0])
		fwriteprot[0] = 1;
	updatelinuxgui();
	return D_O_K;
}
int gui_wprot1()
{
	writeprot[1] = !writeprot[1];
	if (fwriteprot[1])
		fwriteprot[1] = 1;
	updatelinuxgui();
	return D_O_K;
}
int gui_wprotd()
{
	defaultwriteprot = !defaultwriteprot;
	updatelinuxgui();
	return D_O_K;
}

MENU discmenu[8] =
{
	{ "Load disc :&0/2...",	     gui_load0,	 NULL, 0, NULL },
	{ "Load disc :&1/3...",	     gui_load1,	 NULL, 0, NULL },
	{ ejecttext[0],		     gui_eject0, NULL, 0, NULL },
	{ ejecttext[1],		     gui_eject1, NULL, 0, NULL },
	{ "Write protect disc :0/2", gui_wprot0, NULL, 0, NULL },
	{ "Write protect disc :1/3", gui_wprot1, NULL, 0, NULL },
	{ "Default write protect",   gui_wprotd, NULL, 0, NULL },
	{ NULL,			     NULL,	 NULL, 0, NULL }
};

char tapefn[260];

int gui_loadt()
{
	char tempname[260];
	int ret;
	int xsize = windx - 32, ysize = windy - 16;

	memcpy(tempname, tapefn, 260);
	ret = file_select_ex("Please choose a tape image", tempname, "UEF;CSW", 260, xsize, ysize);
	if (ret)
	{
		closeuef();
		closecsw();
		memcpy(tapefn, tempname, 260);
		loadtape(tapefn);
//                tapeloaded=1;
	}
	return D_O_K;
}

int gui_rewind()
{
	closeuef();
	closecsw();
	loadtape(tapefn);
	return D_O_K;
}

int gui_ejectt()
{
	closeuef();
	closecsw();
//        tapeloaded=0;
	return D_O_K;
}

int gui_normal()
{
	fasttape = 0;
	updatelinuxgui();
	return D_O_K;
}
int gui_fast()
{
	fasttape = 1;
	updatelinuxgui();
	return D_O_K;
}

MENU tapespdmenu[3] =
{
	{ "Normal", gui_normal, NULL, 0, NULL },
	{ "Fast",   gui_fast,	NULL, 0, NULL },
	{ NULL,	    NULL,	NULL, 0, NULL }
};

MENU tapemenu[] =
{
	{ "Load tape...", gui_loadt,  NULL,	   0, NULL },
	{ "Eject tape",	  gui_ejectt, NULL,	   0, NULL },
	{ "Rewind tape",  gui_rewind, NULL,	   0, NULL },
	{ "Tape speed",	  NULL,	      tapespdmenu, 0, NULL },
	{ NULL,		  NULL,	      NULL,	   0, NULL }
};

int gui_fullscreen()
{
	if (fullscreen)
	{
		fullscreen = 0;
		leavefullscreen();
	}
	else
	{
		fullscreen = 1;
		enterfullscreen();
	}
	return D_EXIT;
}

int gui_snow()
{
	snow = !snow;
	updatelinuxgui();
	return D_O_K;
}

int gui_colour()
{
	colourboard = !colourboard;
	updatelinuxgui();
	palit = 1;
	return D_O_K;
}

int gui_palnotntsc()
{
	palnotntsc = !palnotntsc;
	updatelinuxgui();
	return D_O_K;
}

MENU videomenu[5] =
{
	{ "Fullscreen", gui_fullscreen, NULL, 0, NULL },
	{ "Snow",	gui_snow,	NULL, 0, NULL },
	{ "&Colour board", gui_colour, NULL, 0, NULL },
	{ "&Pal (50Hz)", gui_palnotntsc, NULL, 0, NULL },
	{ NULL,		NULL,		NULL, 0, NULL }
};

int gui_sidtype()
{
        cursid=(int)active_menu->dp;
        sid_settype(sidmethod, cursid);
        updatelinuxgui();
        return D_O_K;
}

MENU sidtypemenu[15]=
{
        {"6581",                    gui_sidtype,NULL,0,(void *)SID_MODEL_6581},
        {"8580",                    gui_sidtype,NULL,0,(void *)SID_MODEL_8580},
        {"8580 + digi boost",       gui_sidtype,NULL,0,(void *)SID_MODEL_8580D},
        {"6581R4",                  gui_sidtype,NULL,0,(void *)SID_MODEL_6581R4},
        {"6581R3 4885",             gui_sidtype,NULL,0,(void *)SID_MODEL_6581R3_4885},
        {"6581R3 0486S",            gui_sidtype,NULL,0,(void *)SID_MODEL_6581R3_0486S},
        {"6581R3 3984",             gui_sidtype,NULL,0,(void *)SID_MODEL_6581R3_3984},
        {"6581R4AR 3789",           gui_sidtype,NULL,0,(void *)SID_MODEL_6581R4AR_3789},
        {"6581R3 4485",             gui_sidtype,NULL,0,(void *)SID_MODEL_6581R3_4485},
        {"6581R4 1986S",            gui_sidtype,NULL,0,(void *)SID_MODEL_6581R4_1986S},
        {"8580R5 3691",             gui_sidtype,NULL,0,(void *)SID_MODEL_8580R5_3691},
        {"8580R5 3691 + digi boost",gui_sidtype,NULL,0,(void *)SID_MODEL_8580R5_3691D},
        {"8580R5 1489",             gui_sidtype,NULL,0,(void *)SID_MODEL_8580R5_1489},
        {"8580R5 1489 + digi boost",gui_sidtype,NULL,0,(void *)SID_MODEL_8580R5_1489D},
        {NULL,NULL,NULL,0,NULL}
};

int gui_method()
{
        sidmethod=(int)active_menu->dp;
        sid_settype(sidmethod, cursid);
        updatelinuxgui();
        return D_O_K;
}

MENU methodmenu[3]=
{
        {"Interpolating", gui_method,NULL,0,(void *)0},
        {"Resampling",    gui_method,NULL,0,(void *)1},
        {NULL,NULL,NULL,0,NULL}
};

MENU residmenu[3]=
{
        {"Model",NULL,sidtypemenu,0,NULL},
        {"Sample method",NULL,methodmenu,0,NULL},
        {NULL,NULL,NULL,0,NULL}
};

int gui_ddtype()
{
	ddtype = (int)active_menu->dp;
	closeddnoise();
	loaddiscsamps();
	updatelinuxgui();
	return D_O_K;
}

MENU ddtypemenu[3] =
{
	{ "5.25", gui_ddtype, NULL, 0, (void*)0 },
	{ "3.5",  gui_ddtype, NULL, 0, (void*)1 },
	{ NULL,	  NULL,	      NULL, 0, NULL	}
};

int gui_ddvol()
{
	ddvol = (int)active_menu->dp;
	updatelinuxgui();
	return D_O_K;
}

MENU ddvolmenu[4] =
{
	{ "33%",  gui_ddvol, NULL, 0, (void*)1 },
	{ "66%",  gui_ddvol, NULL, 0, (void*)2 },
	{ "100%", gui_ddvol, NULL, 0, (void*)3 },
	{ NULL,	  NULL,	     NULL, 0, NULL     }
};

int gui_internalsnd()
{
	spon = !spon;
	updatelinuxgui();
	return D_O_K;
}

int gui_atomsid()
{
        sndatomsid=!sndatomsid;
        updatelinuxgui();
        return D_O_K;
}

int gui_tnoise()
{
	tpon = !tpon;
	updatelinuxgui();
	return D_O_K;
}

int gui_ddnoise()
{
	sndddnoise = !sndddnoise;
	updatelinuxgui();
	return D_O_K;
}

MENU soundmenu[8] =
{
	{ "Internal speaker",  	gui_internalsnd, 	NULL,	    0, 	NULL },
	{ "Tape noise",	       	gui_tnoise,			NULL,	    0, 	NULL },
	{ "AtomSID",			gui_atomsid,		NULL,		0,	NULL},
    { "reSID configuration",NULL,				residmenu,	0,	NULL},
    { "Disc drive noise",  	gui_ddnoise,		NULL,	    0, 	NULL },
	{ "Disc drive type",   	NULL,				ddtypemenu, 0, 	NULL },
	{ "Disc drive volume", 	NULL,				ddvolmenu,  0, 	NULL },
	{ NULL,		       		NULL,				NULL,	    0, 	NULL }
};

int gui_keydefault()
{
	int c;

	for (c = 0; c < 128; c++)
		keylookup[c] = c;
	return D_O_K;
}

MENU keymenu[3] =
{
	{ "Redefine keyboard", gui_keydefine,  NULL, 0, NULL },
	{ "Default mapping",   gui_keydefault, NULL, 0, NULL },
	{ NULL,		       NULL,	       NULL, 0, NULL }
};

int gui_bbc()
{
	RR_jumpers ^= RAMROM_FLAG_BBCMODE;
	resetit = 1;
	updatelinuxgui();
	return D_O_K;
}

int gui_joystk_en()
{
	joyst = !joyst;
	updatelinuxgui();
	return D_O_K;
}

MENU hardmenu[2] =
{
	{ "&BBC BASIC",	   gui_bbc,    NULL, 0, NULL },
	{ NULL,		   NULL,       NULL, 0, NULL }
};

MENU settingsmenu[7] =
{
	{ "&Video",    	NULL, videomenu, 0, NULL },
	{ "&Hardware", 	NULL, hardmenu,	0, NULL },
	{ "&RamRom",	NULL, ramrommenu, 0, NULL},
	{ "&Sound",    	NULL, soundmenu, 0, NULL },
	{ "&Keyboard", 	NULL, keymenu,	0, NULL },
	{ "Joystick PORTB", 	gui_joystk_en, NULL, 0, NULL },
	{ NULL,	       	NULL, NULL,	0, NULL }
};

int gui_ramrom_en()
{
	ramrom_enable = !ramrom_enable;
	resetit = 1;
	updatelinuxgui();
	return D_O_K;
}

int gui_ramromdsk_en()
{
	RR_jumpers ^= RAMROM_FLAG_DISKROM;
	resetit = 1;
	updatelinuxgui();
	return D_O_K;
}

MENU ramrommenu[2] =
{
	{ "RAM/ROM enabled",			gui_ramrom_en,		NULL,	0,	NULL},
	{ "RAM/ROM disk rom enabled",	gui_ramromdsk_en,	NULL,	0,	NULL},
};

int gui_scrshot()
{
	char tempname[260];
	int ret, c;
	int xsize = windx - 32, ysize = windy - 16;

	tempname[0] = 0;
	ret = file_select_ex("Please enter filename", tempname, "BMP", 260, xsize, ysize);
	if (ret)
	{
		memcpy(scrshotname, tempname, 260);
		printf("Save scrshot\n");
		savescrshot = 1;
	}
	return D_CLOSE;
}

int gui_speed()
{
	emuspeed = (int)active_menu->dp;
	changetimerspeed(timerspeeds[emuspeed]);
	fskipmax = frameskips[emuspeed];
	updatelinuxgui();
	return D_O_K;
}

MENU speedmenu[11] =
{
	{ "&10%",  gui_speed, NULL, 0, (void*)0 },
	{ "&25%",  gui_speed, NULL, 0, (void*)1 },
	{ "&50%",  gui_speed, NULL, 0, (void*)2 },
	{ "&75%",  gui_speed, NULL, 0, (void*)3 },
	{ "&100%", gui_speed, NULL, 0, (void*)4 },
	{ "&150%", gui_speed, NULL, 0, (void*)5 },
	{ "&200%", gui_speed, NULL, 0, (void*)6 },
	{ "&300%", gui_speed, NULL, 0, (void*)7 },
	{ "&400%", gui_speed, NULL, 0, (void*)8 },
	{ "&500%", gui_speed, NULL, 0, (void*)9 },
	{ NULL,	   NULL,      NULL, 0, NULL	}
};

MENU miscmenu[3] =
{
	{ "&Speed",	     NULL,	  speedmenu, 0, NULL },
	{ "Save screenshot", gui_scrshot, NULL,	     0, NULL },
	{ NULL,		     NULL,	  NULL,	     0, NULL }
};

MENU mainmenu[6] =
{
	{ "&File",     NULL, filemenu,	   0, NULL },
	{ "&Tape",     NULL, tapemenu,	   0, NULL },
	{ "&Disc",     NULL, discmenu,	   0, NULL },
	{ "&Settings", NULL, settingsmenu, 0, NULL },
	{ "&Misc",     NULL, miscmenu,	   0, NULL },
	{ NULL,	       NULL, NULL,	   0, NULL }
};

DIALOG bemgui[] =
{
	{ d_ctext_proc, 200, 260, 0,   0,  15, 0, 0, 0, 0,     0, "Atomulator V1.24" },
	{ d_menu_proc,	0,   0,	  0,   0,  15, 0, 0, 0, 0,     0, mainmenu	    },
	{ d_yield_proc },
	{ 0,		0,   0,	  0,   0,  0,  0, 0, 0, 0,     0, NULL, NULL, NULL  }
};

BITMAP *mouse, *_mouse_sprite;

void entergui()
{
	int x = 1;
	DIALOG_PLAYER *dp;

	resetit = palit = 0;

	while (keypressed())
		readkey();
//        while (key[KEY_F11]) rest(100);

	updatelinuxgui();

	install_mouse();


	set_color_depth(desktop_color_depth());
	show_mouse(screen);
	if (fullscreen)
	{
		bemgui[0].x = (1024 / 2) - 36;
		bemgui[0].y = 768 - 8;
	}
	else
	{
		bemgui[0].x = (windx / 2) - 36;
		bemgui[0].y = windy - 8;
	}
	bemgui[0].fg = makecol(255, 255, 255);
	dp = init_dialog(bemgui, 0);
	while (x && !(key[KEY_F11] && !oldf11) && !key[KEY_ESC])
	{
		oldf11 = key[KEY_F11];
		x = update_dialog(dp);
	}
	shutdown_dialog(dp);
	show_mouse(NULL);
	set_color_depth(8);

	remove_mouse();

//        while (key[KEY_F11]) rest(100);

	clearscreen();
	if (palit)
		updatepal();

	if (resetit)
		atom_reset(0);
}
#endif
