
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "x52mfd.h"

void print_help(void) {
    puts(
            "Options are:\n"
            "   -h          show this help and exit\n"
            "   -d          fall in background\n"
            "   -p <path>   search 'path' for loadable modules\n"
            "   -m <name>   load and use module 'name.so' from dir 'path'\n"
            "Note: '-p' option can also be set by env var 'X52MFD_MODULES_DIR'"
        );
}

void fatality(int really, char *reason) {
    if (really)
        error(1, errno, reason);
}

int main(int argc, char *argv[]) {
    char *opts = "hdp:m:";
    int opt;
    char *modules_dir = getenv("X52MFD_MODULES_DIR");
    char *module_name = NULL;
    char *module_path;
    int daemonize = 0;
    void *x52mod;
    x52mfd_func_t x52mfd_init;
    x52mfd_func_t x52mfd_run;
    x52mfd_func_t x52mfd_finish;


    while ((opt = getopt(argc, argv, opts)) != -1) {
        switch (opt) {
            case 'd':
                daemonize = 1;
                break;
            case 'p':
                modules_dir = optarg;
                break;
            case 'm':
                module_name = optarg;
                break;
            case 'h':
                print_help();
                return 0;
                break;
            default:
                break;
        }
    }

    if (module_name == NULL) {
        fprintf(stderr, "Error: you must specify module to load\n");
        return 1;
    }


    if (modules_dir == NULL) {
        fprintf(stderr, "Warning: no modules path specified, using current dir\n");
        modules_dir = "./";
    }

    if (daemonize) {
        pid_t pid = fork();
        fatality(pid < 0, "fork() failed");
        if (pid > 0)
            return 0;
        setsid();
    }

    module_path = calloc(1, strlen(modules_dir) + strlen(module_name) + 5); // 5 = "/.so\0"
    fatality(module_path == NULL, "calloc() failed");
    sprintf(module_path, "%s/%s.so", modules_dir, module_name);

    x52mod = dlopen(module_path, RTLD_LAZY|RTLD_GLOBAL);
    fatality(x52mod == NULL, module_path);

    x52mfd_init = dlsym(x52mod, "x52mfd_mod_init");
    x52mfd_run = dlsym(x52mod, "x52mfd_mod_run");
    x52mfd_finish = dlsym(x52mod, "x52mfd_mod_finish");

    if (x52mfd_init == NULL || x52mfd_init())
        fprintf(stderr, "Failed to init module %s\n", module_path);
    else if (x52mfd_run == NULL || x52mfd_run())
        fprintf(stderr, "Failed to run module %s\n", module_path);
    else if (x52mfd_finish == NULL || x52mfd_finish())
        fprintf(stderr, "Failed to finish module %s\n", module_path);

    dlclose(x52mod);

    return 0;
}
