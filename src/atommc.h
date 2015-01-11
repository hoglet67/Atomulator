/*
	AtoMMC interface functions for Atomulator.
	
	2012-06-11, P.Harvey-Smith.
*/

#ifndef __ATOMMC__

#define DEF_MMC_DIR	"mmc"

extern char BaseMMCPath[MAXPATH+1];

void InitMMC(void);
uint8_t ReadMMC(uint16_t	addr);
void WriteMMC(uint16_t	addr,
			  uint8_t	data);

void FinalizeMMC(void);
#define __ATOMMC__
#endif