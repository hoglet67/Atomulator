Atomulator
~~~~~~~~~~

Atomulator is an emulator of an Acorn Atom - the first micro from Acorn in 1980.

It emulates a 12+16k Atom with colour board, disc pack, and optional BBC BASIC mode.

Atomulator is licensed under the GPL, see COPYING for more details.

Command Line Options
~~~~~~~~~~~~~~~~~~~~

-help           - show help
-fullscreen     - start in fullscreen mode
-disc disc.ssd  - load disc.ssd into drives :0/:2
-disc1 disc.ssd - load disc.ssd into drives :1/:3
-tape tape.uef  - load tape.uef
-fasttape       - set tape speed to fast
-autoboot       - cause atommc to auto boot on power on reset
-debug          - start debugger

Usage
~~~~~

Just run Atomulator.exe. The Atom isn't a very friendly machine, so some basic tips :


Loading a program off tape :

Load the tape image via the menu. You will need to know the name of the file needed, use
the tape catalogue viewer to find this out. For example, Pinball by Bug-Byte has the following
files :

INSTRUCTIONS  Size 03C2 Load 2900 Run C2B2
PINBALL       Size 0AFF Load 2900 Run 2900

To load the game you would need to type 'LOAD "PINBALL"' and hit enter. The emulator will then
proceed to load the game - the tape starts and stops automatically. The Atom gives no messages 
while loading so you will need to be patient! When it's finished loading, type 'RUN' and hit enter.


Loading a program off disc :

Load the disc image via the menu. To enable the disc you will need to type '*DOS'. The machine is
then in disc mode. Typing '*.' will give a catalogue. You then need to identify which file you need
and then LOAD and RUN it as above.

Loading a program off mmc :

Start the machine with ramrom enabled : settings->ramrom->ramrom enabled, and RamRom disk rom 
enabled : settings->ramrom->ramrom diskrom enabled. When the machine is powered on or break is 
pressed you should see "ACORN ATOM + ATOMMC2" as the signon.

Once this is enabled, you should be able to do a *CAT and list the files in the mmc folder, you
can drop files into here as you would do onto the MMC card, and they will be available to 
manipulate with the standard Atom SAVE/LOAD etc commands. A program can also be loaded by typing
*PROGRAMNAME

For mor information see documentation at :

Loading a program from emulated disk image on MMC :

The RAMROM image also contains SDDOS by Kees van Oss, this allows disk images to be loaded from 
the MMC card as if they where real disks. This can be initialised by typing SDDOS (note no 
preceeding *), with ramrom active. The *DHELP command will give a list of the available commands

Disk images can be mounted with the *DIN command :

*DIN <driveno> <imagename>

Commands are used much like their Atom DOS equivelents.

Further documentation can be found here :

Menu
~~~~

File -> Reset                  - reset the emulated Atom
        Exit                   - exits back to Windows

Tape -> Load tape...           - select a new CSW or UEF tape image
        Rewind tape            - rewinds tape image to the start
        Tape catalogue         - displays a file catalogue of the tape
        Fast tape              - accelerates tape access

Disc -> Load disc 0/2          - load a disc image into drives 0 and 2.
        Load disc 1/3          - load a disc image into drives 1 and 3.
        Eject disc 0/2         - removes disc image from drives 0 and 2.
        Eject disc 1/3         - removes disc image from drives 1 and 3.
        New disc 0/2           - creates a new DFS/ADFS disc and loads it into drives 0 and 2.
        New disc 1/3           - creates a new DFS/ADFS disc and loads it into drives 1 and 3.
        Write protect disc 0/2 - toggles write protection on drives 0 and 2.
        Write protect disc 1/3 - toggles write protection on drives 1 and 3.
        Default write protect  - determines whether loaded discs are write protected by default

Settings ->
        Video ->    Snow              - emulate authentic Atom snow
                    Fullscreen        - switches to full screen mode. Use ALT-ENTER to leave

        Hardware -> Colour board      - emulates an Atom colour board. This allows colour in several 
                                        video modes, and slows video refresh from 60hz to 50hz
                    BBC BASIC         - emulates a BBC BASIC language board. This allows a superior
                                        BASIC, but the disc drive is not functional in BBC BASIC mode
                                        (this seems to be the case with the real machine also)
        
        RamRom ->   RAM/ROM Enabled   - Enables emulation of the Ramoth RAM/ROM clockboost board.
                    RAM/ROM dosrom Enabled                      
                                      - Enables the AtoMMC #E000 rom instead of Atom DOS.

        Sound ->    Atom sound        - enables the sound output from the Atom
                    Tape sound        - enables the sound from tape
                    AtomSID           - enables the AtomSID interface.

                    reSID configuration :
                        Model         - choose between many different models of SID. Many tunes sound quite 
                                        different depending on the model chosen.
                        Sample method - choose between interpolation and resampling. Resampling is in theory 
                                        higher quality, but I can't tell the difference.
                    
                    Disc noise        - enables disc drive noise simulation
                    Disc drive type   - choose between sound from 5.25" drive or 3.5" drive.
                    Disc drive volume - set the relative volume of the disc drive noise.



        Keyboard -> Redefine keys     - redefine PC -> Atom key mapping
                    Default mapping   - restore the default keyboard mapping

Misc -> Debugger (Windows only) - open the built-in 6502 debugger. Type '?' for a list of commands
        Break (Windows only)    - break into the debugger


Notes
~~~~~

- The keyboard isn't too responsive for typing. A real Atom is like this as well.

- Atom BASIC is quite non-standard, if you are having problems track down the Atom manual
  'Atomic Theory and Practice' - it's been scanned and is available on the internet.

- When you enter a graphics mode in BASIC the text display doesn't work. This is faithful to
  the real machine as well.

- Atom sound is that bad, and faithful to the real machine.

- Saving via tape is not supported. Saving via disc is however.

- There may be one or two bugs in the tape emulation with some images. This doesn't affect functionality,
  but does give some oddities in messages displayed.

Tom Walker
b-em@bbcmicro.com

Version 1.1 patches by Phill Harvey-Smith.
