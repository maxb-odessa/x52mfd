
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "x52mfd.h"

static int read_cmd(ctx_t *ctx, char *buf);
static int parse_cmd(char *buf, char *cmd[]);
static int execute_cmd(ctx_t *ctx, char *cmd[]);

// let's expect cmd line from prog not longer than this
#define BUFSIZE 128


// reader loop: read commands from prog fd and execute them
// not thread safe but it is in a single thread, right?
void *prg_reader(void *arg) {
    ctx_t *ctx = (ctx_t *)arg;
    char *cmd[1 + 4];  // the longest cmd is 'date' which has 4 args
    char buf[BUFSIZE + 1];
    int rc;

    memset(buf, 0, BUFSIZE + 1);

    while (! ctx->done) {

        // read command and its arg(s) from prg
        rc = read_cmd(ctx, buf);
        if (rc < 0) { // system failure
            ctx->done = 1;
            break;
        } else if (rc == 0) // no data
            continue;

        memset(cmd, 0, sizeof(cmd));

        // parse and execute command
        if (parse_cmd(buf, cmd) == 0)
            execute_cmd(ctx, cmd);

        // cleanups
        for (int i = 0; i < 5; i ++)
            if (cmd[i]) {
                free(cmd[i]);
                cmd[i] = NULL;
            }

    }

    // we're done
    plog("reader: exited\n");
    return NULL;
}



// read command from prog fd
// one command - one line, so we'll accumulate data until '\n' or EOF
int read_cmd(ctx_t *ctx, char *buf) {
    fd_set infds;
    struct timeval tv = {0, LOOP_DELAY_US};
    char *nr;
    int rc;
    size_t buflen = strlen(buf); // calc buf part to first '\0'

    // shift the buf
    // do this before any fd reading to fetch all the lines that are already accumulated in the buffer
    if (buflen > 0) {
        memmove(buf, buf + buflen + 1, BUFSIZE - buflen);
        buflen = strlen(buf);
    }

    // is data available?
    FD_ZERO(&infds);
    FD_SET(ctx->infd, &infds);

    rc = select(ctx->infd + 1, &infds, NULL, NULL, &tv);
    if (rc < 0) {
        perror("select()");
        return -1;
    } 

    // got some data - read it
    // read and append to buffer
    if (rc > 0) {
        rc = read(ctx->infd, buf + buflen, BUFSIZE - buflen);
        if (rc <= 0) {  // 0 indicates the proggie has closed its connection
            perror("read()");
            return -1;
        }
    }

    // search for next '\n' and cut the buf there
    nr = strchr(buf, '\n');
    if (nr) {
        *nr = '\0';
        return 1; // got full cmd
    }

    // cmd string is incomplete yet
    return 0;
}




// parse command
// "this \"is\" a string" will be considered as one arg
int parse_cmd(char *buf, char *cmd[]) {
    char *p1, *p2;
    int cmd_idx = 0;

    p1 = p2 = buf;
    while (*p1) {

        // move p1 to first non-blank
        while (*p1 && isspace(*p1))
            p1 ++;

        // check p1
        if (! *p1)
            break;

        // move p2 to first non-blank after p1
        // but! if p1 points at \" then we must find another \"
        if (*p1 == '\"') {
            p2 = ++ p1; // p1 must skip current \"
            while (*p2 && *p2 != '\"')
                p2 ++;
        } else {
            p2 = p1 + 1; // p1 must stay where it is
            while (*p2 && !isspace(*p2))
                p2 ++;
        }

        // check p2 not needed - could be end-of-string

        // got a word!
        cmd[cmd_idx] = strndup(p1, p2 - p1);
        assert(cmd[cmd_idx]);
        cmd_idx ++;

        // next word awaits
        if (*p2)
            p2 ++;
        p1 = p2;

        // enuff
        if (cmd_idx > 4)
            break;

    }


    // nothing parsed
    if (! cmd[0])
        return 1;

    /*
       for (int i = 0; i < 5; i ++)
       if (cmd[i])
       plog("CMD[%d] = <%s>\n", i, cmd[i]);
       */

    return 0;
}

// exec led command
// led <name> <state>
static int cmd_led(ctx_t *ctx, char *cmd[]) {
    libx52_led_id led;
    libx52_led_state state;
    int rc = 1;

    if (! cmd[1] || ! cmd[2]) {
        plog("reader: command '%s' requires 2 args: led_name and led_state\n", cmd[0]);
        return 1;
    }

    // pick led
    if (! strcasecmp(cmd[1], "fire"))
        led = LIBX52_LED_FIRE;
    else if (! strcasecmp(cmd[1], "a"))
        led = LIBX52_LED_A;
    else if (! strcasecmp(cmd[1], "b"))
        led = LIBX52_LED_B;
    else if (! strcasecmp(cmd[1], "d"))
        led = LIBX52_LED_D;
    else if (! strcasecmp(cmd[1], "e"))
        led = LIBX52_LED_E;
    else if (! strcasecmp(cmd[1], "t1"))
        led = LIBX52_LED_T1;
    else if (! strcasecmp(cmd[1], "t2"))
        led = LIBX52_LED_T2;
    else if (! strcasecmp(cmd[1], "t3"))
        led = LIBX52_LED_T3;
    else if (! strcasecmp(cmd[1], "pov"))
        led = LIBX52_LED_POV;
    else if (! strcasecmp(cmd[1], "clutch"))
        led = LIBX52_LED_CLUTCH;
    else if (! strcasecmp(cmd[1], "throttle"))
        led = LIBX52_LED_THROTTLE;
    else {
        plog("reader: command '%s': unknown led name '%s'\n", cmd[0], cmd[1]);
        return 1;
    }

    // pick led state
    if (! strcasecmp(cmd[2], "off"))
        state = LIBX52_LED_STATE_OFF;
    else if (! strcasecmp(cmd[2], "on"))
        state = LIBX52_LED_STATE_ON;
    else if (! strcasecmp(cmd[2], "red"))
        state = LIBX52_LED_STATE_RED;
    else if (! strcasecmp(cmd[2], "amber"))
        state = LIBX52_LED_STATE_AMBER;
    else if (! strcasecmp(cmd[2], "green"))
        state = LIBX52_LED_STATE_GREEN;
    else {
        plog("reader: command '%s': unknown led stat '%s'\n", cmd[0], cmd[2]);
        return 1;
    }

    // set led state
    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_led_state(ctx->x52dev, led, state);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

#define PRCNT(X, BASE) ((X) * (BASE) / 100)

// exec bri command
// bri <mfd|led> <level>
// level is 0-128 or 0-100%
static int cmd_bri(ctx_t *ctx, char *cmd[]) {
    int rc;
    int mfd;
    long bri;
    char *err = NULL;

    if (! cmd[1] || ! cmd[2]) {
        plog("reader: command '%s' requires 2 args: led|mfd and brightness\n", cmd[0]);
        return 1;
    }

    // pick leds or mfd
    if (! strcasecmp(cmd[1], "mfd"))
        mfd = 1;
    else if (! strcasecmp(cmd[1], "led"))
        mfd = 0;
    else {
        plog("reader: command '%s': unknown led '%s'\n", cmd[0], cmd[1]);
        return 1;
    }

    // pick brightness (can be raw value 0-128 or percents 0-100%)
    if (cmd[2][strlen(cmd[2]) - 1] == '%') {
        cmd[2][strlen(cmd[2]) - 1] = '\0';
        bri = strtol(cmd[2], &err, 10);
        if ((err && *err) || bri < 0 || bri > 100) {
            plog("reader: command '%s': invalid brightness level '%s%%'\n", cmd[0], cmd[2]);
            return 1;
        }
        bri = PRCNT(bri, 128);
    } else {
        bri = strtol(cmd[2], &err, 10);
        if ((err && *err) || bri < 0 || bri > 128) {
            plog("reader: command '%s': invalid brightness level '%s'\n", cmd[0], cmd[2]);
            return 1;
        }
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_brightness(ctx->x52dev, mfd, bri);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec mfd command
// mfd <line> <"text">
static int cmd_mfd(ctx_t *ctx, char *cmd[]) {
    int rc;
    int line;

    if (! cmd[1] || ! cmd[2]) {
        plog("reader: command '%s' requires 2 args: line and \"text\"\n", cmd[0]);
        return 1;
    }

    // pick mfd line
    if (! strcmp(cmd[1], "1"))
        line = 0;
    else if (! strcmp(cmd[1], "2"))
        line  = 1;
    else if (! strcmp(cmd[1], "3"))
        line = 2;
    else {
        plog("reader: command '%s': invalid line '%s', must be 1,2 or 3\n", cmd[0], cmd[1]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_text(ctx->x52dev, line, cmd[2], strlen(cmd[2]));
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec blink command
// blink <on|off>
static int cmd_blink(ctx_t *ctx, char *cmd[]) {
    int rc;
    int blink;

    if (! cmd[1]) {
        plog("reader: command '%s' requires an arg: on|off\n", cmd[0]);
        return 1;
    }

    // pick blink state
    if (! strcasecmp(cmd[1], "on"))
        blink = 1;
    else if (! strcasecmp(cmd[1], "off"))
        blink = 0;
    else {
        plog("reader: command '%s': invalid state '%s', must be on or off\n", cmd[0], cmd[1]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_blink(ctx->x52dev, blink);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec shift command
// shift <on|off>
static int cmd_shift(ctx_t *ctx, char *cmd[]) {
    int rc;
    int shift;

    if (! cmd[1]) {
        plog("reader: command '%s' requires an arg: on|off\n", cmd[0]);
        return 1;
    }

    // pick shift state
    if (! strcasecmp(cmd[1], "on"))
        shift = 1;
    else if (! strcasecmp(cmd[1], "off"))
        shift = 0;
    else {
        plog("reader: command '%s': invalid state '%s', must be on or off\n", cmd[0], cmd[1]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_shift(ctx->x52dev, shift);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec clock command
// clock <local|gmt> <12hr|24hr> <ddmmyy|mmddyy|yymmdd>
static int cmd_clock(ctx_t *ctx, char *cmd[]) {
    int rc;
    int is_gmt;
    libx52_clock_format hr12_24;
    libx52_date_format format;

    if (! cmd[1] || ! cmd[2] || ! cmd[3]) {
        plog("reader: command '%s' requires 3 args: local|gmt 12hr|24hr ddmmyy|mmddyy|yymmdd\n", cmd[0]);
        return 1;
    }

    // pick local or gmt
    if (! strcasecmp(cmd[1], "local"))
        is_gmt = 0;
    else if (! strcasecmp(cmd[1], "gmt"))
        is_gmt = 1;
    else {
        plog("reader: command '%s': invalid tz '%s', must be local or gmt\n", cmd[0], cmd[1]);
        return 1;
    }

    // pick 12 or 24 hr
    if (! strcasecmp(cmd[2], "12hr"))
        hr12_24 = LIBX52_CLOCK_FORMAT_12HR;
    else if (! strcasecmp(cmd[2], "24hr"))
        hr12_24 = LIBX52_CLOCK_FORMAT_24HR;
    else {
        plog("reader: command '%s': invalid 12/24 format '%s', must be 12hr or 24hr\n", cmd[0], cmd[2]);
        return 1;
    }

    // pick format
    if (! strcasecmp(cmd[3], "ddmmyy"))
        format = LIBX52_DATE_FORMAT_DDMMYY;
    else if (! strcasecmp(cmd[3], "mmddyy"))
        format = LIBX52_DATE_FORMAT_MMDDYY;
    else if (! strcasecmp(cmd[3], "yymmdd"))
        format = LIBX52_DATE_FORMAT_YYMMDD;
    else {
        plog("reader: command '%s': invalid date format '%s', must be ddmmyy|mmddyy|yymmdd\n", cmd[0], cmd[3]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_clock(ctx->x52dev, time(NULL), is_gmt);
    rc += libx52_set_clock_format(ctx->x52dev, LIBX52_CLOCK_1, hr12_24);
    rc += libx52_set_date_format(ctx->x52dev, format);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec offset command
// offset <2|3> <offset from clock 1 in minutes> <12hr|24hr>
static int cmd_offset(ctx_t *ctx, char *cmd[]) {
    int rc;
    libx52_clock_id clcid;
    libx52_clock_format hr12_24;
    int offset;
    char *err = NULL;

    if (! cmd[1] || ! cmd[2] || ! cmd[3]) {
        plog("reader: command '%s' requires 3 args: 1|2 offset 12hr|24hr\n", cmd[0]);
        return 1;
    }

    // pick clock id
    if (! strcmp(cmd[1], "2"))
        clcid = LIBX52_CLOCK_2;
    else if (! strcmp(cmd[1], "3"))
        clcid = LIBX52_CLOCK_3;
    else {
        plog("reader: command '%s': invalid clock id '%s', must be 2 or 3\n", cmd[0], cmd[1]);
        return 1;
    }

    // pick clock offset
    offset = strtol(cmd[2], &err, 10);
    if ((err && *err) || offset < -1440 || offset > 1440) {
        plog("reader: command '%s': invalid clock offset '%s', must be in [-1440, 1440]\n", cmd[0], cmd[2]);
        return 1;
    }

    // pick 12 or 24 hr
    if (! strcasecmp(cmd[3], "12hr"))
        hr12_24 = LIBX52_CLOCK_FORMAT_12HR;
    else if (! strcasecmp(cmd[3], "24hr"))
        hr12_24 = LIBX52_CLOCK_FORMAT_24HR;
    else {
        plog("reader: command '%s': invalid 12/24 format '%s', must be 12hr or 24hr\n", cmd[0], cmd[3]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_clock_timezone(ctx->x52dev, clcid, offset);
    rc += libx52_set_clock_format(ctx->x52dev, clcid, hr12_24);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec time command
// time <hour> <minute> <12hr|24hr>
static int cmd_time(ctx_t *ctx, char *cmd[]) {
    int rc;
    int hr, min;
    libx52_clock_format hr12_24;
    char *err = NULL;

    if (! cmd[1] || ! cmd[2] || ! cmd[3]) {
        plog("reader: command '%s' requires 3 args: 1|2 offset 12hr|24hr\n", cmd[0]);
        return 1;
    }

    // pick hour
    hr = strtol(cmd[1], &err, 10);
    if ((err && *err) || hr < 0 || hr > 24) {
        plog("reader: command '%s': invalid hours '%s', must be in [0, 24]\n", cmd[0], cmd[1]);
        return 1;
    }

    // pick minutes
    min = strtol(cmd[2], &err, 10);
    if ((err && *err) || min < 0 || min > 59) {
        plog("reader: command '%s': invalid minutes '%s', must be in [0, 59]\n", cmd[0], cmd[2]);
        return 1;
    }

    // pick 12 or 24 hr
    if (! strcasecmp(cmd[3], "12hr"))
        hr12_24 = LIBX52_CLOCK_FORMAT_12HR; 
    else if (! strcasecmp(cmd[3], "24hr"))
        hr12_24 = LIBX52_CLOCK_FORMAT_24HR;
    else {
        plog("reader: command '%s': invalid 12/24 format '%s', must be 12hr or 24hr\n", cmd[0], cmd[3]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_time(ctx->x52dev, hr, min);
    rc += libx52_set_clock_format(ctx->x52dev, LIBX52_CLOCK_1, hr12_24);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}

// exec date command
// date <dd> <mm> <yy> <ddmmyy | mmddyy | yymmdd>
static int cmd_date(ctx_t *ctx, char *cmd[]) {
    int rc;
    int day, mon, year;
    libx52_date_format format;
    char *err = NULL;

    if (! cmd[1] || ! cmd[2] || ! cmd[3]) {
        plog("reader: command '%s' requires 4 args: day, mon. year and ddmmyy|mmddyy|yymmdd\n", cmd[0]);
        return 1;
    }

    // pick day
    day = strtol(cmd[1], &err, 10);
    if ((err && *err) || day < 0 || day > 31) {
        plog("reader: command '%s': invalid day '%s', must be in [0, 31]\n", cmd[0], cmd[1]);
        return 1;
    }

    // pick mon
    mon = strtol(cmd[2], &err, 10);
    if ((err && *err) || mon < 0 || mon > 12) {
        plog("reader: command '%s': invalid mon '%s', must be in [0, 12]\n", cmd[0], cmd[2]);
        return 1;
    }

    // pick year
    year = strtol(cmd[3], &err, 10);
    if ((err && *err) || year < 0 || year > 99) {
        plog("reader: command '%s': invalid year '%s', must be in [0, 99]\n", cmd[0], cmd[3]);
        return 1;
    }

    // pick format
    if (! strcasecmp(cmd[3], "ddmmyy"))
        format = LIBX52_DATE_FORMAT_DDMMYY;
    else if (! strcasecmp(cmd[3], "mmddyy"))
        format = LIBX52_DATE_FORMAT_MMDDYY;
    else if (! strcasecmp(cmd[3], "yymmdd"))
        format = LIBX52_DATE_FORMAT_YYMMDD;
    else {
        plog("reader: command '%s': invalid date format '%s', must be ddmmyy|mmddyy|yymmdd\n", cmd[0], cmd[3]);
        return 1;
    }

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_set_date(ctx->x52dev, day, mon, year);
    rc += libx52_set_date_format(ctx->x52dev, format);
    pthread_mutex_unlock(&ctx->mutex);

    return 0;
}

// exec update command
static int cmd_update(ctx_t *ctx, char *cmd[]) {
    int rc;

    pthread_mutex_lock(&ctx->mutex);
    rc = libx52_update(ctx->x52dev);
    pthread_mutex_unlock(&ctx->mutex);

    return rc;
}


// select and execute a command
int execute_cmd(ctx_t *ctx, char *cmd[]) {

    // joy is not connected?
    if (! ctx->x52dev_ok) {
        //plog("reader: joystick is not connected\n");
        //prg_writer_aux(ctx, "DISCONNECTED\n"); // moved to writer
        return 1;
    }

    // exec appropriate command
    if (! strcasecmp(cmd[0], "led"))
        return cmd_led(ctx, cmd);
    else if (! strcasecmp(cmd[0], "bri"))
        return cmd_bri(ctx, cmd);
    else if (! strcasecmp(cmd[0], "mfd"))
        return cmd_mfd(ctx, cmd);
    else if (! strcasecmp(cmd[0], "blink"))
        return cmd_blink(ctx, cmd);
    else if (! strcasecmp(cmd[0], "shift"))
        return cmd_shift(ctx, cmd);
    else if (! strcasecmp(cmd[0], "clock"))
        return cmd_clock(ctx, cmd);
    else if (! strcasecmp(cmd[0], "offset"))
        return cmd_offset(ctx, cmd);
    else if (! strcasecmp(cmd[0], "time"))
        return cmd_time(ctx, cmd);
    else if (! strcasecmp(cmd[0], "date"))
        return cmd_date(ctx, cmd);
    else if (! strcasecmp(cmd[0], "update"))
        return cmd_update(ctx, cmd);
    else if (! strcasecmp(cmd[0], "raw")) {
        plog("reader: command 'raw' is not supported\n");
        return 1;
    }

    plog("reader: unknown command '%s ...'\n", cmd[0]);

    return 1;
}


