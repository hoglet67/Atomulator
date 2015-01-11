/*Atomulator v1.0 by Tom Walker
   6522 VIA emulation*/

#include <stdio.h>
#include "atom.h"

#define TIMER1INT 	0x40
#define TIMER2INT 	0x20
#define PORTBINT  	0x18
#define PORTAINT  	0x03

#define ORB			0x00
#define ORA     	0x01
#define DDRB    	0x02
#define DDRA    	0x03
#define T1CL    	0x04
#define T1CH    	0x05
#define T1LL    	0x06
#define T1LH    	0x07
#define T2CL    	0x08
#define T2CH    	0x09
#define SR      	0x0a
#define ACR     	0x0b
#define PCR     	0x0c
#define IFR     	0x0d
#define IER     	0x0e
#define ORAnh   	0x0f

void updateIFR()
{
	if ((via.ifr & 0x7F) & (via.ier & 0x7F))
	{
		via.ifr |= 0x80;
		interrupt |= 2;
	}
	else
	{
		via.ifr &= ~0x80;
		interrupt &= ~2;
	}
}

int timerout = 1;
int lns;
void updatetimers()
{
	if (via.t1c < -3)
	{
		while (via.t1c < -3)
			via.t1c += via.t1l + 4;
		
		if (!via.t1hit)
		{
			via.ifr |= TIMER1INT;
			updateIFR();
		}
		
		if ((via.acr & 0x80) && !via.t1hit)
		{
			via.orb ^= 0x80;
			via.irb ^= 0x80;
			via.portb ^= 0x80;
			timerout ^= 1;
		}
		
		if (!(via.acr & 0x40))
			via.t1hit = 1;
	}
	
	if (!(via.acr & 0x20) /* && !via.t2hit*/)
	{
		if (via.t2c < -3 && !via.t2hit)
		{
//                        via.t2c+=via.t2l+4;
//                        rpclog(" Timer 2 reset %05X %05X %04X\n",via.t2c,via.t2l,pc);
			if (!via.t2hit)
			{
				via.ifr |= TIMER2INT;
				updateIFR();
//                                output=1;
			}
			via.t2hit = 1;
		}
	}
}

void writevia(uint16_t addr, uint8_t val)
{
        rpclog("VIA write %04X %02X %04X\n",addr,val,pc);
	switch (addr & 0xF)
	{
	case ORA:
		via.ifr &= 0xfc; //~PORTAINT;
		updateIFR();
	case ORAnh:
		via.ora = val;
		via.porta = (via.porta & ~via.ddra) | (via.ora & via.ddra);
		break;

	case ORB:
		via.orb = val;
		via.portb = (via.portb & ~via.ddrb) | (via.orb & via.ddrb);
		via.ifr &= 0xfe; //~PORTBINT;
		updateIFR();
		break;

	case DDRA:
		via.ddra = val;
		break;
	case DDRB:
		via.ddrb = val;
		break;
	case ACR:
		via.acr = val;
		break;
	case PCR:
		via.pcr = val;
		break;
	case T1LL:
	case T1CL:
//                printf("T1L write %02X at %04X %i\n",val,pc,lns);
		via.t1l &= 0xFF00;
		via.t1l |= val;
		break;
	case T1LH:
//                printf("T1LH write %02X at %04X %i\n",val,pc,lns);
		via.t1l &= 0xFF;
		via.t1l |= (val << 8);
		if (via.acr & 0x40)
		{
			via.ifr &= ~TIMER1INT;
			updateIFR();
		}
//                printf("%04X\n",via.t1l>>1);
		break;
	case T1CH:
		if ((via.acr & 0xC0) == 0x80)
			timerout = 0;
//                printf("T1CH write %02X at %04X %i\n",val,pc,lns);
		via.t1l &= 0xFF;
		via.t1l |= (val << 8);
//                if (via.t1c<1) printf("UT1 reload %i\n",via.t1c);
//                printf("T1 l now %05X\n",via.t1l);
		via.t1c = via.t1l + 1;
		via.ifr &= ~TIMER1INT;
		updateIFR();
		via.t1hit = 0;
		break;
	case T2CL:
		via.t2l &= 0xFF00;
		via.t2l |= val;
//                printf("T2CL=%02X at line %i\n",val,line);
		break;
	case T2CH:          // && !(via.ifr&TIMER2INT))
		if ((via.t2c == -3 && (via.ier & TIMER2INT)) ||
		    (via.ifr & via.ier & TIMER2INT))
		{
			interrupt |= 128;
//                        rpclog("uTimer 2 extra interrupt\n");
		}
//                if (output) rpclog("Write uT2CH %i\n",via.t2c);
		via.t2l &= 0xFF;
		via.t2l |= (val << 8);
//                if (via.t2c<1) printf("UT2 reload %i\n",via.t2c);
		via.t2c = via.t2l + 1;
		via.ifr &= ~TIMER2INT;
		updateIFR();
		via.t2hit = 0;
//                output=0;
//                printf("T2CH=%02X at line %i\n",val,line);
		break;
	case IER:
/*                if (val==0x40)
                {
                        printf("Here\n");
   //                        output=1;
                }*/
		if (val & 0x80)
			via.ier |= (val & 0x7F);
		else
			via.ier &= ~(val & 0x7F);
		updateIFR();
//                rpclog("Write IER %02X %04X %02X\n",val,pc,via.ier);
//                if (via.ier&0x40) printf("0x40 enabled at %04X\n",pc);
//                via.ifr&=~via.ier;
		break;
	case IFR:
		via.ifr &= ~(val & 0x7F);
		updateIFR();
//                rpclog("Write IFR %02X %04X %02X\n",val,pc,via.ifr);
		break;
	}
}

uint8_t readvia(uint16_t addr)
{
	uint8_t temp;

//        if (addr>=4 && addr<=9) printf("Read U %04X %04X\n",addr,pc);
//        rpclog("Read  VIA %04X %04X\n",addr,pc);
	switch (addr & 0xF)
	{
	case ORA:
		via.ifr &= ~PORTAINT;
		updateIFR();
	case ORAnh:
		temp = via.ora & via.ddra;
		temp |= (via.porta & ~via.ddra);
		temp &= 0x7F;
		return temp;

	case ORB:
//                via.ifr&=~PORTBINT;
		updateIFR();
		temp = via.orb & via.ddrb;
		if (via.acr & 2)
			temp |= (via.irb & ~via.ddrb);
		else
			temp |= (via.portb & ~via.ddrb);
		temp |= 0xFF;
		if (timerout)
			temp |= 0x80;
		else
			temp &= ~0x80;
//                printf("ORB read %02X\n",temp);
//                temp|=0xF0;
		return temp;

	case DDRA:
		return via.ddra;
	case DDRB:
		return via.ddrb;
	case T1LL:
//                printf("Read T1LL %02X %04X\n",(via.t1l&0x1FE)>>1,via.t1l);
		return via.t1l & 0xFF;
	case T1LH:
//                printf("Read T1LH %02X\n",via.t1l>>9);
		return via.t1l >> 8;
	case T1CL:
		via.ifr &= ~TIMER1INT;
		updateIFR();
//                printf("Read T1CL %02X %i %08X\n",((via.t1c+2)>>1)&0xFF,via.t1c,via.t1c);
		if (via.t1c < -1)
			return 0xFF;
		return via.t1c & 0xFF;
	case T1CH:
		if (via.t1c < -1)
			return 0xFF;
		return via.t1c >> 8;
	case T2CL:
		via.ifr &= ~TIMER2INT;
		updateIFR();
//                printf("Read T2CL %02X\n",((via.t2c+2)>>1)&0xFF);
//                if (via.t2c<0) return 0xFF;
		return via.t2c & 0xFF;
	case T2CH:
//                printf("Read T2CH %02X\n",((via.t2c+2)>>1)>>8);
//                printf("T2CH read %05X %04X %02X %04X %i %02X\n",via.t2c,via.t2c>>1,via.t2c>>9,pc,p.i,a);
//                if (via.t2c<0) return 0xFF;
		return via.t2c >> 8;
	case ACR:
		return via.acr;
	case PCR:
		return via.pcr;
	case IER:
		return via.ier | 0x80;
	case IFR:
//                rpclog("IFR %02X\n",via.ifr);
		return via.ifr;
	}
	return 0xFF;
}

void resetvia()
{
	via.ora = 0x80;
	via.ifr = via.ier = 0;
	via.t1c = via.t1l = 0x1FFFE;
	via.t2c = via.t2l = 0x1FFFE;
	via.t1hit = via.t2hit = 1;
	timerout = 1;
	via.acr = 0;
	
	// To make sure interrupts get cleared at reset
	updateIFR();
}

void dumpvia()
{
	rpclog("T1 = %04X %04X T2 = %04X %04X\n", via.t1c, via.t1l, via.t2c, via.t2l);
	rpclog("%02X %02X  %02X %02X\n", via.ifr, via.ier, via.pcr, via.acr);
}
