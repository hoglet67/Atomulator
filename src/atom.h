#include <stdint.h>

#define ATOMULATOR_VERSION	"Atomulator 1.26"

#define MAXPATH	512

void rpclog(char *format, ...);

/*SP7 CHANGES*/

void prtbuf(char *format, ...);

/*END SP7*/

extern char exedir[MAXPATH+1];

void startblit();
void endblit();

int vbl;
int gfxmode;
int css;
int speaker;
uint8_t lastdat;
int cswena;
int cswpoint;

//#define printf rpclog

int colourboard;
int palnotntsc;

// SP3 FOR JOYSTICK SUPPORT

int joyst;

// END SP3

// SP10 FOR KEYBOARDJOYSTICK SUPPORT

int keyjoyst;

// END SP10

int fasttape;
int ramrom_enable;
int RR_jumpers;
int RR_enables;

void set_dosrom_ptr();

int interrupt;

typedef struct VIA
{
	uint8_t ora, orb, ira, irb;
	uint8_t ddra, ddrb;
	uint32_t t1l, t2l;
	int t1c, t2c;
	uint8_t acr, pcr, ifr, ier;
	int t1hit, t2hit;
	uint8_t porta, portb;
} VIA;

VIA via;

int fetchc[65536], readc[65536], writec[65536];
uint16_t pc;
uint8_t a, x, y, s;
struct
{
	int c, z, i, d, v, n;
} p;
extern int nmi;
int debug, debugon;

extern uint8_t opcode;

int spon, tpon;

/* For 1770 based GDOS */
//#define WD1770 1
extern int fdc1770;
extern int GD_bank;
/* end */

// RAM Config
extern int main_ramflag;
extern int vid_ramflag;
extern int vid_top;
#define SET_VID_TOP()	{vid_top=((vid_ramflag+1)*0x0400)+0x8000;}	// Last video RAM address.
// end RAM Config

void (*fdccallback)();
void (*fdcdata)(uint8_t dat);
void (*fdcspindown)();
void (*fdcfinishread)();
void (*fdcnotfound)();
void (*fdcdatacrcerror)();
void (*fdcheadercrcerror)();
void (*fdcwriteprotect)();
int (*fdcgetdata)(int last);

extern int writeprot[2], fwriteprot[2];

void ssd_reset();
void ssd_load(int drive, char *fn);
void ssd_close(int drive);
void dsd_load(int drive, char *fn);
void ssd_seek(int drive, int track);
void ssd_readsector(int drive, int sector, int track, int side, int density);
void ssd_writesector(int drive, int sector, int track, int side, int density);
void ssd_readaddress(int drive, int sector, int side, int density);
void ssd_format(int drive, int sector, int side, int density);
void ssd_poll();

void fdi_reset();
void fdi_load(int drive, char *fn);
void fdi_close(int drive);
void fdi_seek(int drive, int track);
void fdi_readsector(int drive, int sector, int track, int side, int density);
void fdi_writesector(int drive, int sector, int track, int side, int density);
void fdi_readaddress(int drive, int sector, int side, int density);
void fdi_format(int drive, int sector, int side, int density);
void fdi_poll();

void loaddisc(int drive, char *fn);
void newdisc(int drive, char *fn);
void closedisc(int drive);
void disc_reset();
void disc_poll();
void disc_seek(int drive, int track);
void disc_readsector(int drive, int sector, int track, int side, int density);
void disc_writesector(int drive, int sector, int track, int side, int density);
void disc_readaddress(int drive, int track, int side, int density);
void disc_format(int drive, int track, int side, int density);
extern int defaultwriteprot;
extern char discfns[2][260];


void setejecttext(int drive, char *fn);

void loaddiscsamps();
void mixddnoise();
extern int ddvol, ddtype;

extern int motorspin;
extern int fdctime;
extern int motoron;
extern int disctime;

struct
{
	void (*seek)(int drive, int track);
	void (*readsector)(int drive, int sector, int track, int side, int density);
	void (*writesector)(int drive, int sector, int track, int side, int density);
	void (*readaddress)(int drive, int track, int side, int density);
	void (*format)(int drive, int track, int side, int density);
	void (*poll)();
} drives[2];

extern int curdrive;

extern int sndddnoise;
extern int sndatomsid;
extern int cursid;
extern int sidmethod;

void opencsw(char *fn);
void closecsw();
int getcsw();
void findfilenamescsw();

void inituef();
void polluef();
void openuef(char *fn);
void closeuef();
void rewindit();
int getftell();
void findfilenamesuef();

void initalmain(int argc, char *argv[]);
void inital();
void givealbuffer(int16_t *buf);
void givealbufferdd(int16_t *buf);

void reset6502();
void exec6502();
void dumpregs();

void initmem();
void loadroms();
void dumpram();

void initvideo();
void drawline(int l);
void updatepal();

void reset8271();
uint8_t read8271(uint16_t addr);
void write8271(uint16_t addr, uint8_t val);

void loaddiscsamps();
void closeddnoise();
void ddnoise_seek(int len);

void pollsound();
void polltape();

void init8255();
void write8255(uint16_t addr, uint8_t val);
uint8_t read8255(uint16_t addr);
void receive(uint8_t dat);
void dcd();
void dcdlow();

void writevia(uint16_t addr, uint8_t val);
uint8_t readvia(uint16_t addr);
void resetvia();
void updatetimers();

void startdebug();
void enddebug();
void killdebug();
void debugread(uint16_t addr);
void debugwrite(uint16_t addr, uint8_t val);
void dodebugger();
void debuglog(char *format, ...);

void cataddname(char *s);

void atom_init(int argc, char **argv);
void atom_run();
void atom_exit();
void atom_reset(int power_on);

void setquit();

uint8_t readmeml(uint16_t addr);
void writememl(uint16_t addr, uint8_t val);

extern int winsizex, winsizey;


void redefinekeys();
extern int keylookup[128];

void loadconfig();
void saveconfig();

extern int snow;
extern int fullscreen;
void enterfullscreen();
void leavefullscreen();

void updatewindowsize(int x, int y);
void loadtape(char *fn);

void clearscreen();

#ifndef WIN32
void entergui();
#endif

extern char tapefn[260];
extern int emuspeed, fskipmax;
extern char scrshotname[260];
extern int savescrshot;

void changetimerspeed(int i);
