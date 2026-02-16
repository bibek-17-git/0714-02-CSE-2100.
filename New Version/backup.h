#ifndef BACKUP_H
#define BACKUP_H

#include <gtk/gtk.h>
#include "../include/config.h"

typedef struct {
    char backup_destination[MAX_PATH];
    int auto_backup;
    int backup_interval;
    int max_copies;
    int backup_subfolders;
    int include_hidden;
    int show_notifications;
} BackupSettings;

extern BackupSettings settings;

void load_settings(void);
void save_settings(void);
gboolean perform_backup(gpointer data);

#endif
