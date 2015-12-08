/*
        Roms.h, defines for handling roms of Atomulator.

        2012-06-01, P.Harvey-Smith.
 */

// Size of ROM memory, and rom blocks

// SP7 BBC MODE PATCH

#define ROM_MEM_SIZE            0xC000

// END SP7 BBC MODE PATCH

#define ROM_SIZE_ATOM           0x1000
#define ROM_SIZE_BBC_BASIC      0x4000

// ROM offsets within rom memory
#define ROM_OFS_UTILITY         0x0000
#define ROM_OFS_ABASIC          0x1000
#define ROM_OFS_AFLOAT          0x2000
#define ROM_OFS_DOSROM          0x3000
#define ROM_OFS_AKERNEL         0x4000

// SP7 BBC MODE PATCH

#define ROM_OFS_BBC_EXT1        0x5000
#define ROM_OFS_BBC_EXT2        0x6000
#define ROM_OFS_BBC_BASIC1      0x7000
#define ROM_OFS_BBC_BASIC2      0x8000
#define ROM_OFS_BBC_BASIC3      0x9000
#define ROM_OFS_BBC_BASIC4      0xA000
#define ROM_OFS_BBC_OS          0xB000

// END SP7 BBC MODE PATCH

// As of the 2014 Hackfest, the rom for the RAM ROM board is a 128K rom laid out as follows
//
// 0x00000 - Atom #A000 Bank 0
// 0x01000 - Atom #A000 Bank 1
// 0x02000 - Atom #A000 Bank 2
// 0x03000 - Atom #A000 Bank 3
// 0x04000 - Atom #A000 Bank 4
// 0x05000 - Atom #A000 Bank 5
// 0x06000 - Atom #A000 Bank 6
// 0x07000 - Atom #A000 Bank 7
// 0x08000 - BBC #6000 Bank 0 (ExtROM1)
// 0x09000 - BBC #6000 Bank 1 (ExtROM1)
// 0x0A000 - BBC #6000 Bank 2 (ExtROM1)
// 0x0B000 - BBC #6000 Bank 3 (ExtROM1)
// 0x0C000 - BBC #6000 Bank 4 (ExtROM1)
// 0x0D000 - BBC #6000 Bank 5 (ExtROM1)
// 0x0E000 - BBC #6000 Bank 6 (ExtROM1)
// 0x0F000 - BBC #6000 Bank 7 (ExtROM1)
// 0x10000 - Atom Basic (DskRomEn=1)
// 0x11000 - Atom FP (DskRomEn=1)
// 0x12000 - Atom MMC (DskRomEn=1)
// 0x13000 - Atom Kernel (DskRomEn=1)
// 0x14000 - Atom Basic (DskRomEn=0)
// 0x15000 - Atom FP (DskRomEn=0)
// 0x16000 - unused
// 0x17000 - Atom Kernel (DskRomEn=0)
// 0x18000 - unused
// 0x19000 - BBC #7000 (ExtROM2)
// 0x1A000 - BBC Basic 1/4
// 0x1B000 - unused
// 0x1C000 - BBC Basic 2/4
// 0x1D000 - BBC Basic 3/4
// 0x1E000 - BBC Basic 4/4
// 0x1F000 - BBC MOS 3.0
//
// See: http://stardot.org.uk/forums/viewtopic.php?f=44&t=8350&p=92948#p92948

#define RAM_ROM_SIZE            0x20000

// This is the address in the Ram array that ramrom.rom is loaded to
// So all the above addresses need offseting by this amount

#define ROM_OFS_RAMROM          0x0C000

// Number of pages ROMs, same in both Atom and BBC Mode
#define RAM_ROM_ROMS            0x08

// Atom Mode
#define ROM_OFS_RR_UTILITY      (ROM_OFS_RAMROM + 0x00000)

#define ROM_OFS_RR_ABASIC1      (ROM_OFS_RAMROM + 0x10000)
#define ROM_OFS_RR_AFLOAT1      (ROM_OFS_RAMROM + 0x11000)
#define ROM_OFS_RR_DOSROM1      (ROM_OFS_RAMROM + 0x12000)
#define ROM_OFS_RR_AKERNEL1     (ROM_OFS_RAMROM + 0x13000)

#define ROM_OFS_RR_ABASIC0      (ROM_OFS_RAMROM + 0x14000)
#define ROM_OFS_RR_AFLOAT0      (ROM_OFS_RAMROM + 0x15000)
#define ROM_OFS_RR_DOSROM0      (ROM_OFS_RAMROM + 0x16000)
#define ROM_OFS_RR_AKERNEL0     (ROM_OFS_RAMROM + 0x17000)

// BBC Mode

#define ROM_OFS_RR_BBC_EXT1     (ROM_OFS_RAMROM + 0x08000)
#define ROM_OFS_RR_BBC_EXT2     (ROM_OFS_RAMROM + 0x19000)
#define ROM_OFS_RR_BBC_BASIC1   (ROM_OFS_RAMROM + 0x1A000)
#define ROM_OFS_RR_BBC_BASIC2   (ROM_OFS_RAMROM + 0x1C000)
#define ROM_OFS_RR_BBC_BASIC3   (ROM_OFS_RAMROM + 0x1D000)
#define ROM_OFS_RR_BBC_BASIC4   (ROM_OFS_RAMROM + 0x1E000)
#define ROM_OFS_RR_BBC_OS       (ROM_OFS_RAMROM + 0x1F000)   

// if emuspeed is faster than this then ramrom returns fast speed.
#define RAMROM_EMU_FAST			6

// RAMROM bitmaps for 0xBFFD/0xBFFE
#define RAMROM_FLAG_EXTRAM		0x01
#define RAMROM_FLAG_BLKA_RAM	0x02
#define RAMROM_FLAG_DISKROM		0x04
#define RAMROM_FLAG_BBCMODE		0x08

#define RR_bit_set(bit)			((RR_enables ^ RR_jumpers) & bit)

// Only enable Block A RAM if requested, and if DISKROM is not enabled
// Currently there is no GUI option to control BLKA, so the normal value of this bit is 0
// This can be toggled by setting bit 1 of ?#BFFE
// I have actually inverted it's meaning, so the default behaviour is to enable the BLKA RAM when DISROM is disabled 
#define RR_BLKA_enabled()		(!RR_bit_set(RAMROM_FLAG_BLKA_RAM) && RR_bit_set(RAMROM_FLAG_DISKROM))

// GDOS2015
// 16 banks of 4K
#define ROM_SIZE_GDOS2015		0x10000
#define ROM_OFS_GDOS2015		(ROM_OFS_RAMROM + RAM_ROM_SIZE)		
// end GDOS2015