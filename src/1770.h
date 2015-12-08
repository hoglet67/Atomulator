void reset1770();
uint8_t read1770(uint16_t addr);
void write1770(uint16_t addr, uint8_t val);

#define WDBASE	0xbc10
#define CTRLREG	0xbc14

#define WDCMD	WDBASE+0
#define	WDSTAT	WDBASE+0
#define WDTRK	WDBASE+1
#define WDSEC	WDBASE+2
#define WDDATA	WDBASE+3

#define DRQ		0x01
#define INTRQ	0x02

// status register masks
#define ST_BUSY		0x01
#define ST_DRQ		0x02
#define ST_IP		0x02
#define ST_LOST		0x04
#define ST_TR00		0x04
#define ST_CRC		0x08
#define ST_RNF		0x10
#define ST_TYPE		0x20
#define	ST_SPINUP	0x20
#define ST_WRITEP	0x40
#define ST_MOTOR	0x80

// GDOS Control register masks
#define CTL_SIDE	0x01			// Side select mask
#define CTL_DS0		0x02			// Drive select 0
#define CTL_DS1		0x04			// Drive select 1
#define	CTL_DDEN	0x08			// Double density enable (low)
#define CTL_RESET	0x10			// Reset FDC
#define	CTL_4080	0x20			// 40(low)/80(high) track switch
#define	CTL_INDEX	0x40  			// Index pulse????
#define	CTL_DRQ		0x80			// DRQ
