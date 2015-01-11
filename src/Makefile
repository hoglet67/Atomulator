VPATH = . resid-fp AtoMMC
CPP  = g++.exe
CC   = gcc.exe
WINDRES = windres.exe
CFLAGS = -O3 -march=i686 -ffast-math -fomit-frame-pointer -falign-loops -falign-jumps -falign-functions 
# -ggdb
OBJ = 6502.o 6522via.o 8255.o 8271.o atom.o config.o csw.o ddnoise.o debugger.o disc.o fdi.o fdi2raw.o soundopenal.o ssd.o uef.o video.o win.o win-keydefine.o resid.o atom.res
SIDOBJ = convolve-sse.o convolve.o envelope.o extfilt.o filter.o pot.o sid.o voice.o wave6581__ST.o wave6581_P_T.o wave6581_PS_.o wave6581_PST.o wave8580__ST.o wave8580_P_T.o wave8580_PS_.o wave8580_PST.o wave.o
MMCOBJ = atommc.o atmmc2core.o atmmc2wfn.o ff_emu.o ff_emudir.o wildcard.o 

DEFS = 	-DINCLUDE_SDDOS
LIBS =  -mwindows -lalleg -lz -lalut -lopenal32 -lwinmm -lstdc++

Atomulator.exe: $(OBJ) $(SIDOBJ) $(MMCOBJ)
	$(CC) $(OBJ) $(SIDOBJ) $(MMCOBJ) -o "Atomulator.exe" $(LIBS) -static -static-libgcc -static-libstdc++

all : Atomulator.exe

clean :
	rm *.o
	rm *.exe
	rm *.res

%.o : %.c
	$(CC) $(CFLAGS) $(DEFS) -c $<

%.o : %.cc
	$(CPP) $(CFLAGS) -c $<

atom.res: atom.rc
	$(WINDRES) -i atom.rc --input-format=rc -o atom.res -O coff 

