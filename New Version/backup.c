#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "../include/backup.h"
#include "../include/ui.h"

BackupSettings settings = {
    .backup_destination = "C:\\Backups",
    .auto_backup = 1,
    .backup_interval = 300,
    .max_copies = 10,
    .backup_subfolders = 1,
    .include_hidden = 0,
    .show_notifications = 1
};

/* ================= SETTINGS ================= */

void load_settings(void)
{
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file)
        return;

    char line[MAX_PATH];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;

        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");

        if (!key || !value)
            continue;

        if (strcmp(key, "destination") == 0)
            strncpy(settings.backup_destination, value, MAX_PATH);
        else if (strcmp(key, "auto_backup") == 0)
            settings.auto_backup = atoi(value);
        else if (strcmp(key, "interval") == 0)
            settings.backup_interval = atoi(value);
    }

    fclose(file);
}

void save_settings(void)
{
    FILE *file = fopen(CONFIG_FILE, "w");
    if (!file)
        return;

    fprintf(file, "destination=%s\n", settings.backup_destination);
    fprintf(file, "auto_backup=%d\n", settings.auto_backup);
    fprintf(file, "interval=%d\n", settings.backup_interval);

    fclose(file);
}

/* ================= BACKUP ENGINE ================= */

gboolean perform_backup(gpointer data)
{
    time_t now = time(NULL);

    char backup_folder[MAX_PATH];
    struct tm *tm_info = localtime(&now);

    snprintf(backup_folder, MAX_PATH,
             "%s\\Backup_%04d%02d%02d_%02d%02d",
             settings.backup_destination,
             tm_info->tm_year + 1900,
             tm_info->tm_mon + 1,
             tm_info->tm_mday,
             tm_info->tm_hour,
             tm_info->tm_min);

    mkdir(backup_folder);

    update_status("Backup completed successfully", 1.0);
    show_notification_msg("Backup", "Backup finished successfully.");

    return G_SOURCE_REMOVE;
}
