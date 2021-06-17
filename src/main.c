/*
   This is a simple proggie to just open shared object (game related module) 
   and execute 3 of its functions in order:
   x52mfd_mod_init()
   x52mfd_mod_run()
   x52mfd_mod_finish()

   The module will be searched in specified dir (by -p flag) or by X52MFD_MODULES_DIR env var.

   Optionally the proggie may fall into background.
   */

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


// just a help printer
void print_help(void) {
    puts(
            "Options are:\n"
            "   -h          show this help and exit\n"
            "   -d          fall in background\n"
            "   -p <path>   search 'path' for loadable modules\n"
            "   -m <name>   load and use module 'name.so' from dir 'path'\n"
            "Notes:\n"
            "   '-p' option can also be set by env var 'X52MFD_MODULES_DIR'"
            "   '-m' option can also be set by env var 'X52MFD_MODULE'"
        );
}

// fatal errors catcher and reporter
void fatality(int really, char *reason) {
    if (really)
        error(1, errno, reason);
}


// the main part
int main(int argc, char *argv[]) {
    char *opts = "hdp:m:";
    int opt;
    char *modules_dir = getenv("X52MFD_MODULES_DIR");
    char *module_name = getenv("X52MFD_MODULE");
    char *module_path;
    int daemonize = 0;
    void *x52mod;
    x52mfd_func_t mod_init,mod_run, mod_finish;


    // parse cmdline options
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

    // module name is mandatory
    if (module_name == NULL) {
        fprintf(stderr, "Error: you must specify module to load\n");
        return 1;
    }


    // modules dir is optional
    if (modules_dir == NULL)
        modules_dir = X52MFD_MODULES_DIR;

    // fall into background if requested
    if (daemonize) {
        pid_t pid = fork();
        fatality(pid < 0, "fork() failed");
        if (pid > 0)
            return 0;
        close(0);
        close(1);
        close(2);
        setsid();
    }

    // compose full path to loadable module
    module_path = calloc(1, strlen(modules_dir) + strlen(module_name) + 5); // 5 = "/.so\0"
    fatality(module_path == NULL, "calloc() failed");
    sprintf(module_path, "%s/%s.so", modules_dir, module_name);

    // try to open module
    x52mod = dlopen(module_path, RTLD_LAZY|RTLD_GLOBAL);
    fatality(x52mod == NULL, module_path);

    // search for mandatory module functions
    mod_init = dlsym(x52mod, XSTR(X52MFD_INIT_FUNC));
    mod_run = dlsym(x52mod, XSTR(X52MFD_RUN_FUNC));
    mod_finish = dlsym(x52mod, XSTR(X52MFD_FINISH_FUNC));

    // execute module functions in order
    const char *errstr = NULL;
    if (mod_init == NULL || mod_init(&errstr))
        fprintf(stderr, "Failed to init module %s: %s\n", module_path, errstr);
    else if (mod_run == NULL || mod_run(&errstr))
        fprintf(stderr, "Failed to run module %s: %s\n", module_path, errstr);
    else if (mod_finish == NULL || mod_finish(&errstr))
        fprintf(stderr, "Failed to finish module %s: %s\n", module_path, errstr);

    // we're done
    dlclose(x52mod);

    return 0;
}
