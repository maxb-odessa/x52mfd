
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include <libx52/libx52.h>
#include <libx52/libx52io.h>

// threads context for connector, reader and writer
typedef struct {
    libx52_device       *x52dev;    // joy device for writing
    _Atomic int         x52dev_ok;  // is joy dev connected for writing
    libx52io_context    *x52ctx;    // joy ctx for reading
    _Atomic int         x52ctx_ok;  // is joy ctx connected reading
    int                 infd;       // prog stdout to us
    int                 outfd;      // us to prog stdin
    _Atomic int         done;       // threads must exit
    pthread_mutex_t     mutex;      // ctx locker
} ctx_t;

// print log message
void plog(char *fmt, ...);

int prg_sopen(char **args, char **env, int fds[2], pid_t *pid);
void *joy_connector(void *arg);
void *prg_reader(void *arg);
void *prg_writer(void *arg);
int prg_writer_aux(ctx_t *ctx, char *data);

// loops, select() and other polls delay (in mic secs)
#define LOOP_DELAY_US (100 * 1000)

// get int percents from X on BASE
#define PRCNT(X, BASE) ((X) * (BASE) / 100)


