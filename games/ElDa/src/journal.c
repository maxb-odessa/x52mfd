/*
   cmdr journal events handler
   */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fnmatch.h>

#include "ed-odyssey.h"

// journals look like this: Journal.210613182542.01.log
#define JOURNAL_PATTERN "Journal.[0-9]*.[0-9]*.log"

static char journal_dir[PATH_MAX + 1];
static char journal_file[PATH_MAX + 1];
static FILE *journal_fp;
static long journal_fp_pos;
static char journal_event[0xFFFF + 1];


// init
int journal_init(char *dirname) {
    strncpy(journal_dir, dirname, sizeof(journal_dir) - 1);
    return 0;
}


static int filter_journal_name(const struct dirent *de) {
    return !fnmatch(JOURNAL_PATTERN, de->d_name, FNM_PATHNAME);
}

// find latest journal and open it
// the file may not exists yet, so it's ok - wi'll keep on trying
static int reopen_journal() {
    struct dirent **journals;
    int n, rc = 0;
    char journal_full_path[PATH_MAX * 2 + 2];

    // find latest journal file
    n = scandir(journal_dir, &journals, filter_journal_name, alphasort);
    if (n < 0) {
        fprintf(stderr, "Failed to scan journals dir '%s': %s\n", journal_dir, strerror(errno));
        return 1;
    } else if (n == 0) {
        rc = 0;
        goto cleanup;
    }

    // journal is fresh - update its position
    if (!strcmp(journals[n - 1]->d_name, journal_file)) {
        if (journal_fp) {
            journal_fp = freopen(NULL, "r", journal_fp);
            fseek(journal_fp, journal_fp_pos, SEEK_SET);
        }
        goto cleanup;
    }

    // got latest journal file name
    strncpy(journal_file, journals[n - 1]->d_name, sizeof(journal_file) - 1);
    sprintf(journal_full_path, "%s/%s", journal_dir, journal_file);

    // reopen journal file
    if (journal_fp)
        fclose(journal_fp);

    journal_fp = fopen(journal_full_path, "r");
    if (! journal_fp) {
        fprintf(stderr, "Failed to open journal file '%s': %s\n", journal_full_path, strerror(errno));
        rc = 1;
        goto cleanup;
    }
    //puts(journal_full_path);
    fseek(journal_fp, 0L, SEEK_END);
    journal_fp_pos = ftell(journal_fp);

cleanup:
    while (n--)
        free(journals[n]);
    free(journals);

    return rc;
}

// get last event from the journal
char *journal_get_event(void) {

    // keep journal file descriptor always fresh
    if (reopen_journal())
        return NULL;

    if (fgets(journal_event, sizeof(journal_event) - 1, journal_fp)) {
        journal_fp_pos = ftell(journal_fp);
        return journal_event;
    }

    return NULL;
}


