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
    * LIBX52IO\_BTN\_TRIGGER
    * LIBX52IO\_BTN\_TRIGGER\_2
    * LIBX52IO\_BTN\_FIRE
    * LIBX52IO\_BTN\_PINKY
    * LIBX52IO\_BTN\_A
    * LIBX52IO\_BTN\_B
    * LIBX52IO\_BTN\_C
    * LIBX52IO\_BTN\_D
    * LIBX52IO\_BTN\_E
    * LIBX52IO\_BTN\_T1\_UP
    * LIBX52IO\_BTN\_T1\_DN
    * LIBX52IO\_BTN\_T2\_UP
    * LIBX52IO\_BTN\_T2\_DN
    * LIBX52IO\_BTN\_T3\_UP
    * LIBX52IO\_BTN\_T3\_DN
    * LIBX52IO\_BTN\_POV\_1\_N
    * LIBX52IO\_BTN\_POV\_1\_E
    * LIBX52IO\_BTN\_POV\_1\_S
    * LIBX52IO\_BTN\_POV\_1\_W
    * LIBX52IO\_BTN\_POV\_2\_N
    * LIBX52IO\_BTN\_POV\_2\_E
    * LIBX52IO\_BTN\_POV\_2\_S
    * LIBX52IO\_BTN\_POV\_2\_W
    * LIBX52IO\_BTN\_CLUTCH
    * LIBX52IO\_BTN\_MOUSE\_PRIMARY
    * LIBX52IO\_BTN\_MOUSE\_SECONDARY
    * LIBX52IO\_BTN\_MOUSE\_SCROLL\_UP
    * LIBX52IO\_BTN\_MOUSE\_SCROLL\_DN
    * LIBX52IO\_BTN\_FUNCTION
    * LIBX52IO\_BTN\_START\_STOP
    * LIBX52IO\_BTN\_RESET
    * LIBX52IO\_BTN\_PG\_UP
    * LIBX52IO\_BTN\_PG\_DN
    * LIBX52IO\_BTN\_UP
    * LIBX52IO\_BTN\_DN
    * LIBX52IO\_BTN\_SELECT
    * LIBX52IO\_BTN\_MODE\_1
    * LIBX52IO\_BTN\_MODE\_2
    * LIBX52IO\_BTN\_MODE\_3

### Addition:
There are to 'buttons' which could be send to a proggie: 'CONNECTED' and 'DICONNECTED'
indicating current joystick connection status. During diconnection no buttons will be sent and no
led or mfd actions will be processed.
