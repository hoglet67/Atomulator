/*Atomulator v1.0 by Tom Walker
   Windows main*/

#ifdef WIN32
#include <process.h>
#include <stdint.h>
#include <stdio.h>
#include <allegro.h>
#include <winalleg.h>
#include "resources.h"
#include "atom.h"
#include "roms.h"
#include "debugger.h"
#include "sid_atom.h"

HWND hwndCat;

int infocus = 1;

HWND ghwnd;
char szClassName[]  = "WindowsApp";
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void updatewindowsize(int x, int y)
{
	RECT r;

	if (x < 128)
		x = 128;
	if (y < 64)
		y = 64;
	if (x == winsizex && y == winsizey)
		return;
	winsizex = x;
	winsizey = y;
	GetWindowRect(ghwnd, &r);
	MoveWindow(ghwnd, r.left, r.top,
		   x + (GetSystemMetrics(SM_CXFIXEDFRAME) * 2),
		   y + (GetSystemMetrics(SM_CYFIXEDFRAME) * 2) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYCAPTION) + 1,
		   TRUE);
}


int createwindow(HINSTANCE hThisInstance, int nFunsterStil)
{
	WNDCLASSEX wincl;        /* Data structure for the windowclass */

	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;            /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                       /* Catch double-clicks */
	wincl.cbSize = sizeof(WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon(hThisInstance, "allegro_icon");
	wincl.hIconSm = LoadIcon(hThisInstance, "allegro_icon");
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;                      /* No menu */
	wincl.cbClsExtra = 0;                           /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                           /* structure or the window instance */
	/* Use Windows's default color as the background of the window */
	wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx(&wincl))
		return -1;
	/* The class is registered, let's create the program*/
	ghwnd = CreateWindowEx(
		0,                                                                                                                      /* Extended possibilites for variation */
		szClassName,                                                                                                            /* Classname */

/*SP10 CHANGES*/

//		"Atomulator v1.25"
		ATOMULATOR_VERSION,                                                                                                      /* Title Text */

/*END SP10*/

		WS_OVERLAPPEDWINDOW | WS_VISIBLE,                                                                                       /* default window */
		CW_USEDEFAULT,                                                                                                          /* Windows decides the position */
		CW_USEDEFAULT,                                                                                                          /* where the window ends up on the screen */
		512 + (GetSystemMetrics(SM_CXFIXEDFRAME) * 2),                                                                          /* The programs width */
		384 + (GetSystemMetrics(SM_CYFIXEDFRAME) * 2) + GetSystemMetrics(SM_CYMENUSIZE) + GetSystemMetrics(SM_CYCAPTION) + 2,   /* and height in pixels */
		HWND_DESKTOP,                                                                                                           /* The window is a child-window to desktop */
		LoadMenu(hThisInstance, TEXT("MainMenu")),                                                                              /* No menu */
		hThisInstance,                                                                                                          /* Program Instance handler */
		NULL                                                                                                                    /* No Window Creation data */
		);

	/* Make the window visible on the screen */
	ShowWindow(ghwnd, nFunsterStil);
	set_display_switch_mode(SWITCH_BACKGROUND);
	set_display_switch_mode(SWITCH_BACKAMNESIA);
	win_set_window(ghwnd);

	return 0;
}

void initmenu()
{
	HMENU hmenu;

	hmenu = GetMenu(ghwnd);
	CheckMenuItem(hmenu, IDM_HARD_BBC, (RR_jumpers & RAMROM_FLAG_BBCMODE) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_RAMROM_ENABLE, ramrom_enable ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_RAMROM_DISKROM, (RR_jumpers & RAMROM_FLAG_DISKROM) ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hmenu, IDM_VID_SNOW, snow ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_VID_COLOUR, colourboard ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_VID_PALNOTNTSC, palnotntsc ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_TAPES_NORMAL, fasttape ? MF_UNCHECKED : MF_CHECKED);
	CheckMenuItem(hmenu, IDM_TAPES_FAST,  fasttape ? MF_CHECKED : MF_UNCHECKED);
	
	CheckMenuItem(hmenu, IDM_SOUND_ATOM, spon ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_SOUND_TAPE, tpon ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_SOUND_DDNOISE, sndddnoise ? MF_CHECKED : MF_UNCHECKED);

// SP3 JOYSTICK SUPPORT

	CheckMenuItem(hmenu, IDM_JOYSTICK, joyst ? MF_CHECKED : MF_UNCHECKED);	// Joystick support

// END SP3


// SP10 KEYBOARDJOYSTICK SUPPORT

	CheckMenuItem(hmenu, IDM_KEYJOYSTICK, keyjoyst ? MF_CHECKED : MF_UNCHECKED);	// Keyboard joystick support

// END SP10

	// AtomSID.
	CheckMenuItem(hmenu,IDM_SOUND_ATOMSID, sndatomsid ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(hmenu,IDM_SID_TYPE+cursid,MF_CHECKED);
    CheckMenuItem(hmenu,IDM_SID_INTERP+sidmethod,MF_CHECKED);
	
	// GDOS2015
	CheckMenuItem(hmenu,IDM_GDOS2015_ENABLE, fdc1770 ? MF_CHECKED : MF_UNCHECKED);	
	CheckMenuItem(hmenu,IDM_GDOS_BANK+GD_bank,MF_CHECKED);	
	set_dosrom_ptr();
	// End GDOS2015

	// RAM config
	CheckMenuItem(hmenu,IDM_MAIN_RAM+main_ramflag,MF_CHECKED);
	CheckMenuItem(hmenu,IDM_VIDEO_RAM+vid_ramflag,MF_CHECKED);
	// end RAM config
	
	CheckMenuItem(hmenu, IDM_DISC_WPROT_0, (writeprot[0]) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_DISC_WPROT_1, (writeprot[1]) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hmenu, IDM_DISC_WPROT_D, (defaultwriteprot) ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hmenu, IDM_DDT_525 + ddtype, MF_CHECKED);
	CheckMenuItem(hmenu, (IDM_DDV_33 + ddvol) - 1, MF_CHECKED);

	CheckMenuItem(hmenu, IDM_SPD_10 + 4, MF_CHECKED);
	
	CheckMenuItem(hmenu, IDM_MISC_DEBONBRK, debug_on_brk ? MF_CHECKED : MF_UNCHECKED);			
}

HINSTANCE hinstance;

void setejecttext(int drive, char *fn)
{
	MENUITEMINFO mi;
	HMENU hmenu;
	char s[128];

	if (fn[0])
		sprintf(s, "Eject drive :%i/%i - %s", drive, drive + 2, get_filename(fn));
	else
		sprintf(s, "Eject drive :%i/%i", drive, drive + 2);
	memset(&mi, 0, sizeof(MENUITEMINFO));
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_STRING;
	mi.fType = MFT_STRING;
	mi.dwTypeData = s;
	hmenu = GetMenu(ghwnd);
	SetMenuItemInfo(hmenu, IDM_DISC_EJECT_0 + drive, 0, &mi);
	CheckMenuItem(hmenu, IDM_DISC_WPROT_0 + drive, (writeprot[drive]) ? MF_CHECKED : MF_UNCHECKED);
}

int quited = 0;

void setquit()
{
	quited = 1;
}

CRITICAL_SECTION cs;

void startblit()
{
	EnterCriticalSection(&cs);
}

void endblit()
{
	LeaveCriticalSection(&cs);
}

char **argv;
int argc;
char *argbuf;

void processcommandline()
{
	char *cmdline;
	int argc_max;
	int i, q;

	/* can't use parameter because it doesn't include the executable name */
	cmdline = GetCommandLine();
	i = strlen(cmdline) + 1;
	argbuf = malloc(i);
	memcpy(argbuf, cmdline, i);

	argc = 0;
	argc_max = 64;
	argv = malloc(sizeof(char *) * argc_max);
	if (!argv)
	{
		free(argbuf);
		return;
	}

	i = 0;

	/* parse commandline into argc/argv format */
	while (argbuf[i])
	{
		while ((argbuf[i]) && (uisspace(argbuf[i])))
			i++;

		if (argbuf[i])
		{
			if ((argbuf[i] == '\'') || (argbuf[i] == '"'))
			{
				q = argbuf[i++];
				if (!argbuf[i])
					break;
			}
			else
				q = 0;

			argv[argc++] = &argbuf[i];

			if (argc >= argc_max)
			{
				argc_max += 64;
				argv = realloc(argv, sizeof(char *) * argc_max);
				if (!argv)
				{
					free(argbuf);
					return;
				}
			}

			while ((argbuf[i]) && ((q) ? (argbuf[i] != q) : (!uisspace(argbuf[i]))))
				i++;

			if (argbuf[i])
			{
				argbuf[i] = 0;
				i++;
			}
//	 rpclog("Arg %i - %s\n",argc-1,argv[argc-1]);
		}
	}

	argv[argc] = NULL;
//   free(argbuf);
}

HANDLE mainthread;

int atompause = 0;
void _mainthread(PVOID pvoid)
{
	atom_init(argc, argv);
	set_display_switch_mode(SWITCH_BACKGROUND);
	while (1)
	{
		if (atompause)
			Sleep(10);
		else
			atom_run();
	}
}

int WINAPI WinMain(HINSTANCE hThisInstance,
		   HINSTANCE hPrevInstance,
		   LPSTR lpszArgument,
		   int nFunsterStil)
{
	MSG messages;            /* Here messages to the application are saved */
	char *p;
	int oldf = 0;

	hinstance = hThisInstance;
//        cataloguetape("games1.tap");
	if (createwindow(hThisInstance, nFunsterStil))
		return -1;

	allegro_init();
	get_executable_name(exedir, 511);
	p = get_filename(exedir);
	p[0] = 0;

	loadconfig();

	processcommandline();

	if (defaultwriteprot)
		writeprot[0] = writeprot[1] = 1;

	initmenu();

	InitializeCriticalSection(&cs);
	mainthread = (HANDLE)_beginthread(_mainthread, 0, NULL);

	timeBeginPeriod(1);
	while (!quited) // && !key[KEY_F10])
	{
		if (PeekMessage(&messages, NULL, 0, 0, PM_REMOVE))
		{
			if (messages.message == WM_QUIT)
				quited = 1;
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&messages);
			/* Send message to WindowProcedure */
			DispatchMessage(&messages);
		}
		else
			Sleep(1);
//                #endif
		if (key[KEY_ALT] && key[KEY_ENTER] && fullscreen && !oldf)
		{
			EnterCriticalSection(&cs);
			fullscreen = 0;
			leavefullscreen();
			LeaveCriticalSection(&cs);
		}
		else if (key[KEY_ALT] && key[KEY_ENTER] && !fullscreen && !oldf)
		{
			EnterCriticalSection(&cs);
			fullscreen = 1;
			enterfullscreen();
			LeaveCriticalSection(&cs);
		}
		oldf = key[KEY_ALT] && key[KEY_ENTER];
	}

	startblit();
	TerminateThread(mainthread, 0);
	killdebug();

	atom_exit();
	timeEndPeriod(1);
	return 0;
}

//END_OF_MAIN();

char catnames[256][260];
int catnum = 0;

char tapefn[260];
char openfilestring[260];
int getfile(HWND hwnd, char *f, char *fn)
{
	OPENFILENAME ofn;       // common dialog box structure

	EnterCriticalSection(&cs);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = openfilestring;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	//
//        ofn.lpstrFile[0] = '\0';
	strcpy(ofn.lpstrFile, fn);
	ofn.nMaxFile = sizeof(openfilestring);
	ofn.lpstrFilter = f; //"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box.

	if (GetOpenFileName(&ofn))
	{
		LeaveCriticalSection(&cs);
		strcpy(fn, openfilestring);
		return 0;
	}
	LeaveCriticalSection(&cs);
	return 1;
}
int getsfile(HWND hwnd, char *f, char *fn, char *de)
{
	OPENFILENAME ofn;       // common dialog box structure

	EnterCriticalSection(&cs);

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = openfilestring;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	//
//        ofn.lpstrFile[0] = '\0';
	strcpy(ofn.lpstrFile, fn);
	ofn.nMaxFile = sizeof(openfilestring);
	ofn.lpstrFilter = f; //"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = de;

	// Display the Open dialog box.

	if (GetSaveFileName(&ofn))
	{
		LeaveCriticalSection(&cs);
		strcpy(fn, openfilestring);
		return 0;
	}
	LeaveCriticalSection(&cs);
	return 1;
}

HWND cath;
int catwindowopen = 0;

void clearcatwindow()
{
	int c;

	catnum = 0;
	if (!catwindowopen)
		return;
	for (c = 256; c >= 0; c--)
		SendMessage(cath, LB_DELETESTRING, c, (int)NULL);
}

void catupdatewindow()
{
	int c;

	if (!catwindowopen)
		return;
	for (c = 0; c < catnum; c++)
		SendMessage(cath, LB_ADDSTRING, (int)NULL, (int)catnames[c]);
}

void cataddname(char *s)
{
	if (catnum < 256)
	{
		strcpy(catnames[catnum], s);
		catnum++;
	}
//
//        if (!catwindowopen) return;
//        SendMessage(cath,LB_ADDSTRING,NULL,s);
}

BOOL CALLBACK catdlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		cath = GetDlgItem(hdlg, ListBox1);
		catwindowopen = 1;
		catupdatewindow();
//                if (cswena) findfilenamescsw();
//                else        findfilenamesuef();
//                SendMessage(h,LB_ADDSTRING,NULL,"test");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
		case IDOK: 
			return TRUE;
		}
		break;
	case WM_SYSCOMMAND:
		switch (LOWORD(wParam) & 0xFFF0)
		{
		case SC_CLOSE:
			DestroyWindow(hdlg);
			catwindowopen = 0;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int timerspeeds[] 	= { 5, 12, 25, 38, 50, 75, 85, 100, 150, 200, 250 };
int frameskips[] 	= { 0,  0,  0,  0,  0,  0,  0,   1,   2,   3,   4 };
int emuspeed = 4;
int fskipmax = 0;

extern unsigned char hw_to_mycode[256];
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int c;
	HMENU hmenu;

	switch (message)                  /* handle the messages */
	{
	case WM_CREATE:
		return 0;
	case WM_COMMAND:
		hmenu = GetMenu(hwnd);
		switch (LOWORD(wParam))
		{
		case IDM_FILE_RESET:
			atom_reset(0);
			return 0;

		case IDM_FILE_EXIT:
			PostQuitMessage(0);
			return 0;

		case IDM_TAPE_LOAD:
//                                rpclog("Tape load start\n");
			if (!getfile(hwnd, "Tape image (*.UEF;*.CSW)\0*.UEF;*.CSW\0UEF image (*.UEF)\0*.UEF;\0All files (*.*)\0*.*\0", tapefn))
			{
				startblit();
				Sleep(50);
//                                rpclog("CloseUEF\n");
				closeuef();
//                                rpclog("CloseCSW\n");
				closecsw();
//                                rpclog("clearcatwindow\n");
				clearcatwindow();
//                                rpclog("loadtape\n");
				loadtape(tapefn);
//                                rpclog("findfilenames\n");
//                                if (cswena) findfilenamescsw();
//                                else        findfilenamesuef();
//                                rpclog("catupdatewindow\n");
				catupdatewindow();
//                                rpclog("done\n");
				endblit();
			}
//                                rpclog("Tape load end\n");
			return 0;

		case IDM_TAPE_EJECT:
			startblit();
			Sleep(50);
			closeuef();
			closecsw();
			clearcatwindow();
			tapefn[0] = 0;
			endblit();
			return 0;

		case IDM_TAPE_REW:
			if (tapefn[0]) {
				startblit();
				Sleep(50);
				closeuef();
				closecsw();
				clearcatwindow();
				loadtape(tapefn);
				catupdatewindow();
				endblit();
			}
			return 0;

		case IDM_TAPE_CAT:
			if (!IsWindow(hwndCat))
			{
				hwndCat = CreateDialog(hinstance,
						       TEXT("Catalogue"),
						       hwnd,
						       (DLGPROC)catdlgproc);
				ShowWindow(hwndCat, SW_SHOW);
			}
			return 0;

		case IDM_TAPES_NORMAL:
		case IDM_TAPES_FAST:
			fasttape = LOWORD(wParam) - IDM_TAPES_NORMAL;
			CheckMenuItem(hmenu, IDM_TAPES_NORMAL, fasttape ? MF_UNCHECKED : MF_CHECKED);
			CheckMenuItem(hmenu, IDM_TAPES_FAST,  fasttape ? MF_CHECKED : MF_UNCHECKED);
			return 0;

		case IDM_DISC_LOAD_0:
			if (!getfile(hwnd, "Disc image (*.DSK;*.SSD;*.DSD;*.FDI)\0*.DSK;*.SSD;*.DSD;*.FDI\0All files (*.*)\0*.*\0", discfns[0]))
			{
				closedisc(0);
				loaddisc(0, discfns[0]);
				if (defaultwriteprot)
					writeprot[0] = 1;
				CheckMenuItem(hmenu, IDM_DISC_WPROT_0, (writeprot[0]) ? MF_CHECKED : MF_UNCHECKED);
			}
			break;

		case IDM_DISC_LOAD_1:
			if (!getfile(hwnd, "Disc image (*.DSK;*.SSD;*.DSD;*.FDI)\0*.DSK;*.SSD;*.DSD;*.FDI\0All files (*.*)\0*.*\0", discfns[1]))
			{
				closedisc(1);
				loaddisc(1, discfns[1]);
				if (defaultwriteprot)
					writeprot[1] = 1;
				CheckMenuItem(hmenu, IDM_DISC_WPROT_1, (writeprot[1]) ? MF_CHECKED : MF_UNCHECKED);
			}
			break;

		case IDM_DISC_EJECT_0:
			closedisc(0);
			discfns[0][0] = 0;
			setejecttext(0, "");
			break;

		case IDM_DISC_EJECT_1:
			closedisc(1);
			discfns[1][0] = 0;
			setejecttext(1, "");
			break;

		case IDM_DISC_NEW_0:
			if (!getsfile(hwnd, "Disc image (*.DSK;*.SSD;*.DSD)\0*.DSK;*.SSD;*.DSD\0All files (*.*)\0*.*\0", discfns[0], "DSK"))
			{
				closedisc(0);
				newdisc(0, discfns[0]);
				if (defaultwriteprot)
					writeprot[0] = 1;
				CheckMenuItem(hmenu, IDM_DISC_WPROT_0, (writeprot[0]) ? MF_CHECKED : MF_UNCHECKED);
			}
			break;

		case IDM_DISC_NEW_1:
			if (!getsfile(hwnd, "Disc image (*.DSK;*.SSD;*.DSD)\0*.DSK;*.SSD;*.DSD\0All files (*.*)\0*.*\0", discfns[1], "DSK"))
			{
				closedisc(1);
				newdisc(1, discfns[1]);
				if (defaultwriteprot)
					writeprot[1] = 1;
				CheckMenuItem(hmenu, IDM_DISC_WPROT_1, (writeprot[1]) ? MF_CHECKED : MF_UNCHECKED);
			}
			break;

		case IDM_DISC_WPROT_0:
			writeprot[0] = !writeprot[0];
			if (fwriteprot[0])
				writeprot[0] = 1;
			CheckMenuItem(hmenu, IDM_DISC_WPROT_0, (writeprot[0]) ? MF_CHECKED : MF_UNCHECKED);
			break;

		case IDM_DISC_WPROT_1:
			writeprot[1] = !writeprot[1];
			if (fwriteprot[1])
				writeprot[1] = 1;
			CheckMenuItem(hmenu, IDM_DISC_WPROT_1, (writeprot[1]) ? MF_CHECKED : MF_UNCHECKED);
			break;

		case IDM_DISC_WPROT_D:
			defaultwriteprot = !defaultwriteprot;
			CheckMenuItem(hmenu, IDM_DISC_WPROT_D, (defaultwriteprot) ? MF_CHECKED : MF_UNCHECKED);
			break;

		case IDM_HARD_BBC:
			atompause=1;
			RR_jumpers ^= RAMROM_FLAG_BBCMODE;
			CheckMenuItem(hmenu, LOWORD(wParam), (RR_jumpers & RAMROM_FLAG_BBCMODE) ? MF_CHECKED : MF_UNCHECKED);
			atom_reset(1);
			atompause=0;
			return 0;

		case IDM_RAMROM_ENABLE:
			ramrom_enable = !ramrom_enable;
			CheckMenuItem(hmenu, LOWORD(wParam), ramrom_enable ? MF_CHECKED : MF_UNCHECKED);
			atom_reset(1);
			return 0;

		case IDM_RAMROM_DISKROM:
			RR_jumpers ^= RAMROM_FLAG_DISKROM;
			CheckMenuItem(hmenu, IDM_RAMROM_DISKROM, (RR_jumpers & RAMROM_FLAG_DISKROM) ? MF_CHECKED : MF_UNCHECKED);
			atom_reset(1);
			return 0;

		case IDM_MISC_DEBUG:
			startblit();
			Sleep(200);
			if (!debugon)
			{
				debug = debugon = 1;
				startdebug();
//                                EnableMenuItem(hmenu,IDM_BREAK,MF_ENABLED);
			}
			else
			{
				debug ^= 1;
				enddebug();
//                                EnableMenuItem(hmenu,IDM_BREAK,MF_GRAYED);
			}
			endblit();
			return 0;

		case IDM_MISC_DEBONBRK :
			debug_on_brk = !debug_on_brk;
			CheckMenuItem(hmenu, IDM_MISC_DEBONBRK, debug_on_brk ? MF_CHECKED : MF_UNCHECKED);
			break;
			
		case IDM_MISC_BREAK:
			debug = 1;
			break;

		case IDM_VID_SNOW:
			snow = !snow;
			CheckMenuItem(hmenu, LOWORD(wParam), snow ? MF_CHECKED : MF_UNCHECKED);
			return 0;

		case IDM_VID_COLOUR:
			colourboard = !colourboard;
			CheckMenuItem(hmenu, LOWORD(wParam), colourboard ? MF_CHECKED : MF_UNCHECKED);
			updatepal();
			return 0;

		case IDM_VID_PALNOTNTSC:
			palnotntsc = !palnotntsc;
			CheckMenuItem(hmenu, LOWORD(wParam), palnotntsc ? MF_CHECKED : MF_UNCHECKED);
			return 0;

		case IDM_SOUND_ATOM:
			spon = !spon;
			CheckMenuItem(hmenu, LOWORD(wParam), spon ? MF_CHECKED : MF_UNCHECKED);
			return 0;

		case IDM_SOUND_TAPE:
			tpon = !tpon;
			CheckMenuItem(hmenu, LOWORD(wParam), tpon ? MF_CHECKED : MF_UNCHECKED);
			return 0;

		case IDM_SOUND_DDNOISE:
			sndddnoise = !sndddnoise;
			CheckMenuItem(hmenu, LOWORD(wParam), sndddnoise ? MF_CHECKED : MF_UNCHECKED);
			return 0;
			
		case IDM_SOUND_ATOMSID:
			sndatomsid=!sndatomsid;
            CheckMenuItem(hmenu,IDM_SOUND_ATOMSID,sndatomsid ? MF_CHECKED : MF_UNCHECKED);
            break;
                        
		case IDM_SID_INTERP: 
		case IDM_SID_RESAMP:
			CheckMenuItem(hmenu,IDM_SID_INTERP,MF_UNCHECKED);
            CheckMenuItem(hmenu,IDM_SID_RESAMP,MF_UNCHECKED);
            CheckMenuItem(hmenu,LOWORD(wParam),MF_CHECKED);
            sidmethod=LOWORD(wParam)-IDM_SID_INTERP;
            sid_settype(sidmethod, cursid);
            break;
        
		case IDM_DDV_33:
		case IDM_DDV_66:
		case IDM_DDV_100:
			CheckMenuItem(hmenu, (IDM_DDV_33 + ddvol) - 1, MF_UNCHECKED);
			ddvol = (LOWORD(wParam) - IDM_DDV_33) + 1;
			CheckMenuItem(hmenu, (IDM_DDV_33 + ddvol) - 1, MF_CHECKED);
			break;

		case IDM_DDT_525:
		case IDM_DDT_35:
			CheckMenuItem(hmenu, IDM_DDT_525 + ddtype, MF_UNCHECKED);
			ddtype = LOWORD(wParam) - IDM_DDT_525;
			CheckMenuItem(hmenu, IDM_DDT_525 + ddtype, MF_CHECKED);
			closeddnoise();
			loaddiscsamps();
			break;

// SP3 JOYSTICK SUPPORT

		case IDM_JOYSTICK:
			joyst = !joyst;
			CheckMenuItem(hmenu, LOWORD(wParam), joyst ? MF_CHECKED : MF_UNCHECKED);
			return 0;

// END SP3

// SP10 KEYBOARDJOYSTICK SUPPORT

		case IDM_KEYJOYSTICK:
			keyjoyst = !keyjoyst;
			CheckMenuItem(hmenu, LOWORD(wParam), keyjoyst ? MF_CHECKED : MF_UNCHECKED);
			return 0;

// END SP10

// GDOS2015
		case IDM_GDOS2015_ENABLE:
			fdc1770 = !fdc1770;
			CheckMenuItem(hmenu,IDM_GDOS2015_ENABLE, fdc1770 ? MF_CHECKED : MF_UNCHECKED);	
			break;
// end GDOS2015
			
		case IDM_VID_FULLSCREEN:
			fullscreen = 1;
			EnterCriticalSection(&cs);
			enterfullscreen();
			LeaveCriticalSection(&cs);
			break;

		case IDM_KEY_REDEFINE:
			redefinekeys();
			break;
		case IDM_KEY_DEFAULT:
			for (c = 0; c < 128; c++)
				keylookup[c] = c;
			break;

		case IDM_MISC_SCRSHOT:
			if (!getsfile(hwnd, "Bitmap file (*.BMP)\0*.BMP\0All files (*.*)\0*.*\0", scrshotname, "BMP"))
			{
				savescrshot = 1;
			}
			break;
		case IDM_MISC_START_MOVIE:
			if (!getsfile(hwnd, "Video capture file (*.VID)\0*.VID\0All files (*.*)\0*.*\0", moviename, "VID"))
			{
				startmovie();
			}
			break;
		case IDM_MISC_STOP_MOVIE:
			stopmovie();
			break;
		case IDM_SPD_10:
		case IDM_SPD_25:
		case IDM_SPD_50:
		case IDM_SPD_75:
		case IDM_SPD_100:
		case IDM_SPD_150:
		case IDM_SPD_170:
		case IDM_SPD_200:
		case IDM_SPD_300:
		case IDM_SPD_400:
		case IDM_SPD_500:
			CheckMenuItem(hmenu, IDM_SPD_10 + emuspeed, MF_UNCHECKED);
			emuspeed = LOWORD(wParam) - IDM_SPD_10;
			changetimerspeed(timerspeeds[emuspeed]);
			fskipmax = frameskips[emuspeed];
			CheckMenuItem(hmenu, IDM_SPD_10 + emuspeed, MF_CHECKED);
			
/*SP8 CHANGE CORRECTED SPEEDFLAG

			if(emuspeed >= RAMROM_EMU_FAST)
				RR_jumpers |= RAMROM_FLAG_FAST;
			else
				RR_jumpers &= ~RAMROM_FLAG_FAST;
*SP8 END */				
			break;
		}
        if (LOWORD(wParam)>=IDM_SID_TYPE && LOWORD(wParam)<(IDM_SID_TYPE+100))
        {
			CheckMenuItem(hmenu,IDM_SID_TYPE+cursid,MF_UNCHECKED);
            cursid=LOWORD(wParam)-IDM_SID_TYPE;
            CheckMenuItem(hmenu,IDM_SID_TYPE+cursid,MF_CHECKED);
            sid_settype(sidmethod, cursid);
        }
// GDOS2015
        if (LOWORD(wParam)>=IDM_GDOS_BANK && LOWORD(wParam)<(IDM_GDOS_BANK+16))
        {
			CheckMenuItem(hmenu,IDM_GDOS_BANK + GD_bank,MF_UNCHECKED);
            GD_bank=LOWORD(wParam)-IDM_GDOS_BANK;
            CheckMenuItem(hmenu,IDM_GDOS_BANK + GD_bank,MF_CHECKED);
            set_dosrom_ptr();
		}
// end GDOS2015

// RAM config
        if (LOWORD(wParam)>=IDM_MAIN_RAM && LOWORD(wParam)<(IDM_MAIN_RAM+6))
        {
			CheckMenuItem(hmenu,IDM_MAIN_RAM + main_ramflag,MF_UNCHECKED);
            main_ramflag=LOWORD(wParam)-IDM_MAIN_RAM;
            CheckMenuItem(hmenu,IDM_MAIN_RAM + main_ramflag,MF_CHECKED);
		}

        if (LOWORD(wParam)>=IDM_VIDEO_RAM && LOWORD(wParam)<(IDM_VIDEO_RAM+8))
        {
			CheckMenuItem(hmenu,IDM_VIDEO_RAM + vid_ramflag,MF_UNCHECKED);
            vid_ramflag=LOWORD(wParam)-IDM_VIDEO_RAM;
            CheckMenuItem(hmenu,IDM_VIDEO_RAM + vid_ramflag,MF_CHECKED);
			SET_VID_TOP();
		}
		
// end RAM config
        return 0;

	case WM_SIZE:
		winsizex = lParam & 0xFFFF;
		winsizey = lParam >> 16;
		break;

	case WM_ENTERMENULOOP:
//		rpclog("EnterMenuLoop\n");
		atompause = 1;
		//EnterCriticalSection(&cs);
		break;
	case WM_EXITMENULOOP:
//		rpclog("ExitMenuLoop\n");
		atompause = 0;
//                clearkeys();
		for (c = 0; c < 128; c++)
			key[c] = 0;
		//LeaveCriticalSection(&cs);
		break;

	case WM_SETFOCUS:
//		rpclog("SetFocus\n");
//                clearkeys();
		for (c = 0; c < 128; c++)
			key[c] = 0;
		atompause = 0;
		break;


	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (LOWORD(wParam) != 255)
		{
//                        rpclog("Key %04X %04X\n",LOWORD(wParam),VK_LEFT);
			c = MapVirtualKey(LOWORD(wParam), 0);
			c = hw_to_mycode[c];
//                        rpclog("MVK %i %i %i\n",c,hw_to_mycode[c],KEY_PGUP);
			if (LOWORD(wParam) == VK_LEFT)
				c = KEY_LEFT;
			if (LOWORD(wParam) == VK_RIGHT)
				c = KEY_RIGHT;
			if (LOWORD(wParam) == VK_UP)
				c = KEY_UP;
			if (LOWORD(wParam) == VK_DOWN)
				c = KEY_DOWN;
			if (LOWORD(wParam) == VK_HOME)
				c = KEY_HOME;
			if (LOWORD(wParam) == VK_END)
				c = KEY_END;
			if (LOWORD(wParam) == VK_INSERT)
				c = KEY_INSERT;
			if (LOWORD(wParam) == VK_DELETE)
				c = KEY_DEL;
			if (LOWORD(wParam) == VK_PRIOR)
				c = KEY_PGUP;
			if (LOWORD(wParam) == VK_NEXT)
				c = KEY_PGDN;
//                        rpclog("MVK2 %i %i %i\n",c,hw_to_mycode[c],KEY_PGUP);
			key[c] = 1;
		}
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		if (LOWORD(wParam) != 255)
		{
//                        rpclog("Key %04X %04X\n",LOWORD(wParam),VK_LEFT);
			c = MapVirtualKey(LOWORD(wParam), 0);
			c = hw_to_mycode[c];
			if (LOWORD(wParam) == VK_LEFT)
				c = KEY_LEFT;
			if (LOWORD(wParam) == VK_RIGHT)
				c = KEY_RIGHT;
			if (LOWORD(wParam) == VK_UP)
				c = KEY_UP;
			if (LOWORD(wParam) == VK_DOWN)
				c = KEY_DOWN;
			if (LOWORD(wParam) == VK_HOME)
				c = KEY_HOME;
			if (LOWORD(wParam) == VK_END)
				c = KEY_END;
			if (LOWORD(wParam) == VK_INSERT)
				c = KEY_INSERT;
			if (LOWORD(wParam) == VK_DELETE)
				c = KEY_DEL;
			if (LOWORD(wParam) == VK_PRIOR)
				c = KEY_PGUP;
			if (LOWORD(wParam) == VK_NEXT)
				c = KEY_PGDN;
//                        rpclog("MVK %i\n",c);
			key[c] = 0;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);             /* send a WM_QUIT to the message queue */
		break;
	default:                                /* for messages that we don't deal with */
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

#endif
