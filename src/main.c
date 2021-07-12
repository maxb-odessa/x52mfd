
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#include "x52mfd.h"


// child prog pid
static pid_t child_pid;

// therads context
static ctx_t ctx;

// sigchild handler
static void sigchild(int sig) {
    pid_t wp;
    int ws;

    do {
        wp = waitpid(child_pid, &ws, WUNTRACED | WCONTINUED);
        if (wp < 0) {
            perror("waitpid()");
            exit(1);
        }
        if (WIFEXITED(ws))
            plog("child %d exited, status=%d\n", child_pid, WEXITSTATUS(ws));
        else if (WIFSIGNALED(ws))
            plog("child %d killed by signal %d\n", child_pid, WTERMSIG(ws));
    } while (!WIFEXITED(ws) && !WIFSIGNALED(ws));

    child_pid = 0;

    // inform threads to finish (in case of unexpected child death)
    pthread_cond_signal(&ctx.connected_condvar);
    ctx.done = 1;

    return;
}

// main signal handler
static void sigterm(int sig) {

    plog("got signal %d", sig);

    // inform threads to finish (cleanups() will handle running child)
    pthread_cond_signal(&ctx.connected_condvar);
    ctx.done = 1;
}

// cleanup on exit, termination or return
// TODO case if child ignored TERM - send KILL?
static void cleanups(void) {

    // kill child and wait for it to die
    if (child_pid > 0) {
        plog("terminating child %d\n", child_pid);
        if (! kill(child_pid, SIGTERM))
            pause();
        else
            perror("kill()");
    }

    plog("exiting\n");
}

// show help
static void show_help(char *me) {
    char *me2 = strrchr(me, '/');
    if (! me2)
        me2 = me;
    else
        me2 ++;
    printf("This is '" PACKAGE_STRING "', send bugs and other to <" PACKAGE_BUGREPORT ">\n"
            "Usage:\n%s <progname> [progargs]\n", me2);
}

// the main part, obviousely
int main(int argc, char *argv[], char *envp[]){
    int fds[2];
    pthread_t threads[3]; // connector, reader, writer

    // 'parse' cmdline :)
    if (argc < 2) {
        show_help(argv[0]);
        return 1;
    } else
        argv ++;

    // start external proggie
    if (prg_sopen(argv, envp, fds, &child_pid))
        return 1;

    // setup signals handlers
    signal(SIGCHLD, sigchild);
    signal(SIGTERM, sigterm);
    signal(SIGINT, sigterm);
    signal(SIGQUIT, sigterm);

    // ignore some other signals
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);

    // setup clean exit
    atexit(cleanups);

    // init threads context
    ctx.infd = fds[0];
    ctx.outfd = fds[1];
    ctx.connected = 0;
    ctx.done = 0;
    pthread_cond_init(&ctx.connected_condvar, NULL);
    pthread_mutex_init(&ctx.mutex, NULL);

    // start threads for connector, reader and writer
    pthread_create(&threads[0], NULL, joy_connector, (void *)&ctx);
    pthread_create(&threads[1], NULL, prg_reader, (void *)&ctx);
    pthread_create(&threads[2], NULL, prg_writer, (void *)&ctx);

    // wait for all threads to finish (ignore their retval atm)
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pthread_join(threads[2], NULL);

    // we're done
    return 0;
}


