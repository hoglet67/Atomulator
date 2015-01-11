/*
	resources.h for Atomulator.
*/
#include "sidtypes.h"

#define IDM_FILE_RESET      40000
#define IDM_FILE_EXIT       40001
#define IDM_TAPE_LOAD       40010
#define IDM_TAPE_EJECT      40011
#define IDM_TAPE_REW        40012
#define IDM_TAPE_CAT        40013
#define IDM_TAPES_NORMAL    40015
#define IDM_TAPES_FAST      40016
#define IDM_DISC_LOAD_0     40020
#define IDM_DISC_LOAD_1     40021
#define IDM_DISC_EJECT_0    40022
#define IDM_DISC_EJECT_1    40023
#define IDM_DISC_NEW_0      40024
#define IDM_DISC_NEW_1      40025
#define IDM_DISC_WPROT_0    40026
#define IDM_DISC_WPROT_1    40027
#define IDM_DISC_WPROT_D    40028
#define IDM_DISC_AUTOBOOT   40029
#define IDM_HARD_COLOUR     40030
#define IDM_HARD_BBC        40031
#define IDM_SOUND_ATOM      40040
#define IDM_SOUND_TAPE      40041
#define IDM_SOUND_DDNOISE   40042
#define IDM_VID_SNOW        40050
#define IDM_VID_FULLSCREEN  40051
#define IDM_KEY_REDEFINE    40060
#define IDM_KEY_DEFAULT     40061

// SP3 JOYSTICK SUPPORT

#define IDM_JOYSTICK        40062 // Joystick support

// END SP3

#define IDM_DDV_33          40070
#define IDM_DDV_66          40071
#define IDM_DDV_100         40072
#define IDM_DDT_525         40075
#define IDM_DDT_35          40076
#define IDM_MISC_DEBUG      40080
#define IDM_MISC_DEBONBRK	40081
#define IDM_MISC_BREAK      40082
#define IDM_MISC_SCRSHOT    40083

#define IDM_RAMROM_ENABLE   40084
#define IDM_RAMROM_DISKROM	40085

#define IDM_SOUND_ATOMSID   42000
#define IDM_WAVE_SID        42001
#define IDM_SID_INTERP      42002
#define IDM_SID_RESAMP      42003

#define IDM_SID_TYPE        43000

#define IDM_SPD_10          41080
#define IDM_SPD_25          41081
#define IDM_SPD_50          41082
#define IDM_SPD_75          41083
#define IDM_SPD_100         41084
#define IDM_SPD_150         41085
#define IDM_SPD_170         41086
#define IDM_SPD_200         41087
#define IDM_SPD_300         41088
#define IDM_SPD_400         41089
#define IDM_SPD_500         41090

#define Button1 			1000
#define ListBox1 			40900

#define IDM_RAMROM_FAST		IDM_SPD_170