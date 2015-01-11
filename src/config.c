/*Atomulator v1.0 by Tom Walker
  Configuration handling*/

#include <string.h>
#include <stdio.h>
#include <allegro.h>
#include "atom.h"

int snow;
int defaultwriteprot;
char discfns[2][260];

void loadconfig()
{
        int c;
        char s[256];
        char *p;
        sprintf(s,"%satom.cfg",exedir);
        //printf("%s\n",s);
        set_config_file(s);

        p=(char *)get_config_string(NULL,"disc0",NULL);
        if (p) strcpy(discfns[0],p);
        else   discfns[0][0]=0;
        p=(char *)get_config_string(NULL,"disc1",NULL);
        if (p) strcpy(discfns[1],p);
        else   discfns[1][0]=0;

        colourboard=get_config_int(NULL,"colourboard",1);
        bbcmode=get_config_int(NULL,"bbcbasic",0);
        snow=get_config_int(NULL,"snow",0);
        
        fasttape=get_config_int(NULL,"fasttape",0);
        
        defaultwriteprot=get_config_int(NULL,"defaultwriteprot",1);
        ddvol=get_config_int(NULL,"ddvol",2);
        ddtype=get_config_int(NULL,"ddtype",0);
        
        spon=get_config_int(NULL,"snd_internal",1);
        tpon=get_config_int(NULL,"snd_tape",0);
        sndddnoise=get_config_int(NULL,"snd_ddnoise",1);

        
        for (c=0;c<128;c++)
        {
                sprintf(s,"key_define_%03i",c);
                keylookup[c]=get_config_int("user_keyboard",s,c);
        }
}

void saveconfig()
{
        int c;
        char s[256];
        sprintf(s,"%satom.cfg",exedir);
        set_config_file(s);

        set_config_string(NULL,"disc0",discfns[0]);
        set_config_string(NULL,"disc1",discfns[1]);

        set_config_int(NULL,"colourboard",colourboard);
        set_config_int(NULL,"bbcbasic",bbcmode);
        set_config_int(NULL,"snow",snow);
        set_config_int(NULL,"fasttape",fasttape);
        
        set_config_int(NULL,"defaultwriteprot",defaultwriteprot);
        
        set_config_int(NULL,"ddvol",ddvol);
        set_config_int(NULL,"ddtype",ddtype);

        set_config_int(NULL,"snd_internal",spon);
        set_config_int(NULL,"snd_tape",tpon);
        set_config_int(NULL,"snd_ddnoise",sndddnoise);

        for (c=0;c<128;c++)
        {
                sprintf(s,"key_define_%03i",c);
                set_config_int("user_keyboard",s,keylookup[c]);
        }
}
