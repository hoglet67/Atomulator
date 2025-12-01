# Target OS detection
ifeq ($(OS),Windows_NT) # OS is a preexisting environment variable on Windows
	OS = windows
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Linux)
		OS = linux
	else
    	$(error OS not supported by this Makefile)
	endif
endif

SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Commands
MKDIR := mkdir

ifeq ($(OS),windows)
	VPATH = $(SRC_DIR) $(SRC_DIR)/resid-fp $(SRC_DIR)/AtoMMC
	CPP  = g++.exe
    CC   = gcc.exe
    WINDRES = windres.exe
    CFLAGS = -O3 -ffast-math -fomit-frame-pointer -falign-loops -falign-jumps -falign-functions
    # -ggdb -march=i686
    OBJ = 6502.o 6522via.o 8255.o 8271.o 1770.o atom.o config.o csw.o ddnoise.o debugger.o disc.o fdi.o fdi2raw.o soundopenal.o ssd.o uef.o video.o avi.o win.o win-keydefine.o resid.o atom.res atommc.o
    SIDOBJ = convolve-sse.o convolve.o envelope.o extfilt.o filter.o pot.o sid.o voice.o wave6581__ST.o wave6581_P_T.o wave6581_PS_.o wave6581_PST.o wave8580__ST.o wave8580_P_T.o wave8580_PS_.o wave8580_PST.o wave.o
    MMCOBJ = atmmc2core.o atmmc2wfn.o ff_emu.o ff_emudir.o wildcard.o

    DEFS = 	-DINCLUDE_SDDOS -Ilib/allegro4/include -Ilib/allegro4/Build/include -Ilib/openal-soft/include -Ilib/freealut/include $(EXTRA_DEFS)
    LIBS =  -mwindows \
    		-Llib/allegro4/Build/lib \
    		-Llib/openal-soft/build \
    		-Llib/freealut/build/src \
    		-lalleg44.dll \
    		-lz \
    		-lalut.dll \
    		-lOpenAL32.dll \
    		-lwinmm \
    		-lstdc++ \
    		-static \
    		-static-libgcc \
    		-static-libstdc++

    TARGET_BIN = Atomulator.exe
else ifeq ($(OS),linux)
	VPATH = $(SRC_DIR) $(SRC_DIR)/resid-fp $(SRC_DIR)/atommc
	CPP  = g++
    CC   = gcc
    WINDRES =
    CFLAGS = -O3 -ffast-math -fomit-frame-pointer -falign-loops -falign-jumps -falign-functions -DINCLUDE_SDDOS
    # -march=i686 -ggdb
    OBJ = 6502.o 6522via.o 8255.o 8271.o atom.o config.o csw.o ddnoise.o debugger.o disc.o fdi.o fdi2raw.o soundopenal.o ssd.o uef.o video.o avi.o linux.o linux-keydefine.o linux-gui.o resid.o 1770.o atommc.o
    SIDOBJ = convolve-sse.o convolve.o envelope.o extfilt.o filter.o pot.o sid.o voice.o wave6581__ST.o wave6581_P_T.o wave6581_PS_.o wave6581_PST.o wave8580__ST.o wave8580_P_T.o wave8580_PS_.o wave8580_PST.o wave.o
    MMCOBJ = atmmc2core.o atmmc2wfn.o ff_emu.o ff_emudir.o wildcard.o

    DEFS = $(EXTRA_DEFS)
    LIBS =  -lalleg -lz -lalut -lopenal -lstdc++ -L/usr/local/lib -lm

	MKDIR += -p

    TARGET_BIN = Atomulator
else
	$(error Should not happen)
endif

FULLOBJ = $(foreach objname, $(OBJ), $(BUILD_DIR)/$(objname))
FULLSIDOBJ = $(foreach objname, $(SIDOBJ), $(BUILD_DIR)/resid-fp/$(objname))
FULLMMCOBJ = $(foreach objname, $(MMCOBJ), $(BUILD_DIR)/atommc/$(objname))

help:
	@echo Available targets: all, clean
	@echo See $(MAKEFILE_TARGET) in src for more targets

$(BIN_DIR)/$(TARGET_BIN) : $(FULLOBJ) $(FULLSIDOBJ) $(FULLMMCOBJ) | $(BIN_DIR)
	$(CC) $^ -o $(BIN_DIR)/$(TARGET_BIN) $(LIBS)

all : $(BIN_DIR)/$(TARGET_BIN)

clean :
	$(RM) $(BUILD_DIR)/*.o
	$(RM) $(BUILD_DIR)/atommc/*.o
	$(RM) $(BUILD_DIR)/resid-fp/*.o
	$(RM) $(BIN_DIR)/$(TARGET_BIN)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(BUILD_DIR)/atommc/%.o : $(SRC_DIR)/atommc/%.c | $(BUILD_DIR)/atommc
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(BUILD_DIR)/atommc/%.o : $(SRC_DIR)/atommc/%.cc | $(BUILD_DIR)/atommc
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(BUILD_DIR)/resid-fp/%.o : $(SRC_DIR)/resid-fp/%.c | $(BUILD_DIR)/resid-fp
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(BUILD_DIR)/resid-fp/%.o : $(SRC_DIR)/resid-fp/%.cc | $(BUILD_DIR)/resid-fp
	$(CC) $(CFLAGS) $(DEFS) -c $< -o $@

$(BUILD_DIR)/atom.res: src/atom.rc | $(BUILD_DIR)
	$(WINDRES) -i $(SRC_DIR)/atom.rc --input-format=rc -o $(BUILD_DIR)/atom.res -O coff

$(BUILD_DIR) : ;
	$(MKDIR) $(BUILD_DIR)

$(BUILD_DIR)/resid-fp : $(BUILD_DIR)
	$(MKDIR) "$(BUILD_DIR)/resid-fp"

$(BUILD_DIR)/atommc : $(BUILD_DIR)
	$(MKDIR) "$(BUILD_DIR)/atommc"

$(BIN_DIR) : ;
	$(MKDIR) $(BIN_DIR)
