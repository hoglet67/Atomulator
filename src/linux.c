/*Atomulator v1.0 by Tom Walker
   Linux main*/

#ifndef WIN32

#include <stdio.h>
#include <allegro.h>
#include "atom.h"

int quited = 0;
extern int oldf11;

void startblit()
{
}
void endblit()
{
}
void updatewindowsize(int x, int y)
{
}

int keylookup[128];

void cataddname(char *s)
{
}

int main(int argc, char **argv)
{
	char *p;

	allegro_init();
	get_executable_name(exedir, 511);
	p = get_filename(exedir);
	p[0] = 0;

	loadconfig();

	if (defaultwriteprot)
		writeprot[0] = writeprot[1] = 1;

	atom_init(argc, argv);
	install_keyboard();
	while (!quited)
	{
		atom_run();
		if (key[KEY_F11] && !oldf11)
		{
			oldf11 = 1;
			entergui();
		}
		oldf11 = key[KEY_F11];
	}
	atom_exit();
	return 0;
}

END_OF_MAIN();

#endif
