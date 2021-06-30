
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>

#include "x52mfd.h"

// open r/w pipe (2 pipes actually) to exec'd process
int sopen(const char *program, int fds[2], pid_t *pid) {

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0)
        return -1;

    *pid = fork();

    if (*pid < 0) {
        // error
        close(fds[0]);
        close(fds[1]);
        return -1;
    } else if (*pid == 0) {
        // child
        dup2(fds[0], 0);
        dup2(fds[1], 1);
        close(fds[1]);
        close(fds[0]);
        execl("/bin/sh", "sh", "-c", program, NULL);
        _exit(127);
    }

    // parent
    return 0;
}

