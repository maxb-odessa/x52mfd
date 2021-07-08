
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include <libx52/libx52.h>

// threads context
typedef struct {
    int             infd;       // prog stdout to us
    int             outfd;      // us to prog stdin
    _Atomic int     connected;  // is joy connected
    _Atomic int     done;       // threads must exit
    libx52_device   *x52dev;    // joy device
    pthread_mutex_t mutex;
} ctx_t;

int prg_sopen(char **args, char **env, int fds[2], pid_t *pid);
void *joy_connector(void *arg);
void *prg_reader(void *arg);
void *prg_writer(void *arg);

// loops, select() and other things delay (in mic secs)
#define LOOP_DELAY_US (100 * 1000)


