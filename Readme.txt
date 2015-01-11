Atomulator v1.0
~~~~~~~~~~~~~~~

Atomulator is an emulator of an Acorn Atom - the first micro from Acorn in 1980.

It emulates a 12+16k Atom with colour board, disc pack, and optional BBC BASIC mode.

Atomulator is licensed under the GPL, see COPYING for more details.


Usage
~~~~~

Just run Atom.exe. The Atom isn't a very friendly machine, so some basic tips :


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
	New disc 0/2   	       - creates a new DFS/ADFS disc and loads it into drives 0 and 2.
	New disc 1/3           - creates a new DFS/ADFS disc and loads it into drives 1 and 3.
	Write protect disc 0/2 - toggles write protection on drives 0 and 2.
	Write protect disc 1/3 - toggles write protection on drives 1 and 3.
	Default write protect  - determines whether loaded discs are write protected by default

Settings ->
	Video ->    Snow              - emulate authentic Atom snow
		    Fullscreen	      - switches to full screen mode. Use ALT-ENTER to leave
        Hardware -> Colour board      - emulates an Atom colour board. This allows colour in several 
                                        video modes, and slows video refresh from 60hz to 50hz
                    BBC BASIC         - emulates a BBC BASIC language board. This allows a superior
                                        BASIC, but the disc drive is not functional in BBC BASIC mode
					(this seems to be the case with the real machine also)
	Sound ->    Atom sound        - enables the sound output from the Atom
                    Tape sound        - enables the sound from tape
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