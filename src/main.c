
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


#include "x52mfd.h"

int main(void){


    int fds[2];
    pid_t pid;
    const char *prog = "./prog.sh";
    char buf1[32] = {0,};
    char buf2[32] = {0,};

    if (sopen(prog, fds, &pid)) {
        printf("sopen\n");
        return 1;
    }

    if (read(fds[0], buf1, 30) < 0) {
        printf("no read\n");
    } else {
        printf("read: %s\n", buf1);
    }

    if (write(fds[1], "123", 3) < 0) {
        printf("no write\n");
    }

    if (read(fds[0], buf2, 30) < 0) {
        printf("no read again\n");
    } else {
        printf("read2: %s\n", buf2);
    }

    return 0;
}
