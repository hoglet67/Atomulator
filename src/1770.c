/*Adapted from Elkulator v1.0 by Tom Walker
  1770 FDC emulation*/
#include <stdio.h>
#include <stdlib.h>
#include "atom.h"
#include "1770.h"

extern int output; /* defined in 6502.c */
#ifndef ABS
#define ABS(x) (((x)>0)?(x):-(x))
#endif

void callback1770();
void data1770(uint8_t dat);
void spindown1770();
void finishread1770();
void notfound1770();
void datacrcerror1770();
void headercrcerror1770();
void writeprotect1770();
int getdata1770(int last);
/* PHS set status of interrupt lines */
void setdrq(int state);
void setintrq(int state);
/* end */

struct
{
        uint8_t command,sector,track,status,data;
        uint8_t ctrl;
        int curside,curtrack;
        int density;
        int written;
        int stepdir;
        int fifo;
} wd1770;

static int byte;

void reset1770()
{
        nmi=0;
		setintrq(0);
		setdrq(0);
        wd1770.status=0;
        wd1770.fifo=0;
        motorspin=0;
//        rpclog("Reset 1770\n");
        fdctime=0;
        if (fdc1770)
        {
		        fdccallback=callback1770;
                fdcdata=data1770;
                fdcspindown=spindown1770;
                fdcfinishread=finishread1770;
                fdcnotfound=notfound1770;
                fdcdatacrcerror=datacrcerror1770;
                fdcheadercrcerror=headercrcerror1770;
                fdcwriteprotect=writeprotect1770;
                fdcgetdata=getdata1770;
        }
        motorspin=45000;
// PHS - 2015-07-03, WD177x unlike the WD179x & WD279x do not generate an interrupt after reset.
//		setintrq(1);
}

void spinup1770()
{
        wd1770.status |= ST_MOTOR; // flag motor on
        motoron=1;
        motorspin=0;
}

void spindown1770()
{
        wd1770.status &= ~ST_MOTOR; // flag motor off
        motoron=0;
}

void setspindown1770()
{
        motorspin=45000;
}

#define track0 (wd1770.curtrack?4:0)

void write1770(uint16_t addr, uint8_t val)
{
        //debuglog("Write 1770 %04X %02X\n",addr,val);
        switch (addr)
        {
                case CTRLREG:
//                        rpclog("Write CTRL FE24 %02X\n",val);
					debuglog("Write CTRL %02X\n",val);
					wd1770.ctrl=val;
					if (val & CTL_DS0) curdrive=0;
					else               curdrive=1;
				
					wd1770.curside=(wd1770.ctrl & CTL_SIDE) ? 1:0;
					wd1770.density=!(wd1770.ctrl & CTL_DDEN);
					//discspd=16;//(wd1770.density)?16:32;
					
					if((val & CTL_RESET) ==0)
						reset1770();
					
					break;
					
                case WDCMD:
					if (wd1770.status & ST_BUSY && (val>>4)!=0xD) { rpclog("Command rejected\n"); return; }
//                	rpclog("FDC command %02X %i %i %i\n",val,wd1770.curside,wd1770.track,wd1770.sector);
					wd1770.command=val;
					if ((val>>4)!=0xD)/* && !(val&8)) */spinup1770();
                
					switch (val>>4)
					{
                        case 0x0: /*Restore*/
							wd1770.status=ST_MOTOR | ST_SPINUP | ST_BUSY | track0;
							disc_seek(curdrive,0);
							break;
                        
                        case 0x1: /*Seek*/
							wd1770.status=ST_MOTOR | ST_SPINUP | ST_BUSY | track0;
							disc_seek(curdrive,wd1770.data);
							break;
                        
                        case 0x2:
                        case 0x3: /*Step*/
							wd1770.status=ST_MOTOR | ST_SPINUP | ST_BUSY | track0;
							wd1770.curtrack+=wd1770.stepdir;
							if (wd1770.curtrack<0) wd1770.curtrack=0;
							disc_seek(curdrive,wd1770.curtrack);
							break;

                        case 0x4:
                        case 0x5: /*Step in*/
							wd1770.status=ST_MOTOR | ST_SPINUP | ST_BUSY | track0;
							wd1770.curtrack++;
							disc_seek(curdrive,wd1770.curtrack);
							wd1770.stepdir=1;
							break;
							
                        case 0x6:
                        case 0x7: /*Step out*/
							wd1770.status=ST_MOTOR | ST_SPINUP | ST_BUSY | track0;
							wd1770.curtrack--;
							if (wd1770.curtrack<0) wd1770.curtrack=0;
							disc_seek(curdrive,wd1770.curtrack);
							wd1770.stepdir=-1;
							break;

                        case 0x8: /*Read sector*/
							wd1770.status=ST_MOTOR | ST_BUSY;
							disc_readsector(curdrive,wd1770.sector,wd1770.track,wd1770.curside,wd1770.density);
							//printf("Read sector drv=%i sec=%i trk=%i hd=%i dns=%i\n",curdrive,wd1770.sector,wd1770.track,wd1770.curside,wd1770.density);
							byte=0;
//                        	output=1;
							break;
                        
						case 0xA: /*Write sector*/
							wd1770.status=ST_MOTOR | ST_BUSY;
							disc_writesector(curdrive,wd1770.sector,wd1770.track,wd1770.curside,wd1770.density);
							byte=0;
//                        	nmi|=2;
							setdrq(1);
							wd1770.status|=ST_DRQ;
							break;
                        
						case 0xC: /*Read address*/
							wd1770.status=ST_MOTOR | ST_BUSY;
							disc_readaddress(curdrive,wd1770.track,wd1770.curside,wd1770.density);
							byte=0;
							break;
                        
						case 0xD: /*Force interrupt*/
//                      	  rpclog("Force interrupt\n");
							fdctime=0;
							wd1770.status=ST_MOTOR | track0;
//                        	nmi=(val&8)?1:0;
							setintrq(1);
							spindown1770();
							break;
                        
						case 0xF: /*Write track*/
							wd1770.status=ST_MOTOR | ST_BUSY;
							disc_format(curdrive,wd1770.track,wd1770.curside,wd1770.density);
							break;
                        
                        default:
//                          rpclog("Bad 1770 command %02X\n",val);
/*                        	fdctime=0;
							nmi=1;
							wd1770.status=0x90;
							spindown1770();
							break;*/
							rpclog("Bad 1770 command %02X\n",val);
							dumpregs();
							dumpram();
							exit(-1);
					}	
					break;
					
                case WDTRK:
					wd1770.track=val;
					break;
					
                case WDSEC:
					wd1770.sector=val;
					break;
                
				case WDDATA:
					//nmi&=~2;
					wd1770.status &= ~ST_DRQ;
					wd1770.data=val;
					setdrq(0);
					wd1770.written=1;
					break;
        }
}

uint8_t read1770(uint16_t addr)
{
	uint8_t temp;

//    debuglog("Read 1770 %04X\n",addr);
    switch (addr)
    {
		case CTRLREG:
			wd1770.ctrl ^= CTL_INDEX;
			return wd1770.ctrl;
		case WDSTAT:
			//nmi&=~1;
			setintrq(0);
//          rpclog("Status %02X\n",wd1770.status);
            return wd1770.status;
        
		case WDTRK:
			return wd1770.track;
        
		case WDSEC:
			return wd1770.sector;
        
        case WDDATA:
			temp=wd1770.data;
            //nmi&=~2;
			wd1770.status &= ~ST_DRQ;
			setdrq(0);
//          rpclog("Read data %02X %04X\n",temp,pc);
            return temp;
        }
        return 0xFE;
}

void callback1770()
{
//        rpclog("FDC callback %02X\n",wd1770.command);
        fdctime=0;
        switch (wd1770.command>>4)
        {
			case 0: /*Restore*/
                wd1770.curtrack=wd1770.track=0;
                wd1770.status=ST_MOTOR;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
//                disc_seek(curdrive,0);
                break;
            
			case 1: /*Seek*/
                wd1770.curtrack=wd1770.track=wd1770.data;
                wd1770.status=ST_MOTOR | track0;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
//                disc_seek(curdrive,wd1770.curtrack);
                break;
            
			case 3: /*Step*/
            case 5: /*Step in*/
            case 7: /*Step out*/
                wd1770.track=wd1770.curtrack;
            
		    case 2: /*Step*/
            case 4: /*Step in*/
            case 6: /*Step out*/
				wd1770.status=ST_MOTOR | track0;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
                break;

			case 8: /*Read sector*/
                wd1770.status=ST_MOTOR;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
                break;
            
			case 0xA: /*Write sector*/
                wd1770.status=ST_MOTOR;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
                break;
            
			case 0xC: /*Read address*/
                wd1770.status=ST_MOTOR;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
                wd1770.sector=wd1770.track;
                break;
            
			case 0xF: /*Write tracl*/
                wd1770.status=ST_MOTOR;
                setspindown1770();
				setintrq(1);
//                nmi|=1;
                break;
        }
}

void data1770(uint8_t dat)
{
        if (!(wd1770.status & ST_BUSY)) return;
        if (wd1770.status & ST_DRQ)
        {
                fdctime=0;
                wd1770.status=ST_MOTOR | ST_LOST; // lost data
                spindown1770();
                rpclog("Overrun\n");
                return;
        }
        wd1770.data=dat;
        wd1770.status |= ST_DRQ;
		setdrq(1);
}

void finishread1770()
{
        fdctime=200;
}

void notfound1770()
{
//        rpclog("Not found\n");
        fdctime=0;
//        nmi=1;
        wd1770.status=ST_MOTOR | ST_RNF; // record not found
        setintrq(1);
		spindown1770();
}

void datacrcerror1770()
{
//        rpclog("Data CRC\n");
        fdctime=0;
//        nmi=1;
        wd1770.status=ST_MOTOR | ST_CRC;
        setintrq(1);
		spindown1770();
}

void headercrcerror1770()
{
//        rpclog("Header CRC\n");
        fdctime=0;
//        nmi=1;
        wd1770.status=ST_MOTOR | ST_RNF | ST_CRC;
        setintrq(1);
		spindown1770();
}

int getdata1770(int last)
{
        if (!(wd1770.status & ST_BUSY)) return 0xFF;
//        rpclog("Disc get data\n");
        if (!wd1770.written) return -1;
        if (!last)
        {
//                nmi|=2;
                wd1770.status |= ST_DRQ;
				setdrq(1);
        }
        wd1770.written=0;
        return wd1770.data;
}

void writeprotect1770()
{
        fdctime=0;
//        nmi=1;
        wd1770.status=ST_MOTOR | ST_WRITEP;
		setintrq(1);
        spindown1770();
}

void setdrq(int state)
{
	//debuglog("1770:setdrq=%d\n",state);
	
	if (state)
		wd1770.ctrl |= CTL_DRQ;
	else
		wd1770.ctrl &= ~CTL_DRQ;
}

void setintrq(int state)
{
	//debuglog("1770:setintrq=%d\n",state);

	if (state)
		nmi = 1;
	else
		nmi	= 0;
}
