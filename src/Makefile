VPATH = .
CC   = gcc.exe
WINDRES = windres.exe
CFLAGS = -O3 -march=i686 -ffast-math -fomit-frame-pointer -falign-loops -falign-jumps -falign-functions
OBJ = 6502.o 6522via.o 8255.o 8271.o atom.o config.o csw.o ddnoise.o debugger.o disc.o fdi.o fdi2raw.o soundopenal.o ssd.o uef.o video.o win.o win-keydefine.o atom.res

LIBS =  -mwindows -lalleg -lz -lalut -lopenal32 -lwinmm

Atomulator.exe: $(OBJ)
	$(CC) $(OBJ) -o "Atomulator.exe" $(LIBS)

all : Atomulator.exe

clean :
	del *.o
	del *.exe
	del *.res

%.o : %.c
	$(CC) $(CFLAGS) -c $<

%.o : %.cc
	$(CPP) $(CFLAGS) -c $<

atom.res: atom.rc
	$(WINDRES) -i atom.rc --input-format=rc -o atom.res -O coff 