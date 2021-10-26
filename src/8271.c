/*Atomulator v1.0 by Tom Walker
   8271 FDC emulation*/

#include <stdio.h>
#include <stdlib.h>
#include "atom.h"

void callback8271();
void data8271(uint8_t dat);
void spindown8271();
void finishread8271();
void notfound8271();
void datacrcerror8271();
void headercrcerror8271();
void writeprotect8271();
int getdata8271(int last);

static int byte;
int verify8271 = 0;
struct
{
	uint8_t command, params[5];
	int drivesel;
	int paramnum, paramreq;
	uint8_t status;
	uint8_t result;
	int curtrack[2], cursector;
	int realtrack[2];
	int sectorsleft;
	uint8_t data;
	int phase;
	int written;

	uint8_t drvout;
} i8271;

void reset8271()
{
	fdccallback = callback8271;
	fdcdata = data8271;
	fdcspindown = spindown8271;
	fdcfinishread = finishread8271;
	fdcnotfound = notfound8271;
	fdcdatacrcerror = datacrcerror8271;
	fdcheadercrcerror = headercrcerror8271;
	fdcwriteprotect = writeprotect8271;
	fdcgetdata = getdata8271;

	i8271.paramnum = i8271.paramreq = 0;
	i8271.status = 0;

    debuglog("Reset 8271\n");
	fdctime = 0;
	i8271.curtrack[0] = i8271.curtrack[1] = 0;
	i8271.command = 0xFF;
	i8271.realtrack[0] = i8271.realtrack[1] = 0;
//        motorspin=45000;
}

static void NMI8271()
{
	if (i8271.status & 8)
	{
		nmi = 1;
//                rpclog("NMI\n");
	}
	else
		nmi = 0;
		
//	debuglog("FDC:NMI=%d\n",nmi);
}


void spinup8271()
{
//        rpclog("spinup8271\n");
	motoron = 1;
	motorspin = 0;
}

void spindown8271()
{
//        rpclog("spindown8271\n");
	motoron = 0;
}

void setspindown8271()
{
//        rpclog("setspindown8271\n");
	motorspin = 45000;
}


int params[][2] =
{
	{ 0x35, 4 }, { 0x29, 1 }, { 0x2C, 0 }, { 0x3D, 1 }, { 0x3A, 2 }, { 0x13, 3 }, { 0x0B, 3 }, { 0x1B, 3 }, { 0x1F, 3 }, { 0x23, 5 }, { -1, -1 }
};

int getparams8271()
{
	int c = 0;

	while (params[c][0] != -1)
	{
		if (params[c][0] == i8271.command)
			return params[c][1];
		c++;
	}
	return 0;
/*        printf("Unknown 8271 command %02X\n",i8271.command);
        dumpregs();
        exit(-1);*/
}
extern int output, timetolive; /* defined in 6502.c */
uint8_t read8271(uint16_t addr)
{
	uint8_t val;

//        if ((addr&7)!=4) rpclog("Read 8271 %04X\n",addr);
	switch (addr & 7)
	{
	case 0:         /*Status register*/
//                rpclog("Read status reg %04X %02X\n",pc,i8271.status);
		return i8271.status;
	case 1:         /*Result register*/
		i8271.status &= ~0x18;
		NMI8271();
		val = i8271.result;
//                printf("Read result reg %04X %02X\n",pc,i8271.status);
		//              output=1; timetolive=50;
//                rpclog("Read result reg %02X\n",i8271.result);
		i8271.result = 0;
		return val;
	case 4:         /*Data register*/
		i8271.status &= ~0xC;
		NMI8271();
//                printf("Read data reg %04X %02X\n",pc,i8271.status);
		return i8271.data;
	}
	return 0;
}

#define track0 (i8271.curtrack[curdrive] ? 0 : 2)

void seek8271()
{
	int diff = i8271.params[0] - i8271.curtrack[curdrive];

	i8271.realtrack[curdrive] += diff;
	disc_seek(curdrive, i8271.realtrack[curdrive]);
}

void write8271(uint16_t addr, uint8_t val)
{
//        rpclog("Write 8271 %04X %02X\n",addr,val);
    debuglog("Write 8271 %04X %02X\n",addr,val);
	switch (addr & 7)
	{
	case 0:         /*Command register*/
		if (i8271.status & 0x80)
			return;
		i8271.command = val & 0x3F;
		if (i8271.command == 0x17)
			i8271.command = 0x13;
//                printf("8271 command %02X!\n",i8271.command);
		i8271.drivesel = val >> 6;
		if (val & 0x80)
			curdrive = 1;
		else
			curdrive = 0;
		i8271.paramnum = 0;
		i8271.paramreq = getparams8271();
		i8271.status = 0x80;
		
		// No parmeters required, excute it immediatly.
		if (!i8271.paramreq)
		{
			switch (i8271.command)
			{
			case 0x2C:         /*Read drive status*/
				i8271.status = 0x10;
				i8271.result = 0x80 | 8 | track0;
				if (i8271.drivesel & 1)
					i8271.result |= 0x04;
				if (i8271.drivesel & 2)
					i8271.result |= 0x40;
//                                printf("Status %02X\n",i8271.result);
				break;

			default:
				// invalid / unknown command
				i8271.result = 0x10;
				i8271.status = 0x10;
				NMI8271();
				fdctime = 0;
				break;
//                                printf("Unknown 8271 command %02X 3\n",i8271.command);
//                                dumpregs();
//                                exit(-1);
			}
		}
		break;
	case 1:         /*Parameter register*/
		if (i8271.paramnum < 5)
			i8271.params[i8271.paramnum++] = val;
		if (i8271.paramnum == i8271.paramreq)
		{
			switch (i8271.command)
			{
			case 0x0B:         /*Write sector*/
				i8271.sectorsleft = i8271.params[2] & 31;
				i8271.cursector = i8271.params[1];
				spinup8271();
				i8271.phase = 0;
				if (i8271.curtrack[curdrive] != i8271.params[0])
					seek8271();
				else
					fdctime = 200;
				break;
			case 0x13:         /*Read sector*/
				i8271.sectorsleft = i8271.params[2] & 31;
				i8271.cursector = i8271.params[1];
//                                rpclog("Read var len %i %02X %02X\n",i8271.params[0],i8271.params[1],i8271.params[2]);
//                                if (i8271.params[0]==12) output=1;
				spinup8271();
				i8271.phase = 0;
				if (i8271.curtrack[curdrive] != i8271.params[0])
					seek8271();
				else
					fdctime = 200;
				break;
			case 0x1F:         /*Verify sector*/
				i8271.sectorsleft = i8271.params[2] & 31;
				i8271.cursector = i8271.params[1];
				spinup8271();
				i8271.phase = 0;
				if (i8271.curtrack[curdrive] != i8271.params[0])
					seek8271();
				else
					fdctime = 200;
				verify8271 = 1;
				break;
			case 0x1B:         /*Read ID*/
//                                printf("8271 : Read ID start\n");
				i8271.sectorsleft = i8271.params[2] & 31;
				spinup8271();
				i8271.phase = 0;
				if (i8271.curtrack[curdrive] != i8271.params[0])
					seek8271();
				else
					fdctime = 200;
				break;
			case 0x23:         /*Format track*/
				spinup8271();
				i8271.phase = 0;
				if (i8271.curtrack[curdrive] != i8271.params[0])
					seek8271();
				else
					fdctime = 200;
				break;
				break;
			case 0x29:         /*Seek*/
//                                fdctime=10000;
				seek8271();
				spinup8271();
				break;
			case 0x35:         /*Specify*/
				i8271.status = 0;
				break;
			case 0x3A:         /*Write special register*/
				i8271.status = 0;
//                                printf("Write special %02X\n",i8271.params[0]);
				switch (i8271.params[0])
				{
				case 0x12: i8271.curtrack[0] = val; /*printf("Write real track now %i\n",val);*/ break;
				case 0x17: break;         /*Mode register*/
				case 0x1A: i8271.curtrack[1] = val; /*printf("Write real track now %i\n",val);*/ break;
				case 0x23: i8271.drvout = i8271.params[1]; break;
				default:
					i8271.result = 0x18;
					i8271.status = 0x18;
					NMI8271();
					fdctime = 0;
					break;
//                                        default:
//                                        printf("8271 Write bad special register %02X\n",i8271.params[0]);
//                                        dumpregs();
//                                        exit(-1);
				}
				break;
			case 0x3D:         /*Read special register*/
				i8271.status = 0x10;
				i8271.result = 0;
				switch (i8271.params[0])
				{
				case 0x06: i8271.result = 0; break;
				case 0x12: i8271.result = i8271.curtrack[0]; break;
				case 0x1A: i8271.result = i8271.curtrack[1]; break;
				case 0x23: i8271.result = i8271.drvout; break;
				default:
					i8271.result = 0x18;
					i8271.status = 0x18;
					NMI8271();
					fdctime = 0;
					break;
//                                        default:
//                                        printf("8271 Read bad special register %02X\n",i8271.params[0]);
//                                        dumpregs();
//                                        exit(-1);
				}
				break;

			default:
				i8271.result = 0x18;
				i8271.status = 0x18;
				NMI8271();
				fdctime = 0;
				break;
//                                printf("Unknown 8271 command %02X 2\n",i8271.command);
//                                dumpregs();
//                                exit(-1);
			}
		}
		break;
	case 2:         /*Reset register*/
		if (val & 1)
			reset8271();
		break;
	case 4:         /*Data register*/
		i8271.data = val;
		i8271.written = 1;
		i8271.status &= ~0xC;
		NMI8271();
		break;
	}
}

void callback8271()
{
	fdctime = 0;
//        rpclog("Callback 8271 - command %02X\n",i8271.command);
	switch (i8271.command)
	{
	case 0x0B:         /*Write*/
		if (!i8271.phase)
		{
			i8271.curtrack[curdrive] = i8271.params[0];
			disc_writesector(curdrive, i8271.cursector, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
			i8271.phase = 1;

			i8271.status = 0x8C;
			i8271.result = 0;
			NMI8271();
			return;
		}
		i8271.sectorsleft--;
		if (!i8271.sectorsleft)
		{
			i8271.status = 0x18;
			i8271.result = 0;
			NMI8271();
			setspindown8271();
			verify8271 = 0;
			return;
		}
		i8271.cursector++;
		disc_writesector(curdrive, i8271.cursector, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
		byte = 0;
		i8271.status = 0x8C;
		i8271.result = 0;
		NMI8271();
		break;

	case 0x13:              /*Read*/
	case 0x1F:              /*Verify*/
		if (!i8271.phase)
		{
//                        printf("Seek to %i\n",i8271.params[0]);
			i8271.curtrack[curdrive] = i8271.params[0];
//                        i8271.realtrack+=diff;
//                        disc_seek(0,i8271.realtrack);
//                        printf("Re-seeking - track now %i %i\n",i8271.curtrack,i8271.realtrack);
//                        rpclog("Read sector %i %i\n",i8271.params[0],i8271.cursector);
			disc_readsector(curdrive, i8271.cursector, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
			i8271.phase = 1;
			return;
		}
		i8271.sectorsleft--;
		if (!i8271.sectorsleft)
		{
			i8271.status = 0x18;
			i8271.result = 0;
			NMI8271();
			setspindown8271();
			verify8271 = 0;
			return;
		}
		i8271.cursector++;
//                        rpclog("Read sector %i %i\n",i8271.params[0],i8271.cursector);
		disc_readsector(curdrive, i8271.cursector, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
		byte = 0;
		break;

	case 0x1B:         /*Read ID*/
//                printf("Read ID callback %i\n",i8271.phase);
		if (!i8271.phase)
		{
			i8271.curtrack[curdrive] = i8271.params[0];
//                        i8271.realtrack+=diff;
//                        disc_seek(0,i8271.realtrack);
			disc_readaddress(curdrive, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
			i8271.phase = 1;
			return;
		}
//                printf("Read ID track %i %i\n",i8271.params[0],i8271.sectorsleft);
		i8271.sectorsleft--;
		if (!i8271.sectorsleft)
		{
			i8271.status = 0x18;
			i8271.result = 0;
			NMI8271();
//                        printf("8271 : ID read done!\n");
			setspindown8271();
			return;
		}
		i8271.cursector++;
		disc_readaddress(curdrive, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
		byte = 0;
		break;

	case 0x23:         /*Format*/
		if (!i8271.phase)
		{
			i8271.curtrack[curdrive] = i8271.params[0];
			disc_writesector(curdrive, i8271.cursector, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
			i8271.phase = 1;

			i8271.status = 0x8C;
			i8271.result = 0;
			NMI8271();
			return;
		}
		if (i8271.phase == 2)
		{
			i8271.status = 0x18;
			i8271.result = 0;
			NMI8271();
			setspindown8271();
			verify8271 = 0;
			return;
		}
		disc_format(curdrive, i8271.params[0], (i8271.drvout & 0x20) ? 1 : 0, 0);
		i8271.phase = 2;
		break;

	case 0x29:         /*Seek*/
		i8271.curtrack[curdrive] = i8271.params[0];
//                i8271.realtrack+=diff;
		i8271.status = 0x18;
		i8271.result = 0;
		NMI8271();
//                disc_seek(0,i8271.realtrack);
//                printf("Seek done!\n");
		setspindown8271();
		break;

	case 0xFF: break;

	default: break;
//                printf("Unknown 8271 command %02X 3\n",i8271.command);
//                dumpregs();
//                exit(-1);
	}
}

void data8271(uint8_t dat)
{
	if (verify8271)
		return;
	i8271.data = dat;
	i8271.status = 0x8C;
	i8271.result = 0;
	NMI8271();
//        printf("%02X : Data %02X\n",byte,dat);
	byte++;
}

void finishread8271()
{
	fdctime = 200;
}

void notfound8271()
{
	i8271.result = 0x18;
	i8271.status = 0x18;
	NMI8271();
	fdctime = 0;
	setspindown8271();
//        printf("Not found 8271\n");
}

void datacrcerror8271()
{
	i8271.result = 0x0E;
	i8271.status = 0x18;
	NMI8271();
	fdctime = 0;
	setspindown8271();
//        printf("CRCdat 8271\n");
}

void headercrcerror8271()
{
	i8271.result = 0x0C;
	i8271.status = 0x18;
	NMI8271();
	fdctime = 0;
	setspindown8271();
//        printf("CRChead 8271\n");
}

int getdata8271(int last)
{
//        rpclog("Disc get data %i %i\n",byte,last);
	byte++;
	if (!i8271.written)
		return -1;
	if (!last)
	{
//                rpclog("Get data nmi\n");
		i8271.status = 0x8C;
		i8271.result = 0;
		NMI8271();
	}
	i8271.written = 0;
	return i8271.data;
}

void writeprotect8271()
{
	i8271.result = 0x12;
	i8271.status = 0x18;
	NMI8271();
	fdctime = 0;
}
