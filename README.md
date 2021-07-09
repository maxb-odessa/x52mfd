# x52mfd
This is to control your Saitek x52(pro) joystick

Exec a proggie and communicate with it via STDIN/STDOUT
Read commands from proggie STDOUT and execute them.
Write joystick events to proggie STDIN.

Built on libx52 (https://github.com/nirenjan/x52pro-linux.git)

Commands are exactly the same as used by x52cli:
=CUT=
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
=/CUT=

Exceptions:
    - 'raw' command is not supported
    - all fields are mandatory

Addition:
    - proggie must send new command 'update' for prev commands to be applied
    - commands and args (except text) are case insensitive
    - brightness may be specified by value (0-128) or by percents (0-100%)
    - call to 'clock' command will set current time automatically

Examples (bash):
=CUT=
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
=/CUT=

Led IDs:
    - fire
    - a
    - b
    - d
    - e
    - t1
    - t2
    - t3
    - pov
    - clutch
    - throttle

Led States:
    - off
    - on
    - red
    - amber
    - green

