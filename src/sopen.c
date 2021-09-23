
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "x52mfd.h"

// create 2 pipes, dup() them to stdin/stdout and fork a child
// return !0 and set errno on failure
int prg_sopen(char **args, char **env, int fds[2], pid_t *pid) {
    int rc;
    int fdr[2], fdw[2];

    // create pipes
    if (pipe(fdr) < 0) {
        perror("pipe()");
        return -1;
    }

    if (pipe(fdw) < 0) {
        perror("pipe()");
        return -1;
    }

    // save our side pipe descriptors
    fds[0] = fdr[0];
    fds[1] = fdw[1];

    *pid = fork();

    if (*pid < 0) {
        // error
        perror("fork()");
        close(fdr[0]);
        close(fdr[1]);
        close(fdw[0]);
        close(fdw[1]);
        return -1;
    } else if (*pid == 0) {
        // child
        dup2(fdw[0], 0);
        close(fdw[0]);
        close(fdr[0]);
        dup2(fdr[1], 1);
        close(fdr[1]);
        close(fdw[1]);
        rc = execvpe(args[0], args, env);
        plog("sopen: exec of '%s ...' failed: %s\n", args[0], strerror(errno));
        _exit(rc);
    }

    // parent
    return 0;
}

