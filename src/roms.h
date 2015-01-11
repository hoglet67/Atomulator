/*
        Roms.h, defines for handling roms of Atomulator.

        2012-06-01, P.Harvey-Smith.
 */

// Size of ROM memory, and rom blocks
#define ROM_MEM_SIZE            0xA000
#define ROM_SIZE_ATOM           0x1000
#define ROM_SIZE_BBC_BASIC      0x4000

// ROM offsets within rom memory
#define ROM_OFS_UTILITY         0x0000
#define ROM_OFS_ABASIC          0x1000
#define ROM_OFS_AFLOAT          0x2000
#define ROM_OFS_DOSROM          0x3000
#define ROM_OFS_AKERNEL         0x4000
#define ROM_OFS_BBC_OS          0x5000
#define ROM_OFS_BBC_BASIC       0x6000

// The rom for the RAM ROM board is a 128K rom laid out as follows
// 0x00000 - 0x0FFFF up to 16 utility roms in slots 00..0F
// 0x10000 - 0x10FFF Atom basic rom
// 0x11000 - 0x11FFF Atom floating point rom
// 0x12000 - 0x12FFF Atom DOS / AtoMMC rom
// 0x13000 - 0x13FFF Atom Kernel rom
// 0x14000 - 0x1FFFF Unused

#define RAM_ROM_SIZE            0x20000
#define ROM_OFS_RAMROM          0x0A000
#define RAM_ROM_ROMS            0x10
#define RAM_ROM_OFS_SYSROM      (RAM_ROM_ROMS * ROM_SIZE_ATOM)
#define ROM_OFS_RR_ABASIC       (ROM_OFS_RAMROM + RAM_ROM_OFS_SYSROM + 0x0000)
#define ROM_OFS_RR_AFLOAT       (ROM_OFS_RAMROM + RAM_ROM_OFS_SYSROM + 0x1000)
#define ROM_OFS_RR_DOSROM       (ROM_OFS_RAMROM + RAM_ROM_OFS_SYSROM + 0x2000)
#define ROM_OFS_RR_AKERNEL      (ROM_OFS_RAMROM + RAM_ROM_OFS_SYSROM + 0x3000)

// if emuspeed is faster than this then ramrom returns fast speed.
#define RAMROM_EMU_FAST			6

// RAMROM bitmaps for 0xBFFD
#define RAMROM_FLAG_JMPDISK		0x04
#define	RAMROM_FLAG_FAST		0x08

// RAMROM bitmaps for 0xBFFE
#define RAMROM_FLAG_EXTRAM		0x01
#define RAMROM_FLAG_BLKA_RAM	0x02
#define RAMROM_FLAG_DISKROM		0x04

#define RR_bit_set(bit)			(0!=((RR_enables ^ RR_jumpers) & bit))
#define RR_BLKA_enabled()		(0!=((RR_jumpers & RAMROM_FLAG_JMPDISK) ^ ((RR_enables & RAMROM_FLAG_BLKA_RAM)<<1)))

