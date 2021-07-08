
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

// threads context
typedef struct {
    int             infd;
    int             outfd;
    _Atomic int     done;
} ctx_t;

int prg_sopen(char **args, char **env, int fds[2], pid_t *pid);
void *joy_connector(void *arg);
void *prg_reader(void *arg);
void *prg_writer(void *arg);

