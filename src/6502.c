/*Atomulator v1.0 by Tom Walker
   6502 emulation*/
#include <allegro.h>
#include <string.h>
#include <stdio.h>
#include "atom.h"
#include "roms.h"
#include "8271.h"
#include "atommc.h"

int tapeon;
int totcyc = 0;
int tapecyc = 635;
void dumpregs();

int skipint, nmi, nmilock, interrupt = 0, timetolive, oldnmi;
int skipint2, oldnmi2;

uint8_t *ram, *rom;

// Pointers to memory blocks
uint8_t *utility_ptr;
uint8_t *abasic_ptr;
uint8_t *afloat_ptr;
uint8_t *dosrom_ptr;
uint8_t *akernel_ptr;

/*6502 registers*/
uint8_t a, x, y, s;
uint16_t pc;
/*struct
   {
        int c,z,i,d,v,n;
   } p;*/

/*Memory structures*/
uint8_t *mem[0x100];
int memstat[0x100];

int cycles = 0;
int output = 0;
int ins = 0;

/* RAMROM */
int RR_bankreg	= 0;
int RR_enables	= 0;
int RR_jumpers	= 0;

void dumpram()
{
	FILE *f = fopen("ram.dmp", "wb");

	fwrite(ram, 0x10000, 1, f);
	fclose(f);
}

void initmem()
{
	int c;

	ram = (uint8_t*)malloc(0x10000);
	rom = (uint8_t*)malloc(ROM_MEM_SIZE + RAM_ROM_SIZE);
	
	memset(ram, 0, 0x10000);
	
	for (c = 0; c < 0x100; c++)
		memstat[c] = 2;
	
	ram[8] = rand() & 255;
	ram[9] = rand() & 255;
	ram[10] = rand() & 255;
	ram[11] = rand() & 255;
}

void load_rom(char	*Name,
			  int 	Size,
	          int 	Offset)
{
	FILE    *RomFile;
	int 	bytes;

	rpclog("loading rom %s at %05X, size=%04X ", Name, Offset, Size);

	RomFile = fopen(Name, "rb");
	if(RomFile!=NULL)
	{
		bytes = fread(&rom[Offset], 1, Size, RomFile);
		fclose(RomFile);

		rpclog("bytes read %X\n", bytes);
	}
	else
		rpclog("Loading %s failed !\n",Name);
}

void loadroms()
{
	load_rom("roms/akernel.rom",			ROM_SIZE_ATOM,          ROM_OFS_AKERNEL);
	load_rom("roms/dosrom.rom",             ROM_SIZE_ATOM,          ROM_OFS_DOSROM);
	load_rom("roms/afloat.rom",             ROM_SIZE_ATOM,          ROM_OFS_AFLOAT);
	load_rom("roms/abasic.rom",             ROM_SIZE_ATOM,          ROM_OFS_ABASIC);
	load_rom("roms/axr1.rom",               ROM_SIZE_ATOM,          ROM_OFS_UTILITY);
	load_rom("roms/atom_bbc_basic_os.rom",  ROM_SIZE_ATOM,          ROM_OFS_BBC_OS);
	load_rom("roms/basic1.rom",             ROM_SIZE_BBC_BASIC, 	ROM_OFS_BBC_BASIC);
	load_rom("roms/ramrom.rom",             RAM_ROM_SIZE,           ROM_OFS_RAMROM);
}

void set_rr_ptrs()
{
	if (ramrom_enable)
	{
		// Select what is mapped into $A000 block, this allows the 
		// mapping of rom or ram into the utility block so that a 
		// rom may be loaded from disk and then mapped in.
		if ((RR_enables & RAMROM_FLAG_EXTRAM) && (RR_bankreg==0))
			utility_ptr = &ram[0x7000];
		else
			utility_ptr = &rom[ROM_OFS_RAMROM + (RR_bankreg * ROM_SIZE_ATOM)];
		
		rpclog("RR_enables=%2X, RR_bankreg=%2X\n",RR_enables, RR_bankreg);
		rpclog("utility_ptr set to %X\n\n",utility_ptr);


		// select Kernel and dosrom based on jumper setting.
		// The kernel needs to be changed as well as the dosrom, as the ramrom 
		// has a modified kernel to allow the starting of the AtoMMC firmware
		// when it resides at $E000.
		// Note as of 2012-06-28 the real ram-rom does not do this (yet).
		if (ramrom_enable && RR_bit_set(RAMROM_FLAG_DISKROM))
		{
			dosrom_ptr      = &rom[ROM_OFS_RR_DOSROM];
			akernel_ptr     = &rom[ROM_OFS_RR_AKERNEL];
		}
		else
		{
			dosrom_ptr      = &rom[ROM_OFS_DOSROM];
			akernel_ptr     = &rom[ROM_OFS_AKERNEL];
		}
	}
}

void reset_rom()
{
	debuglog("reset_rom(), ramrom=%d, bbcmode=%d\n",ramrom_enable,bbcmode);
	
	if (!ramrom_enable)
	{
		utility_ptr     = &rom[ROM_OFS_UTILITY];
		abasic_ptr      = &rom[ROM_OFS_ABASIC];
		afloat_ptr      = &rom[ROM_OFS_AFLOAT];
		dosrom_ptr      = &rom[ROM_OFS_DOSROM];
		akernel_ptr     = &rom[ROM_OFS_AKERNEL];
		rpclog("Running with standard roms\n");
	}
	else
	{
		abasic_ptr      = &rom[ROM_OFS_RR_ABASIC];
		afloat_ptr      = &rom[ROM_OFS_RR_AFLOAT];
		set_rr_ptrs();
		rpclog("Running with Ramoth RAMROM roms\n");
	}
	rpclog("rom=%X\nkernel=%X\nbasic=%X\n",rom,akernel_ptr,abasic_ptr);
	rpclog("ROM_OFS_RR_AKERNEL=%5X\n", ROM_OFS_RR_AKERNEL);
	
	debuglog("reset_rom():done\n");
}

uint8_t fetcheddat[32];

uint8_t readmeml(uint16_t addr)
{
	if (debugon)
		debugread(addr);

	if (pc == addr)
		fetchc[addr] = 31;
	else
		readc[addr] = 31;

	if (!bbcmode)
	{
		switch (addr & 0xFC00)
		{
		case 0x0000:         /*Block zero RAM*/
			return ram[addr];

		case 0x0400:						   case 0x0C00:
		case 0x1000: case 0x1400: case 0x1800: case 0x1C00:
		case 0x2000: case 0x2400:         /*DOS RAM*/
		case 0x2800: case 0x2C00:
		case 0x3000: case 0x3400: case 0x3800: case 0x3C00:
		case 0x4000: case 0x4400: case 0x4800: case 0x4C00:
		case 0x5000: case 0x5400: case 0x5800: case 0x5C00:
		case 0x6000: case 0x6400: case 0x6800: case 0x6C00:
			return ram[addr];
		
		case 0x7000: case 0x7400: case 0x7800: case 0x7C00:
			if(!ramrom_enable || (RR_enables & RAMROM_FLAG_EXTRAM)==0)
				return ram[addr];

		case 0x8000: case 0x8400: case 0x8800: case 0x8C00:         /*Video RAM*/
		case 0x9000: case 0x9400: case 0x9800: case 0x9C00:
			if (snow && cycles >= 0 && cycles < 32)
				fetcheddat[31 - cycles] = ram[addr];
			return ram[addr];

		case 0x0800:
			if ((addr & 0x0F00) == 0x0A00)	/*FDC*/
			{
				if(ramrom_enable && RR_BLKA_enabled())
					return ram[addr];
				else
					return read8271(addr);                  /*FDC*/
			}
			else
				return ram[addr];

		case 0xB000:         /*8255 PIA*/
			return read8255(addr);

		case 0xB400:
			return ReadMMC(addr);

		case 0xB800:         /*6522 VIA*/
			return readvia(addr);
			
		case 0xBC00:
			if((sndatomsid) && (addr>=0xBDC0) && (addr<=0xBDDF))
				return sid_read(addr & 0x1F);
				
			if(ramrom_enable)
			{
				switch(addr)
				{
					case 0xBFFF :
						return (0xB0 | (RR_bankreg & 0x0F));
					
					case 0xBFFE :
						return (0xB0 | (RR_enables & 0x0F));
						
					case 0xBFFD :
						return (0xB0 | (RR_jumpers & 0x0F));
						
					default:
						return 0xBF;
				}
			}

		case 0xA000: case 0xA400: case 0xA800: case 0xAC00:         /*Utility ROM*/
			return utility_ptr[addr & 0x0FFF];

		case 0xC000: case 0xC400: case 0xC800: case 0xCC00:         /*BASIC*/
			return abasic_ptr[addr & 0x0FFF];

		case 0xD000: case 0xD400: case 0xD800: case 0xDC00:         /*Floating point ROM*/
			return afloat_ptr[addr & 0x0FFF];

		case 0xE000: case 0xE400: case 0xE800: case 0xEC00:         /*Disc ROM*/
			return dosrom_ptr[addr & 0x0FFF];

		case 0xF000: case 0xF400: case 0xF800: case 0xFC00:         /*Kernel*/
			return akernel_ptr[addr & 0x0FFF];
		}
	}
	else
	{
		switch (addr & 0xFC00)
		{
		case 0x0000: case 0x0400: case 0x0800: case 0x0C00:         /*RAM*/
		case 0x1000: case 0x1400: case 0x1800: case 0x1C00:
		case 0x2000: case 0x2400: case 0x2800: case 0x2C00:
		case 0x3000: case 0x3400: case 0x3800: case 0x3C00:
			return ram[addr];

		case 0x4000: case 0x4400: case 0x4800: case 0x4C00:         /*Video RAM*/
		case 0x5000: case 0x5400: case 0x5800: case 0x5C00:
			if (snow && cycles >= 0 && cycles < 32)
				fetcheddat[31 - cycles] = ram[addr];
			return ram[addr + 0x4000];

		case 0x7000:         /*8255 PIA*/
			return read8255(addr);

		case 0x7800:         /*6522 VIA*/
			return readvia(addr);

		case 0x8000: case 0x8400: case 0x8800: case 0x8C00:
		case 0x9000: case 0x9400: case 0x9800: case 0x9C00:
		case 0xA000: case 0xA400: case 0xA800: case 0xAC00:
		case 0xB000: case 0xB400: case 0xB800: case 0xBC00:
			return rom[(addr & 0x3FFF) + ROM_OFS_BBC_BASIC];        /*Not implemented*/

		case 0xF000: case 0xF400: case 0xF800: case 0xFC00:             /*Kernel*/
			return rom[(addr & 0xFFF) + ROM_OFS_BBC_OS];            /*Not implemented*/
		}
	}
	return 0;
//        printf("Error : Bad read from %04X\n",addr);
//        dumpregs();
//        exit(-1);
}

void writememl(uint16_t addr, uint8_t val)
{
	if (debugon)
		debugwrite(addr, val);
	writec[addr] = 31;
	
	if (!bbcmode)
	{
		switch (addr & 0xFC00)
		{
		case 0x0000:         /*Block zero RAM*/
			ram[addr] = val;
			return;

		case 0x0400:						   case 0x0C00:
		case 0x1000: case 0x1400: case 0x1800: case 0x1C00:
		case 0x2000: case 0x2400:         /*DOS RAM*/
		case 0x2800: case 0x2C00:
		case 0x3000: case 0x3400: case 0x3800: case 0x3C00:
		case 0x4000: case 0x4400: case 0x4800: case 0x4C00:
		case 0x5000: case 0x5400: case 0x5800: case 0x5C00:
		case 0x6000: case 0x6400: case 0x6800: case 0x6C00:
			ram[addr] = val;
			return;

		case 0x7000: case 0x7400: case 0x7800: case 0x7C00:
			if(!ramrom_enable || (RR_enables & RAMROM_FLAG_EXTRAM)==0)
			{
				ram[addr] = val;
				return;
			}
			
		case 0x8000: case 0x8400: case 0x8800: case 0x8C00:         /*Video RAM*/
		case 0x9000: case 0x9400: case 0x9800: case 0x9C00:
			if (snow && cycles >= 0 && cycles < 32)
				fetcheddat[31 - cycles] = val;
			ram[addr] = val;
			return;

		case 0x0800:
			if ((addr & 0x0F00) == 0xA00)	/*FDC*/
			{
				if(ramrom_enable && RR_BLKA_enabled())
					ram[addr]=val;
				else
					write8271(addr, val); 
				
				return;
			}   
			else
				ram[addr]=val;
                                                      
			return;
			
		case 0xB000:         /*8255 PIA*/
			write8255(addr, val);
			return;

		case 0xB800:         /*6522 VIA*/
			if (addr < 0xB810)
				writevia(addr, val);
//                        if (addr=0xBFFF) rpclog("Write BFFF %02X\n",val);
			return;
		
		case 0xB400:
			
//			debuglog("addr=%04X, val=%02X\n",addr,val);
			
			WriteMMC(addr,val);
			return;
			
		case 0xBC00:
			if((sndatomsid) && (addr>=0xBDC0) && (addr<=0xBDDF))
			{
				sid_write(addr & 0x1F,val);
				return;
			}
	
			if(ramrom_enable)
			{
				switch(addr)
				{
					case 0xBFFF :
						RR_bankreg = val & 0x0F;
						set_rr_ptrs();
						break;
					
					case 0xBFFE :
						RR_enables = val & 0x0F;
						set_rr_ptrs();
						break;
				}
			}
		

		case 0xA000: case 0xA400: case 0xA800: case 0xAC00:             /*Utility ROM*/
			// Special case of RAM mapped into utility rom space and rom bank 0 selected
			if(ramrom_enable && (RR_enables & RAMROM_FLAG_EXTRAM) && (RR_bankreg==0))
				utility_ptr[addr & 0x0FFF]=val;
			break;
			
		case 0xC000: case 0xC400: case 0xC800: case 0xCC00:             /*BASIC*/
		case 0xD000: case 0xD400: case 0xD800: case 0xDC00:             /*Floating point ROM*/
		case 0xE000: case 0xE400: case 0xE800: case 0xEC00:             /*Disc ROM*/
		case 0xF000: case 0xF400: case 0xF800: case 0xFC00:             /*Kernel*/

			return;
		}
	}
	else
	{
		switch (addr & 0xFC00)
		{
		case 0x0000: case 0x0400: case 0x0800: case 0x0C00:         /*RAM*/
		case 0x1000: case 0x1400: case 0x1800: case 0x1C00:
		case 0x2000: case 0x2400: case 0x2800: case 0x2C00:
		case 0x3000: case 0x3400: case 0x3800: case 0x3C00:
			ram[addr] = val;
			return;

		case 0x4000: case 0x4400: case 0x4800: case 0x4C00:         /*Video RAM*/
		case 0x5000: case 0x5400: case 0x5800: case 0x5C00:
			if (snow && cycles >= 0 && cycles < 32)
				fetcheddat[31 - cycles] = val;
			ram[addr + 0x4000] = val;
			return;

		case 0x7000:         /*8255 PIA*/
			write8255(addr, val);
			return;

		case 0x7800:         /*6522 VIA - not emulated*/
			writevia(addr, val);
			return;

		case 0x8000: case 0x8400: case 0x8800: case 0x8C00:
		case 0x9000: case 0x9400: case 0x9800: case 0x9C00:
		case 0xA000: case 0xA400: case 0xA800: case 0xAC00:
		case 0xB000: case 0xB400: case 0xB800: case 0xBC00:
			return;

		case 0xF000: case 0xF400: case 0xF800: case 0xFC00:         /*Kernel*/
			return;
		}
	}
	return;
//        printf("Error : Bad write to %04X data %02X\n",addr,val);
//        dumpregs();
//        exit(-1);
}


#define readmem(a) readmeml(a)
#define writemem(a, v) writememl(a, v)
//#define readmem(a) ((memstat[(a)>>8]==2)?readmeml(a):mem[(a)>>8][(a)&0xFF])
//#define writemem(a,b) if (memstat[(a)>>8]==0) mem[(a)>>8][(a)&0xFF]=b; else if (memstat[(a)>>8]==2) writememl(a,b,lines);
inline uint16_t getsw()
{
	uint16_t temp = readmem(pc); pc++;

	temp |= (readmem(pc) << 8); pc++;
	return temp;
}
#define getw() getsw()
//#define getw() (readmem(pc)|(readmem(pc+1)<<8)); pc+=2

void reset6502()
{
//        atexit(dumpram);
	reset_rom();
	pc = readmem(0xFFFC) | (readmem(0xFFFD) << 8);
	p.i = 1;
	nmi = oldnmi = nmilock = 0;
}

void dumpregs()
{
	dumpram();
	printf("6502 registers :\n");
	printf("A=%02X X=%02X Y=%02X S=01%02X PC=%04X\n", a, x, y, s, pc);
	printf("Status : %c%c%c%c%c%c\n", (p.n) ? 'N' : ' ', (p.v) ? 'V' : ' ', (p.d) ? 'D' : ' ', (p.i) ? 'I' : ' ', (p.z) ? 'Z' : ' ', (p.c) ? 'C' : ' ');
}

#define setzn(v) p.z = !(v); p.n = (v) & 0x80

#define push(v) ram[0x100 + (s--)] = v
#define pull()  ram[0x100 + (++s)]

#define polltime(c) { cycles -= c; tapecyc -= (c << 2); if (tapecyc < 0) polltape(); totcyc += c; via.t1c -= c; if (!(via.acr & 0x20)) via.t2c -= c; if (via.t1c < -1 || via.t2c < -1) updatetimers(); }

/*ADC/SBC temp variables*/
uint16_t tempw;
int tempv, hc, al, ah;
uint8_t tempb;

#define ADC(temp)       if (!p.d)			     \
	{				   \
		tempw = (a + temp + (p.c ? 1 : 0));	   \
		p.v = (!((a ^ temp) & 0x80) && ((a ^ tempw) & 0x80));  \
		a = tempw & 0xFF;		   \
		p.c = tempw & 0x100;		      \
		setzn(a);		   \
	}				   \
	else				   \
	{				   \
		ah = 0;	       \
		tempb = a + temp + (p.c ? 1 : 0);			     \
		if (!tempb)					 \
			p.z = 1;					  \
		al = (a & 0xF) + (temp & 0xF) + (p.c ? 1 : 0);				  \
		if (al > 9)					   \
		{						 \
			al -= 10;				   \
			al &= 0xF;				   \
			ah = 1;					   \
		}						 \
		ah += ((a >> 4) + (temp >> 4));				    \
		if (ah & 8) p.n = 1;				       \
		p.v = (((ah << 4) ^ a) & 128) && !((a ^ temp) & 128);	\
		p.c = 0;					     \
		if (ah > 9)					   \
		{						 \
			p.c = 1;				     \
			ah -= 10;				   \
			ah &= 0xF;				   \
		}						 \
		a = (al & 0xF) | (ah << 4);				 \
	}

#define SBC(temp)       if (!p.d)			     \
	{				   \
		tempw = a - (temp + (p.c ? 0 : 1));    \
		tempv = (int16_t)a - (int16_t)(temp + (p.c ? 0 : 1));		 \
		p.v = ((a ^ (temp + (p.c ? 0 : 1))) & (a ^ (int16_t)tempv) & 0x80); \
		p.c = tempv >= 0; \
		a = tempw & 0xFF;	       \
		setzn(a);		   \
	}				   \
	else				   \
	{				   \
		hc = 0;				      \
		p.z = p.n = 0;				  \
		if (!((a - temp) - ((p.c) ? 0 : 1)))		\
			p.z = 1;			     \
		al = (a & 15) - (temp & 15) - ((p.c) ? 0 : 1);	    \
		if (al & 16)			       \
		{				    \
			al -= 6;		      \
			al &= 0xF;		      \
			hc = 1;			      \
		}				    \
		ah = (a >> 4) - (temp >> 4);		    \
		if (hc) ah--;			    \
		if ((a - (temp + ((p.c) ? 0 : 1))) & 0x80)	  \
			p.n = 1;			     \
		p.v = (((a - (temp + ((p.c) ? 0 : 1))) ^ temp) & 128) && ((a ^ temp) & 128); \
		p.c = 1; \
		if (ah & 16)			       \
		{				    \
			p.c = 0; \
			ah -= 6;		      \
			ah &= 0xF;		      \
		}				    \
		a = (al & 0xF) | ((ah & 0xF) << 4);		    \
	}

int lns;
uint8_t opcode;
void exec6502(int linenum, int cpl)
{
	uint16_t addr;
	uint8_t temp;
	int tempi;
	int8_t offset;
	int c;
	int lines;
	int oldcyc;

	for (lines = 0; lines < linenum; lines++)
	{
//                rpclog("Exec line %i\n",lines);
		lns = lines;
		if (lines < 262 || lines == 311)
			drawline(lines);
		pollsound();
		cycles += cpl;
//                badline=0;
		while (cycles > 0)
		{
			oldcyc = cycles;
			if (skipint == 1)
				skipint = 0;
			if (debugon)
				dodebugger();
			opcode = readmem(pc); pc++;
			switch (opcode)
			{
			case 0x00:         /*BRK*/
/*                                printf("BRK at %04X\n",pc);
                                dumpregs();
                                dumpram();
                                exit(-1);*/
				pc++;
				push(pc >> 8);
				push(pc & 0xFF);
				temp = 0x30;
				if (p.c)
					temp |= 1;
				if (p.z)
					temp |= 2;
				if (p.d)
					temp |= 8;
				if (p.v)
					temp |= 0x40;
				if (p.n)
					temp |= 0x80;
				push(temp);
				pc = readmem(0xFFFE) | (readmem(0xFFFF) << 8);
				p.i = 1;
				polltime(7);
				break;

			case 0x01:         /*ORA (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				a |= readmem(addr);
				setzn(a);
				polltime(6);
				break;

			case 0x05:         /*ORA zp*/
				addr = readmem(pc); pc++;
				a |= readmem(addr);
				setzn(a);
				polltime(3);
				break;

			case 0x06:         /*ASL zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				p.c = temp & 0x80;
				temp <<= 1;
				setzn(temp);
				writemem(addr, temp);
				polltime(5);
				break;

			case 0x08:         /*PHP*/
				temp = 0x30;
				if (p.c)
					temp |= 1;
				if (p.z)
					temp |= 2;
				if (p.i)
					temp |= 4;
				if (p.d)
					temp |= 8;
				if (p.v)
					temp |= 0x40;
				if (p.n)
					temp |= 0x80;
				push(temp);
				polltime(3);
				break;

			case 0x09:         /*ORA imm*/
				a |= readmem(pc); pc++;
				setzn(a);
				polltime(2);
				break;

			case 0x0A:         /*ASL A*/
				p.c = a & 0x80;
				a <<= 1;
				setzn(a);
				polltime(2);
				break;

			case 0x0B:         /*ANC imm*/
				a &= readmem(pc); pc++;
				setzn(a);
				p.c = p.n;
				polltime(2);
				break;

			case 0x0D:         /*ORA abs*/
				addr = getw();
				a |= readmem(addr);
				setzn(a);
				polltime(4);
				break;

			case 0x0E:         /*ASL abs*/
				addr = getw();
				temp = readmem(addr);
				p.c = temp & 0x80;
				temp <<= 1;
				setzn(temp);
				writemem(addr, temp);
				polltime(6);
				break;

			case 0x10:         /*BPL*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (!p.n)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0x11:         /*ORA (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a |= readmem(addr + y);
				setzn(a);
				polltime(5);
				break;

			case 0x15:         /*ORA zp,x*/
				addr = readmem(pc); pc++;
				a |= ram[(addr + x) & 0xFF];
				setzn(a);
				polltime(3);
				break;

			case 0x16:         /*ASL zp,x*/
				addr = (readmem(pc) + x) & 0xFF; pc++;
				temp = ram[addr];
				p.c = temp & 0x80;
				temp <<= 1;
				setzn(temp);
				ram[addr] = temp;
				polltime(5);
				break;

			case 0x18:         /*CLC*/
				p.c = 0;
				polltime(2);
				break;

			case 0x19:         /*ORA abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a |= readmem(addr + y);
				setzn(a);
				polltime(4);
				break;

			case 0x1D:         /*ORA abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				addr += x;
				a |= readmem(addr);
				setzn(a);
				polltime(4);
				break;

			case 0x1E:         /*ASL abs,x*/
				addr = getw(); addr += x;
				temp = readmem(addr);
				p.c = temp & 0x80;
				temp <<= 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(7);
				break;

			case 0x20:         /*JSR*/
				addr = getw(); pc--;
				push(pc >> 8);
				push(pc);
				pc = addr;
				polltime(6);
				break;

			case 0x21:         /*AND (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				a &= readmem(addr);
				setzn(a);
				polltime(6);
				break;

			case 0x24:         /*BIT zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				p.z = !(a & temp);
				p.v = temp & 0x40;
				p.n = temp & 0x80;
				polltime(3);
				break;

			case 0x25:         /*AND zp*/
				addr = readmem(pc); pc++;
				a &= readmem(addr);
				setzn(a);
				polltime(3);
				break;

			case 0x26:         /*ROL zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				tempi = p.c;
				p.c = temp & 0x80;
				temp <<= 1;
				if (tempi)
					temp |= 1;
				setzn(temp);
				writemem(addr, temp);
				polltime(5);
				break;

			case 0x28:         /*PLP*/
				temp = pull();
				p.c = temp & 1; p.z = temp & 2;
				p.i = temp & 4; p.d = temp & 8;
				p.v = temp & 0x40; p.n = temp & 0x80;
				polltime(4);
				break;

			case 0x29:         /*AND*/
				a &= readmem(pc); pc++;
				setzn(a);
				polltime(2);
				break;

			case 0x2A:         /*ROL A*/
				tempi = p.c;
				p.c = a & 0x80;
				a <<= 1;
				if (tempi)
					a |= 1;
				setzn(a);
				polltime(2);
				break;

			case 0x2C:         /*BIT abs*/
				addr = getw();
				temp = readmem(addr);
				p.z = !(a & temp);
				p.v = temp & 0x40;
				p.n = temp & 0x80;
				polltime(4);
				break;

			case 0x2D:         /*AND abs*/
				addr = getw();
				a &= readmem(addr);
				setzn(a);
				polltime(4);
				break;

			case 0x2E:         /*ROL abs*/
				addr = getw();
				temp = readmem(addr);
				tempi = p.c;
				p.c = temp & 0x80;
				temp <<= 1;
				if (tempi)
					temp |= 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(6);
				break;

			case 0x30:         /*BMI*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (p.n)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0x31:         /*AND (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a &= readmem(addr + y);
				setzn(a);
				polltime(5);
				break;

			case 0x35:         /*AND zp,x*/
				addr = readmem(pc); pc++;
				a &= ram[(addr + x) & 0xFF];
				setzn(a);
				polltime(3);
				break;

			case 0x36:         /*ROL zp,x*/
				addr = readmem(pc); pc++;
				addr += x; addr &= 0xFF;
				temp = ram[addr];
				tempi = p.c;
				p.c = temp & 0x80;
				temp <<= 1;
				if (tempi)
					temp |= 1;
				setzn(temp);
				ram[addr] = temp;
				polltime(5);
				break;

			case 0x38:         /*SEC*/
				p.c = 1;
				polltime(2);
				break;

			case 0x39:         /*AND abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a &= readmem(addr + y);
				setzn(a);
				polltime(4);
				break;

			case 0x3D:         /*AND abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				addr += x;
				a &= readmem(addr);
				setzn(a);
				polltime(4);
				break;

			case 0x3E:         /*ROL abs,x*/
				addr = getw(); addr += x;
				temp = readmem(addr);
				tempi = p.c;
				p.c = temp & 0x80;
				temp <<= 1;
				if (tempi)
					temp |= 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(7);
				break;

			case 0x40:         /*RTI*/
//                                output=0;
				temp = pull();
				p.c = temp & 1; p.z = temp & 2;
				p.i = temp & 4; p.d = temp & 8;
				p.v = temp & 0x40; p.n = temp & 0x80;
				pc = pull();
				pc |= (pull() << 8);
				polltime(6);
				nmilock = 0;
				break;

			case 0x41:         /*EOR (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				a ^= readmem(addr);
				setzn(a);
				polltime(6);
				break;

			case 0x45:         /*EOR zp*/
				addr = readmem(pc); pc++;
				a ^= readmem(addr);
				setzn(a);
				polltime(3);
				break;

			case 0x46:         /*LSR zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				p.c = temp & 1;
				temp >>= 1;
				setzn(temp);
				writemem(addr, temp);
				polltime(5);
				break;

			case 0x48:         /*PHA*/
				push(a);
				polltime(3);
				break;

			case 0x49:         /*EOR*/
				a ^= readmem(pc); pc++;
				setzn(a);
				polltime(2);
				break;

			case 0x4A:         /*LSR A*/
				p.c = a & 1;
				a >>= 1;
				setzn(a);
				polltime(2);
				break;

			case 0x4C:         /*JMP*/
				addr = getw();
				pc = addr;
				polltime(3);
				break;

			case 0x4D:         /*EOR abs*/
				addr = getw();
				a ^= readmem(addr);
				setzn(a);
				polltime(4);
				break;

			case 0x4E:         /*LSR abs*/
				addr = getw();
				polltime(4);
				temp = readmem(addr);
				polltime(1);
				writemem(addr, temp);
				polltime(1);
				p.c = temp & 1;
				temp >>= 1;
				setzn(temp);
				writemem(addr, temp);
				polltime(6);
				break;

			case 0x50:         /*BVC*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (!p.v)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0x51:         /*EOR (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a ^= readmem(addr + y);
				setzn(a);
				polltime(5);
				break;

			case 0x55:         /*EOR zp,x*/
				addr = readmem(pc); pc++;
				a ^= ram[(addr + x) & 0xFF];
				setzn(a);
				polltime(3);
				break;

			case 0x56:         /*LSR zp,x*/
				addr = (readmem(pc) + x) & 0xFF; pc++;
				temp = ram[addr];
				p.c = temp & 1;
				temp >>= 1;
				setzn(temp);
				ram[addr] = temp;
				polltime(5);
				break;

			case 0x58:         /*CLI*/
//                                if (pc<0x8000) printf("CLI at %04X\n",pc);
				p.i = 0;
				skipint = 1;
				polltime(2);
				break;

			case 0x59:         /*EOR abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a ^= readmem(addr + y);
				setzn(a);
				polltime(4);
				break;

			case 0x5D:         /*EOR abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				addr += x;
				a ^= readmem(addr);
				setzn(a);
				polltime(4);
				break;

			case 0x5E:         /*LSR abs,x*/
				addr = getw(); addr += x;
				temp = readmem(addr);
				p.c = temp & 1;
				temp >>= 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(7);
				break;

			case 0x60:         /*RTS*/
				pc = pull();
				pc |= (pull() << 8);
				pc++;
				polltime(6);
				break;

			case 0x61:         /*ADC (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				temp = readmem(addr);
				ADC(temp);
				polltime(6);
				break;

			case 0x65:         /*ADC zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				ADC(temp);
				polltime(3);
				break;

			case 0x66:         /*ROR zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				tempi = p.c;
				p.c = temp & 1;
				temp >>= 1;
				if (tempi)
					temp |= 0x80;
				setzn(temp);
				writemem(addr, temp);
				polltime(5);
				break;

			case 0x68:         /*PLA*/
				a = pull();
				setzn(a);
				polltime(4);
				break;

			case 0x69:         /*ADC imm*/
				temp = readmem(pc); pc++;
				ADC(temp);
				polltime(2);
				break;

			case 0x6A:         /*ROR A*/
				tempi = p.c;
				p.c = a & 1;
				a >>= 1;
				if (tempi)
					a |= 0x80;
				setzn(a);
				polltime(2);
				break;

			case 0x6C:         /*JMP ()*/
				addr = getw();
				if ((addr & 0xFF) == 0xFF)
					pc = readmem(addr) | (readmem(addr - 0xFF) << 8);
				else
					pc = readmem(addr) | (readmem(addr + 1) << 8);
				polltime(5);
				break;

			case 0x6D:         /*ADC abs*/
				addr = getw();
				temp = readmem(addr);
				ADC(temp);
				polltime(4);
				break;

			case 0x6E:         /*ROR abs*/
				addr = getw();
				polltime(4);
				temp = readmem(addr);
				polltime(1);
				writemem(addr, temp);
				polltime(1);
				tempi = p.c;
				p.c = temp & 1;
				temp >>= 1;
				if (tempi)
					temp |= 0x80;
				setzn(temp);
				writemem(addr, temp);
				break;

			case 0x70:         /*BVS*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (p.v)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0x71:         /*ADC (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				temp = readmem(addr + y);
				ADC(temp);
				polltime(5);
				break;

			case 0x75:         /*ADC zp,x*/
				addr = readmem(pc); pc++;
				temp = ram[(addr + x) & 0xFF];
				ADC(temp);
				polltime(4);
				break;

			case 0x76:         /*ROR zp,x*/
				addr = readmem(pc); pc++;
				addr += x; addr &= 0xFF;
				temp = ram[addr];
				tempi = p.c;
				p.c = temp & 1;
				temp >>= 1;
				if (tempi)
					temp |= 0x80;
				setzn(temp);
				ram[addr] = temp;
				polltime(5);
				break;

			case 0x78:         /*SEI*/
//                                if (pc<0x8000) printf("SEI at %04X\n",pc);
				p.i = 1;
				polltime(2);
//                                if (output2) printf("SEI at line %i %04X %02X %02X\n",lines,pc,ram[0x103+s],ram[0x104+s]);
				break;

			case 0x79:         /*ADC abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				temp = readmem(addr + y);
				ADC(temp);
				polltime(4);
				break;

			case 0x7D:         /*ADC abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				addr += x;
				temp = readmem(addr);
				ADC(temp);
				polltime(4);
				break;

			case 0x7E:         /*ROR abs,x*/
				addr = getw(); addr += x;
				temp = readmem(addr);
				tempi = p.c;
				p.c = temp & 1;
				temp >>= 1;
				if (tempi)
					temp |= 0x80;
				writemem(addr, temp);
				setzn(temp);
				polltime(7);
				break;

			case 0x81:         /*STA (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				writemem(addr, a);
				polltime(6);
				break;

			case 0x84:         /*STY zp*/
				addr = readmem(pc); pc++;
				writemem(addr, y);
				polltime(3);
				break;

			case 0x85:         /*STA zp*/
				addr = readmem(pc); pc++;
				writemem(addr, a);
				polltime(3);
				break;

			case 0x86:         /*STX zp*/
				addr = readmem(pc); pc++;
				writemem(addr, x);
				polltime(3);
				break;

			case 0x88:         /*DEY*/
				y--;
				setzn(y);
				polltime(2);
				break;

			case 0x8A:         /*TXA*/
				a = x;
				setzn(a);
				polltime(2);
				break;

			case 0x8C:         /*STY abs*/
				addr = getw();
				polltime(3);
				writemem(addr, y);
				polltime(1);
				break;

			case 0x8D:         /*STA abs*/
				addr = getw();
				polltime(3);
				writemem(addr, a);
				polltime(1);
				break;

			case 0x8E:         /*STX abs*/
				addr = getw();
				polltime(3);
				writemem(addr, x);
				polltime(1);
				break;

			case 0x90:         /*BCC*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (!p.c)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0x91:         /*STA (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8) + y;
				writemem(addr, a);
				polltime(6);
				break;

			case 0x94:         /*STY zp,x*/
				addr = readmem(pc); pc++;
				ram[(addr + x) & 0xFF] = y;
				polltime(4);
				break;

			case 0x95:         /*STA zp,x*/
				addr = readmem(pc); pc++;
				writemem((addr + x) & 0xFF, a);
//                                ram[(addr+x)&0xFF]=a;
				polltime(4);
				break;

			case 0x96:         /*STX zp,y*/
				addr = readmem(pc); pc++;
				ram[(addr + y) & 0xFF] = x;
				polltime(4);
				break;

			case 0x98:         /*TYA*/
				a = y;
				setzn(a);
				polltime(2);
				break;

			case 0x99:         /*STA abs,y*/
				addr = getw();
				polltime(4);
				writemem(addr + y, a);
				polltime(1);
				break;

			case 0x9A:         /*TXS*/
				s = x;
				polltime(2);
				break;

			case 0x9D:         /*STA abs,x*/
				addr = getw();
				polltime(4);
				writemem(addr + x, a);
				polltime(1);
				break;

			case 0xA0:         /*LDY imm*/
				y = readmem(pc); pc++;
				setzn(y);
				polltime(2);
				break;

			case 0xA1:         /*LDA (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				a = readmem(addr);
				setzn(a);
				polltime(6);
				break;

			case 0xA2:         /*LDX imm*/
				x = readmem(pc); pc++;
				setzn(x);
				polltime(2);
				break;

			case 0xA4:         /*LDY zp*/
				addr = readmem(pc); pc++;
				y = readmem(addr);
				setzn(y);
				polltime(3);
				break;

			case 0xA5:         /*LDA zp*/
				addr = readmem(pc); pc++;
				a = readmem(addr);
				setzn(a);
				polltime(3);
				break;

			case 0xA6:         /*LDX zp*/
				addr = readmem(pc); pc++;
				x = readmem(addr);
				setzn(x);
				polltime(3);
				break;

			case 0xA8:         /*TAY*/
				y = a;
				setzn(y);
				break;

			case 0xA9:         /*LDA imm*/
				a = readmem(pc); pc++;
				setzn(a);
				polltime(2);
				break;

			case 0xAA:         /*TAX*/
				x = a;
				setzn(x);
				polltime(2);
				break;

			case 0xAC:         /*LDY abs*/
				addr = getw();
				polltime(3);
				y = readmem(addr);
				setzn(y);
				polltime(1);
				break;

			case 0xAD:         /*LDA abs*/
				addr = getw();
				polltime(3);
				a = readmem(addr);
				setzn(a);
				polltime(1);
				break;

			case 0xAE:         /*LDX abs*/
				addr = getw();
				polltime(3);
				x = readmem(addr);
				setzn(x);
				polltime(1);
				break;

			case 0xB0:         /*BCS*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (p.c)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0xB1:         /*LDA (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a = readmem(addr + y);
				setzn(a);
				polltime(5);
				break;

			case 0xB4:         /*LDY zp,x*/
				addr = readmem(pc); pc++;
				y = ram[(addr + x) & 0xFF];
				setzn(y);
				polltime(3);
				break;

			case 0xB5:         /*LDA zp,x*/
				addr = readmem(pc); pc++;
				a = ram[(addr + x) & 0xFF];
				setzn(a);
				polltime(3);
				break;

			case 0xB6:         /*LDX zp,y*/
				addr = readmem(pc); pc++;
				x = ram[(addr + y) & 0xFF];
				setzn(x);
				polltime(3);
				break;

			case 0xB8:         /*CLV*/
				p.v = 0;
				polltime(2);
				break;

			case 0xB9:         /*LDA abs,y*/
				addr = getw();
				polltime(3);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				a = readmem(addr + y);
				setzn(a);
				polltime(1);
				break;

			case 0xBA:         /*TSX*/
				x = s;
				setzn(x);
				polltime(2);
				break;

			case 0xBC:         /*LDY abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				y = readmem(addr + x);
				setzn(y);
				polltime(4);
				break;

			case 0xBD:         /*LDA abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				a = readmem(addr + x);
				setzn(a);
				polltime(4);
				break;

			case 0xBE:         /*LDX abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				x = readmem(addr + y);
				setzn(x);
				polltime(4);
				break;

			case 0xC0:         /*CPY imm*/
				temp = readmem(pc); pc++;
				setzn(y - temp);
				p.c = (y >= temp);
				polltime(2);
				break;

			case 0xC1:         /*CMP (,x)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				temp = readmem(addr);
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(6);
				break;

			case 0xC4:         /*CPY zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				setzn(y - temp);
				p.c = (y >= temp);
				polltime(3);
				break;

			case 0xC5:         /*CMP zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(3);
				break;

			case 0xC6:         /*DEC zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr) - 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(5);
				break;

			case 0xC8:         /*INY*/
				y++;
				setzn(y);
				polltime(2);
				break;

			case 0xC9:         /*CMP imm*/
				temp = readmem(pc); pc++;
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(2);
				break;

			case 0xCA:         /*DEX*/
				x--;
				setzn(x);
				polltime(2);
				break;

			case 0xCC:         /*CPY abs*/
				addr = getw();
				temp = readmem(addr);
				setzn(y - temp);
				p.c = (y >= temp);
				polltime(4);
				break;

			case 0xCD:         /*CMP abs*/
				addr = getw();
				temp = readmem(addr);
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(4);
				break;

			case 0xCE:         /*DEC abs*/
				addr = getw();
				polltime(4);
				temp = readmem(addr) - 1;
				polltime(1);
				writemem(addr, temp + 1);
				polltime(1);
				writemem(addr, temp);
				setzn(temp);
				break;

			case 0xD0:         /*BNE*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (!p.z)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0xD1:         /*CMP (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				temp = readmem(addr + y);
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(5);
				break;

			case 0xD5:         /*CMP zp,x*/
				addr = readmem(pc); pc++;
				temp = ram[(addr + x) & 0xFF];
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(3);
				break;

			case 0xD6:         /*DEC zp,x*/
				addr = readmem(pc); pc++;
				ram[(addr + x) & 0xFF]--;
				setzn(ram[(addr + x) & 0xFF]);
				polltime(5);
				break;

			case 0xD8:         /*CLD*/
				p.d = 0;
				polltime(2);
				break;

			case 0xD9:         /*CMP abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				temp = readmem(addr + y);
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(4);
				break;

			case 0xDD:         /*CMP abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				temp = readmem(addr + x);
				setzn(a - temp);
				p.c = (a >= temp);
				polltime(4);
				break;

			case 0xDE:         /*DEC abs,x*/
				addr = getw(); addr += x;
				temp = readmem(addr) - 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(6);
				break;

			case 0xE0:         /*CPX imm*/
				temp = readmem(pc); pc++;
				setzn(x - temp);
				p.c = (x >= temp);
				polltime(3);
				break;

			case 0xE4:         /*CPX zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				setzn(x - temp);
				p.c = (x >= temp);
				polltime(3);
				break;

			case 0xE5:         /*SBC zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr);
				SBC(temp);
				polltime(3);
				break;

			case 0xE6:         /*INC zp*/
				addr = readmem(pc); pc++;
				temp = readmem(addr) + 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(5);
				break;

			case 0xE8:         /*INX*/
				x++;
				setzn(x);
				polltime(2);
				break;

			case 0xE9:         /*SBC imm*/
				temp = readmem(pc); pc++;
				SBC(temp);
				polltime(2);
				break;

			case 0xEA:         /*NOP*/
				polltime(2);
				break;

			case 0xEC:         /*CPX abs*/
				addr = getw();
				temp = readmem(addr);
				setzn(x - temp);
				p.c = (x >= temp);
				polltime(3);
				break;

			case 0xED:         /*SBC abs*/
				addr = getw();
				temp = readmem(addr);
				SBC(temp);
				polltime(4);
				break;

			case 0xEE:         /*DEC abs*/
				addr = getw();
				polltime(4);
				temp = readmem(addr) + 1;
				polltime(1);
				writemem(addr, temp - 1);
				polltime(1);
				writemem(addr, temp);
				setzn(temp);
				break;

			case 0xF0:         /*BEQ*/
				offset = (int8_t)readmem(pc); pc++;
				temp = 2;
				if (p.z)
				{
					temp++;
					if ((pc & 0xFF00) ^ ((pc + offset) & 0xFF00))
						temp++;
					pc += offset;
				}
				polltime(temp);
				break;

			case 0xF1:         /*SBC (),y*/
				temp = readmem(pc); pc++;
				addr = readmem(temp) + (readmem(temp + 1) << 8);
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				temp = readmem(addr + y);
				SBC(temp);
				polltime(5);
				break;

			case 0xF5:         /*SBC zp,x*/
				addr = readmem(pc); pc++;
				temp = ram[(addr + x) & 0xFF];
				SBC(temp);
				polltime(3);
				break;

			case 0xF6:         /*INC zp,x*/
				addr = readmem(pc); pc++;
				ram[(addr + x) & 0xFF]++;
				setzn(ram[(addr + x) & 0xFF]);
				polltime(5);
				break;

			case 0xF8:         /*SED*/
				p.d = 1;
				polltime(2);
				break;

			case 0xF9:         /*SBC abs,y*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + y) & 0xFF00))
					polltime(1);
				temp = readmem(addr + y);
				SBC(temp);
				polltime(4);
				break;

			case 0xFD:         /*SBC abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				temp = readmem(addr + x);
				SBC(temp);
				polltime(4);
				break;

			case 0xFE:         /*INC abs,x*/
				addr = getw(); addr += x;
				temp = readmem(addr) + 1;
				writemem(addr, temp);
				setzn(temp);
				polltime(6);
				break;

			case 0x04:         /*Undocumented - NOP zp*/
				addr = readmem(pc); pc++;
				polltime(3);
				break;

			case 0xF4:         /*Undocumented - NOP zpx*/
				addr = readmem(pc); pc++;
				polltime(4);
				break;

			case 0xA3:         /*Undocumented - LAX (,y)*/
				temp = readmem(pc) + x; pc++;
				addr = readmem(temp) | (readmem(temp + 1) << 8);
				a = x = readmem(addr);
				setzn(a);
				polltime(6);
				break;

			case 0x07:         /*Undocumented - SLO zp*/
				addr = readmem(pc); pc++;
				c = ram[addr] & 0x80;
				ram[addr] <<= 1;
				a |= ram[addr];
				setzn(a);
				polltime(5);
				break;

			case 0x23:              /*Undocumented - RLA*/
				break;          /*This was found in Repton 3 and
				                   looks like a mistake, so I'll
				                   ignore it for now*/

			case 0x2F:              /*Undocumented - RLA abs*/
				addr = getw();  /*Found in The Hobbit*/
				temp = readmem(addr);
				tempi = p.c;
				p.c = temp & 0x80;
				temp <<= 1;
				if (tempi)
					temp |= 1;
				writemem(addr, temp);
				a &= temp;
				setzn(a);
				polltime(6);
				break;

			case 0x4B:         /*Undocumented - ASR*/
				a &= readmem(pc); pc++;
				p.c = a & 1;
				a >>= 1;
				setzn(a);
				polltime(2);
				break;

			case 0x67:         /*Undocumented - RRA zp*/
				addr = readmem(pc); pc++;
				ram[addr] >>= 1;
				if (p.c)
					ram[addr] |= 1;
				temp = ram[addr];
				ADC(temp);
				polltime(5);
				break;

			case 0x80:         /*Undocumented - NOP imm*/
				readmem(pc); pc++;
				polltime(2);
				break;

			case 0x87:         /*Undocumented - SAX zp*/
				addr = readmem(pc); pc++;
				ram[addr] = a & x;
				polltime(3);
				break;

			case 0x9C:         /*Undocumented - SHY abs,x*/
				addr = getw();
				writemem(addr + x, y & ((addr >> 8) + 1));
				polltime(5);
				break;

			case 0xDA:         /*Undocumented - NOP*/
//                                case 0xFA:
				polltime(2);
				break;

			case 0xDC:         /*Undocumented - NOP abs,x*/
				addr = getw();
				if ((addr & 0xFF00) ^ ((addr + x) & 0xFF00))
					polltime(1);
				readmem(addr + x);
				polltime(4);
				break;

			default:
				switch (opcode & 0xF)
				{
				case 0xA:
					break;
				case 0x0:
				case 0x2:
				case 0x3:
				case 0x4:
				case 0x7:
				case 0x9:
				case 0xB:
					pc++;
					break;
				case 0xC:
				case 0xE:
				case 0xF:
					pc += 2;
					break;
				}
			}
			oldcyc -= cycles;
/*                        if (!pc)
                        {
                                printf("PC at 0\n");
                                dumpregs();
                                dumpram();
                                exit(-1);
                        }*/
//                        if (pc==0x8000) a=1;
			if (pc == 0xC2CF || pc < 0x8000)
				tapeon = 0;
			if (pc == 0xFB8E)
				tapeon = 1;
/*                        if (pc==0xFC23)
                        {
                                rpclog("Cassette byte - %02X %c",a,a);
                                rpclog("\n");
                                if (a!=lastdat) rpclog("Doesn't match!\n");
                        }*/
/*                        if (pc==0xFC1E) rpclog("COS received %02X %c\n",a,(a&0xE0)?a:a+0x20);
                        if (pc==0xFBC7) rpclog("Preamble test returned - %i\n",p.c);
                        if (pc==0xFBD3) rpclog("Finished loading filename - %02X %02X %02X %02X %02X %02X\n",ram[0xED],ram[0xEE],ram[0xEF],ram[0xF0],ram[0xF1],ram[0xF2]);
                        if (pc==0xFBDB) rpclog("Compare %02X with %02X - %i\n",a,ram[0xED+y],p.z);
                        if (pc==0xFBE1) rpclog("Filename match\n");
                        if (pc==0xF9E0) rpclog("Loading block...\n");
                        if (pc==0xFA07) rpclog("Finished block, %i\n",p.c);*/
//                        if (pc==0xF172) output=1;
//                        if (output) rpclog("%02X A=%02X X=%02X Y=%02X PC=%04X %c%c%c%c%c%c %i\n",opcode,a,x,y,pc,(p.n)?'N':' ',(p.v)?'V':' ',(p.d)?'D':' ',(p.i)?'I':' ',(p.z)?'Z':' ',(p.c)?'C':' ',totcyc);
			ins++;
/*                        if (timetolive)
                        {
                                timetolive--;
                                if (!timetolive) exit(-1);
                        }*/
			if (nmi && !oldnmi)
			{
				push(pc >> 8);
				push(pc & 0xFF);
				temp = 0x20;
				if (p.c)
					temp |= 1;
				if (p.z)
					temp |= 2;
				if (p.i)
					temp |= 4;
				if (p.d)
					temp |= 8;
				if (p.v)
					temp |= 0x40;
				if (p.n)
					temp |= 0x80;
				push(temp);
				pc = readmem(0xFFFA) | (readmem(0xFFFB) << 8);
				p.i = 1;
				polltime(7);
				nmi = 0;
				nmilock = 1;
//                                rpclog("NMI %04X\n",pc);
			}
			oldnmi = nmi;

			if (motoron)
			{
				if (fdctime)
				{
					fdctime -= oldcyc; if (fdctime <= 0)
						fdccallback();
				}
				disctime -= oldcyc; if (disctime <= 0)
				{
					disctime += 8; disc_poll();
				}
			}

			if ((interrupt && !p.i && !skipint) || skipint == 2)
			{
//                                printf("Intrupt\n");
//                                if (skipint==2) printf("interrupt\n");
//				rpclog("Interrupt\n");
				skipint = 0;
				push(pc >> 8);
				push(pc & 0xFF);
				temp = 0x20;
				if (p.c)
					temp |= 1;
				if (p.z)
					temp |= 2;
				if (p.i)
					temp |= 4;
				if (p.d)
					temp |= 8;
				if (p.v)
					temp |= 0x40;
				if (p.n)
					temp |= 0x80;
				push(temp);
				pc = readmem(0xFFFE) | (readmem(0xFFFF) << 8);
				p.i = 1;
				polltime(7);
//                                if (pc<0x100)
//                                {
//                                        printf("Interrupt line %i %04X %04X %02X %02X %02X %02X %i %i %i\n",lines,cia2.t1c,cia2.t2c,cia1.ifr,vic.ifr,cia2.ifr,cia2.ier,nmi,oldnmi,nmilock);
//                                        output2=1;
//                                }
//                                output=1;
//                                printf("Interrupt line %i %i %02X %02X %02X %02X\n",interrupt,lines,sysvia.ifr&sysvia.ier,uservia.ifr&uservia.ier,uservia.ier,uservia.ifr);
			}
			if (interrupt && !p.i && skipint)
			{
				skipint = 2;
//                                printf("skipint=2\n");
			}
		}
		if (motorspin)
		{
			motorspin--;
			if (motorspin <= 0)
			{
				motorspin = 0;
				fdcspindown();
			}
		}
	}
//        rpclog("Exec over\n");
}

