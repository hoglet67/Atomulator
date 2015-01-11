/*Atomulator v1.0 by Tom Walker
  UEF handling (including HQ-UEF support)*/

#include <allegro.h>
#include <zlib.h>
#include <stdio.h>
#include "atom.h"

void findfilenamesuef();
int startchunk;
int blocks=0;
int tapelcount,tapellatch,pps;
//int intone=0;
gzFile *uef;

int infilenames=0;
void inituef()
{
        uef=NULL;
}

int inchunk=0,chunkid=0,chunklen=0;
int chunkpos=0,chunkdatabits=8;
float chunkf;

int getftell()
{
        return gztell(uef);
}

void openuef(char *fn)
{
      int c;
      cswena=0;
//      rpclog("gzclose\n");
      if (uef)
         gzclose(uef);
//         rpclog("gzopen\n");
      uef=gzopen(fn,"rb");
      if (!uef) return;
//      rpclog("gzgetc\n");
      for (c=0;c<12;c++)
          gzgetc(uef);
//        clearcatwindow();
//        rpclog("findfilenamesuef\n");
        findfilenamesuef();
//        rpclog("uef done\n");
//      inchunk=chunklen=chunkid=0;
}

void rewindit()
{
        int c;
        gzseek(uef,0,SEEK_SET);
        for (c=0;c<12;c++)
            gzgetc(uef);
        inchunk=chunklen=chunkid=0;
}

int ueffileopen()
{
        if (!uef)
           return 0;
        return 1;
}

int uefloop=0;
uint8_t fdat;
int ffound;
void receiveuef(uint8_t val)
{
        if (infilenames)
        {
                ffound=1;
                fdat=val;
        }
        else             receive(val);
}

void polluef()
{
        int c;
        uint32_t templ;
        float *tempf;
        uint8_t temp;
        getnewchunk:
        if (!uef)
           return;
        if (!inchunk)
        {
//                rpclog("newchunk pos %i\n",gztell(uef));
                startchunk=1;
//                printf("%i ",gztell(uef));
                gzread(uef,&chunkid,2);
                gzread(uef,&chunklen,4);
                if (gzeof(uef))
                {
                        gzseek(uef,12,SEEK_SET);
                        gzread(uef,&chunkid,2);
                        gzread(uef,&chunklen,4);
                        uefloop=1;
                }
                inchunk=1;
                chunkpos=0;
//                rpclog("Chunk ID %04X len %i\n",chunkid,chunklen);
        }
//        else
//           printf("Chunk %04X\n",chunkid);
        switch (chunkid)
        {
                case 0x000: /*Origin*/
                for (c=0;c<chunklen;c++)
                    gzgetc(uef);
                inchunk=0;
                return;

                case 0x005: /*Target platform*/
                for (c=0;c<chunklen;c++)
                    gzgetc(uef);
                inchunk=0;
                return;

                case 0x100: /*Raw data*/
                if (startchunk)
                {
                        dcdlow();
                        startchunk=0;
                }
                chunklen--;
                if (!chunklen)
                {
                        inchunk=0;
                        blocks++;
                }
                receiveuef(gzgetc(uef));
                return;

                case 0x104: /*Defined data*/
                if (!chunkpos)
                {
                        chunkdatabits=gzgetc(uef);
                        gzgetc(uef);
                        gzgetc(uef);
                        chunklen-=3;
                        chunkpos=1;
                }
//                else
//                {
                        chunklen--;
                        if (chunklen<=0)
                           inchunk=0;
                        temp=gzgetc(uef);
//                        rpclog("Read data %02X\n",temp);
//                        printf("%i : %i %02X\n",gztell(uef),chunklen,temp);
                        if (chunkdatabits==7) receiveuef(temp&0x7F);
                        else                  receiveuef(temp);
//                }
                return;

                case 0x110: /*High tone*/
                if (!infilenames) dcd();
                gzgetc(uef); gzgetc(uef);
                if (infilenames) inchunk=0;
                return;

                case 0x111: /*High tone with dummy byte*/
//                if (!intone)
//                {
                        if (!infilenames) dcd();
/*                        intone=3;
                }
                else
                {
                        if (intone==4)
                           dcd();
                        intone--;
                        if (intone==0 && inchunk==2)
                        {
                                inchunk=0;
                                gzgetc(uef); gzgetc(uef);
                                gzgetc(uef); gzgetc(uef);
                        }
                        else if (!intone)
                        {
                                inchunk=2;
                                intone=4;
                                receive(0xAA);
                        }
                }*/
                return;

                case 0x112: /*Gap*/
                if (!infilenames) dcdlow();
/*                if (!intone)
                {
//                        dcd();
                        intone=gzgetc(uef);
                        intone|=(gzgetc(uef)<<8);
                        intone>>=2;
//                        printf("gap intone %i\n",intone);
                        if (!intone) intone=1;
                }
                else
                {
                        intone--;
                        if (intone==0)
                        {
                                inchunk=0;
                        }
                }*/
                return;
/*                if (!intone)
                {
                        intone=3;
                }
                else
                {
                        intone--;
                        if (intone==0)
                        {
                                inchunk=0;
                                gzgetc(uef); gzgetc(uef);
                        }
                }*/
                return;

                case 0x113: /*Float baud rate*/
                templ=gzgetc(uef);
                templ|=(gzgetc(uef)<<8);
                templ|=(gzgetc(uef)<<16);
                templ|=(gzgetc(uef)<<24);
                tempf=(float *)&templ;
                tapellatch=(1000000/((*tempf)/10))/64;
                pps=(*tempf)/10;
                inchunk=0;
                goto getnewchunk;
                return;

                case 0x116: /*Float gap*/
                if (!chunkpos)
                {
                        templ=gzgetc(uef);
                        templ|=(gzgetc(uef)<<8);
                        templ|=(gzgetc(uef)<<16);
                        templ|=(gzgetc(uef)<<24);
                        tempf=(float *)&templ;
                        chunkf=*tempf;
//                        printf("Gap %f\n",chunkf);
                        chunkpos=1;
                        if (infilenames) inchunk=0;
                }
                else
                {
//                        printf("Gap now %f\n",chunkf);
                        chunkf-=((float)1/(float)pps);
                        if (chunkf<=0) inchunk=0;
                }
                return;

                case 0x114: /*Security waves*/
                case 0x115: /*Polarity change*/
                case 0x117: /*Data encoding change*/
//                default:
                for (c=0;c<chunklen;c++)
                    gzgetc(uef);
                inchunk=0;
                return;

//116 : float gap
//113 : float baud rate

        }
//        allegro_exit();
//        printf("Bad chunk ID %04X length %i\n",chunkid,chunklen);
//        exit(-1);
}

void closeuef()
{
        if (uef)
           gzclose(uef);
        uef=NULL;
}

uint8_t fbuffer[4];

#define getuefbyte()            ffound=0; \
                                while (!ffound && !uefloop) \
                                { \
                                        polluef(); \
                                } \
                                if (uefloop) break;

uint8_t ffilename[16];
void findfilenamesuef()
{
        int temp;
        uint8_t tb;
        int c;
        char s[256];
        int run,load;
        int offset;
        uint8_t status;
        int skip;
        if (!uef) return;
        temp=gztell(uef);
        gzseek(uef,12,SEEK_SET);
inchunk=chunklen=chunkid=0;
        uefloop=0;
        infilenames=1;
//        rpclog("ffn start\n");
//        printf("Looking!\n");
        while (!uefloop)
        {
                ffound=0;
//                rpclog("!ffound\n");
                while (!ffound && !uefloop)
                {
                        polluef();
                }
//                rpclog("ffound over %i\n",uefloop);
                if (uefloop) break;
                fbuffer[0]=fbuffer[1];
                fbuffer[1]=fbuffer[2];
                fbuffer[2]=fbuffer[3];
                fbuffer[3]=fdat;
                if (fbuffer[0]==0x2A && fbuffer[1]==0x2A && fbuffer[2]==0x2A && fbuffer[3]==0x2A)
                {
                        fbuffer[3]=0;
                        c=0;
//                        printf("Found file!\n");
//                        printf("Filename : ");
                        do
                        {
                                ffound=0;
//                rpclog("!ffound2\n");
                                while (!ffound && !uefloop)
                                {
                                        polluef();
                                }
//                rpclog("ffound2 over %i\n",uefloop);
                                if (uefloop) break;
                                ffilename[c++]=fdat;
//                                printf("%c",fdat);
                        } while (fdat!=0xD && c<15);
//                rpclog("ffound2 over2 %i\n",uefloop);
                        if (uefloop) break;
                        c--;
                        while (c<13) ffilename[c++]=32;
                        ffilename[c]=0;
                        getuefbyte();
                        status=fdat;
//                                rpclog("%s ",ffilename);
                                getuefbyte();
                                tb=fdat;
                                getuefbyte();
//                                rpclog("Block %02X%02X ",tb,fdat);
                                c=((tb*256)+fdat)*256;
                                offset=((tb*256)+fdat)*256;
                                getuefbyte();
                                c+=fdat;
                                skip=fdat;
//                                rpclog("Size %04X ",c);
                                getuefbyte();
                                tb=fdat;
                                getuefbyte();
//                                rpclog("Run %02X%02X ",tb,fdat);
                                run=(tb<<8)|fdat;
                                getuefbyte();
                                tb=fdat;
                                getuefbyte();
//                                rpclog("Load %02X%02X\n",tb,fdat);
                                load=(tb<<8)|fdat;
                                load-=offset;
                        if (!(status&0x80))
                        {
                                sprintf(s,"%s Size %04X Load %04X Run %04X",ffilename,c,load,run);
//                                rpclog("cataddname\n");
                                cataddname(s);
                        }
//                        rpclog("skip %i\n",skip);
                        for (c=0;c<skip;c++)
                        {
                                getuefbyte();
                        }
                }
//                rpclog("ffn next\n");
        }
//        rpclog("ffn done\n");
        infilenames=0;
        gzseek(uef,temp,SEEK_SET);
}
        
