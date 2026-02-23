#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <direct.h>

#ifdef _WIN32
    #define stat  _stat
    #define mkdir _mkdir
    #define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#include "settings.h"

/* Default settings */
BackupSettings settings = {
    .backup_destination = "C:\\Backups",
    .auto_backup        = 1,
    .backup_interval    = 300,
    .max_copies         = 10,
    .backup_subfolders  = 1,
    .include_hidden     = 0,
    .show_notifications = 1
};

void save_settings(void) {
    FILE *file = fopen(CONFIG_FILE, "w");
    if (!file) return;

    fprintf(file, "destination=%s\n", settings.backup_destination);
    fprintf(file, "auto_backup=%d\n", settings.auto_backup);
    fprintf(file, "interval=%d\n",    settings.backup_interval);
    fprintf(file, "max_copies=%d\n",  settings.max_copies);
    fprintf(file, "subfolders=%d\n",  settings.backup_subfolders);
    fprintf(file, "hidden=%d\n",      settings.include_hidden);

    fclose(file);
}

void load_settings(void) {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        save_settings();   /* write defaults on first run */
        return;
    }

    char line[MAX_PATH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        char *key   = strtok(line, "=");
        char *value = strtok(NULL, "=");
        if (!key || !value) continue;

        if      (strcmp(key, "destination") == 0) strcpy(settings.backup_destination, value);
        else if (strcmp(key, "auto_backup") == 0) settings.auto_backup       = atoi(value);
        else if (strcmp(key, "interval")    == 0) settings.backup_interval   = atoi(value);
        else if (strcmp(key, "max_copies")  == 0) settings.max_copies        = atoi(value);
        else if (strcmp(key, "subfolders")  == 0) settings.backup_subfolders = atoi(value);
        else if (strcmp(key, "hidden")      == 0) settings.include_hidden    = atoi(value);
    }
    fclose(file);

    mkdir(settings.backup_destination);
}
