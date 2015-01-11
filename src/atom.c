/*Atomulator v1.0 by Tom Walker
  Main loop*/
  
#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include "atom.h"

char exedir[512];

int colourboard=1;
int sndddnoise=1;

int bbcmode=0;
int fasttape=0;
char discfn[260];

FILE *rlog;
void rpclog(char *format, ...)
{
   char buf[256];
   return;
   if (!rlog) rlog=fopen("rlog.txt","wt");
//turn;
   va_list ap;
   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);
   fputs(buf,rlog);
   fflush(rlog);
}

int tapeon;
uint8_t *ram;
char filelist[256][17];
int filelistsize;
int filepos[256];

int drawscr;
int ddframes=0;

void scrupdate()
{
        ddframes++;
        drawscr++;
}

void atom_reset()
{
        memset(ram,0,0x10000);
        ram[8]=rand();
        ram[9]=rand();
        ram[10]=rand();
        ram[11]=rand();
        reset6502();
        resetvia();
}

void atom_init(int argc, char **argv)
{
        int c;
        int tapenext=0,discnext=0;
        for (c=1;c<argc;c++)
        {
                if (!strcasecmp(argv[c],"--help"))
                {
                        printf("Atomulator v1.0 command line options :\n\n");
                        printf("-disc disc.ssd  - load disc.ssd into drives :0/:2\n");
                        printf("-disc1 disc.ssd - load disc.ssd into drives :1/:3\n");
                        printf("-tape tape.uef  - load tape.uef\n");
                        printf("-fasttape       - set tape speed to fast\n");
                        printf("-debug          - start debugger\n");
                        exit(-1);
                }
                else
                if (!strcasecmp(argv[c],"-tape"))
                {
                        tapenext=2;
                }
                else if (!strcasecmp(argv[c],"-disc") || !strcasecmp(argv[c],"-disk"))
                {
                        discnext=1;
                }
                else if (!strcasecmp(argv[c],"-disc1"))
                {
                        discnext=2;
                }
                else if (!strcasecmp(argv[c],"-fasttape"))
                {
                        fasttape=1;
                }
                else if (!strcasecmp(argv[c],"-debug"))
                {
                        debug=debugon=1;
                }
                else if (tapenext)
                   strcpy(tapefn,argv[c]);
                else if (discnext)
                {
                        strcpy(discfns[discnext-1],argv[c]);
                        discnext=0;
                }
                else
                {
                        strcpy(discfns[0],argv[c]);
                        discnext=0;
                }
                if (tapenext) tapenext--;
        }
        initalmain(0,NULL);
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
        install_int_ex(scrupdate,BPS_TO_TIMER(300));
        inital();
        loaddiscsamps();
        loaddisc(0,discfns[0]);
        loaddisc(1,discfns[1]);
}

void changetimerspeed(int i)
{
        remove_int(scrupdate);
        install_int_ex(scrupdate,BPS_TO_TIMER(i*6));
}

int oldf12=0;
void atom_run()
{
        if ((drawscr>0) || (tapeon && fasttape))
        {
                if (colourboard) exec6502(312,64);
                else             exec6502(262,64);
                poll_keyboard();
                if (tapeon && fasttape) drawscr=0;
                else                    drawscr-=(colourboard)?6:5;
                if (drawscr>25) drawscr=0;
                if (ddframes>=25)
                {
                        ddframes-=25;
                        mixddnoise();
                }
                if (key[KEY_F12] && !oldf12)
                {
                        reset6502();
                        resetvia();
                }
                oldf12=key[KEY_F12];
        }
        else
                      rest(1);
}

void atom_exit()
{
        saveconfig();
        closeddnoise();
//        dumpregs();
//        dumpram();
}

