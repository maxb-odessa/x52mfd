
#include "elda.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>


// just show help
void show_help(char *me) {
    char *me2 = strrchr(me, '/');
    if (! me2)
        me2 = me;
    else
        me2 ++;
    printf( "This is '" PACKAGE_STRING "', Elite Dangerous support for x52mfd\n"
#ifdef WITH_XDO
            "(built with XDO support)\n"
#else
            "(built without XDO support)\n"
#endif
            "\nUsage:\n"
            "   %s [-d|-h|-c conf]\n"
            "Where:\n"
            "   -d          debug mode: useful to test config on journal files\n"
            "   -h          show this help\n"
            "   -c conf     use 'conf' file instead of default one\n"
            "\nSend bugs and other stuff to <" PACKAGE_BUGREPORT ">\n"
            ,me2);
}

// debug mode
int debug;

// the main part
int main(int argc, char *argv[]){
    char *config_file = DEFAULT_CONFIG_FILE;
    int opt;

    // parse cmdline args
    while ((opt = getopt(argc, argv, "hdc:")) != -1) {
        switch (opt) {
            case 'd':
                debug = 1;
                break;
            case 'c':
                config_file = optarg;
                break;
            case'h':
                show_help(argv[0]);
                return 0;
            default:
                fprintf(stderr, "Unknown argument.\n");
                show_help(argv[0]);
                return 1;
        }

    }

    // read and parse config
    if (conf_read_file(config_file) == false) {
        plog("failed to read config '%s'\n", config_file);
        return 2;
    }

    // start events loop
    if (events_loop() == false) {
        plog("events loop failed\n");
        return 3;
    }

    // done
    return 0;
}

