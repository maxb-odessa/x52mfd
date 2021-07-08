
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "x52mfd.h"

// open r/w pipe (2 pipes actually) to exec'd process
// return !0 and set errno on failure
int prg_sopen(char **args, char **env, int fds[2], pid_t *pid) {
    int rc;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        perror("socketpair()");
        return -1;
    }

    *pid = fork();

    if (*pid < 0) {
        // error
        perror("fork()");
        close(fds[0]);
        close(fds[1]);
        return -1;
    } else if (*pid == 0) {
        // child
        dup2(fds[0], 0);
        dup2(fds[1], 1);
        close(fds[1]);
        close(fds[0]);
        rc = execvpe(args[0], args, env);
        perror(args[0]);
        _exit(rc);
    }

    // parent
    return 0;
}

