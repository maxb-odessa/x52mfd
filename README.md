# x52mfd

This is to control your Saitek x52(pro) joystick

Exec a proggie and communicate with it via STDIN/STDOUT
Read commands from proggie STDOUT and execute them.
Write joystick events to proggie STDIN.

Built on [libx52](https://github.com/nirenjan/x52pro-linux.git)


## LEDS and MFD commands and such

Commands are exactly the same as used by x52cli:

    Usage: x52cli <command> [arguments]
    
    Commands:
    	led <led-id> <state>
    	bri {mfd | led} <brightness level>
    	mfd <line> <text in quotes>
    	blink { on | off }
    	shift { on | off }
    	clock {local | gmt} {12hr | 24hr} {ddmmyy | mmddyy | yymmdd}
    	offset {2 | 3} <offset from clock 1 in minutes> {12hr | 24hr}
    	time <hour> <minute> {12hr | 24hr}
    	date <dd> <mm> <yy> {ddmmyy | mmddyy | yymmdd}
    	raw <wIndex> <wValue>
    
    WARNING: raw command may damage your device
    

### Exceptions:

    * 'raw' command is not supported
    * all fields are mandatory

### Addition:

    * proggie must send new command 'update' for prev commands to be applied
    * commands and args (except text) are case insensitive
    * brightness may be specified by value (0-128) or by percents (0-100%)
    * call to 'clock' command will set current time automatically

### Examples (bash):

    echo "bri mfd 30%"
    echo "bri led 30%"
    echo update
    echo "clock local 24hr ddmmyy"
    H=$(date +"%H")
    M=$(date +"%M")
    echo "time $H $M 24hr"
    echo update
    echo "mfd 1 \"line one\""
    echo "mfd 2 \"line two\""
    echo "mfd 3 \"line last!\""
    echo update

### Led IDs:

    * fire
    * a
    * b
    * d
    * e
    * t1
    * t2
    * t3
    * pov
    * clutch
    * throttle

### Led States:

    * off
    * on
    * red
    * amber
    * green

## BUTTONS and other

x52mfd will send back to called proggie x52pro joystick buttons states.
Button name will follow 'on' or 'off' to indicate its state except 'modes' which have 3 states: 1,2 and 3
Axis states are not supported 'cause I see no need in them atm.

Button names cound be found in +libx52+ sources plus I made some by myself for HAT states.
### Button names:

    * LIBX52IO_BTN_TRIGGER
    * LIBX52IO_BTN_TRIGGER_2
    * LIBX52IO_BTN_FIRE
    * LIBX52IO_BTN_PINKY
    * LIBX52IO_BTN_A
    * LIBX52IO_BTN_B
    * LIBX52IO_BTN_C
    * LIBX52IO_BTN_D
    * LIBX52IO_BTN_E
    * LIBX52IO_BTN_T1_UP
    * LIBX52IO_BTN_T1_DN
    * LIBX52IO_BTN_T2_UP
    * LIBX52IO_BTN_T2_DN
    * LIBX52IO_BTN_T3_UP
    * LIBX52IO_BTN_T3_DN
    * LIBX52IO_BTN_POV_1_N
    * LIBX52IO_BTN_POV_1_E
    * LIBX52IO_BTN_POV_1_S
    * LIBX52IO_BTN_POV_1_W
    * LIBX52IO_BTN_POV_2_N
    * LIBX52IO_BTN_POV_2_E
    * LIBX52IO_BTN_POV_2_S
    * LIBX52IO_BTN_POV_2_W
    * LIBX52IO_BTN_CLUTCH
    * LIBX52IO_BTN_MOUSE_PRIMARY
    * LIBX52IO_BTN_MOUSE_SECONDARY
    * LIBX52IO_BTN_MOUSE_SCROLL_UP
    * LIBX52IO_BTN_MOUSE_SCROLL_DN
    * LIBX52IO_BTN_FUNCTION
    * LIBX52IO_BTN_START_STOP
    * LIBX52IO_BTN_RESET
    * LIBX52IO_BTN_PG_UP
    * LIBX52IO_BTN_PG_DN
    * LIBX52IO_BTN_UP
    * LIBX52IO_BTN_DN
    * LIBX52IO_BTN_SELECT
    * LIBX52IO_BTN_MODE_1
    * LIBX52IO_BTN_MODE_2
    * LIBX52IO_BTN_MODE_3

### Addition:

There are to 'buttons' which could be send to a proggie: 'CONNECTED' and 'DICONNECTED'
indicating current joystick connection status. During diconnection no buttons will be sent and no
led or mfd actions will be processed.
