
#include "elda.h"

#include <stdio.h>
#include <string.h>

// just show help
void show_help(char *me) {
    char *me2 = strrchr(me, '/');
    if (! me2)
        me2 = me;
    else
        me2 ++;
    printf("This is '" PACKAGE_STRING "', send bugs and other to <" PACKAGE_BUGREPORT ">\n"
            "Usage:\n\t%s [config_file]\n", me2);
}



// the main part
int main(int argc, char *argv[]){
    char *config_file = DEFAULT_CONFIG_FILE;

    // get config file name from cmdline
    if (argc >= 2) {
        if (argc > 2 || argv[1][0] == '-') {
            show_help(argv[0]);
            return 1;
        }
        config_file = argv[1];
    }

    // read and parse config
    if (conf_read_file(config_file) == false) {
        plog("failed to read config '%s'\n", config_file);
        return 2;
    }



    return 0;
}

