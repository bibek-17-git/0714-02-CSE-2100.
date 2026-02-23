#ifndef SETTINGS_H
#define SETTINGS_H

#define CONFIG_FILE "backup_config.txt"
#define LOG_FILE    "backup_log.txt"
#define MAX_PATH    512
#define MAX_ITEMS   1000

typedef struct {
    char backup_destination[MAX_PATH];
    int  auto_backup;
    int  backup_interval;
    int  max_copies;
    int  backup_subfolders;
    int  include_hidden;
    int  show_notifications;
} BackupSettings;

extern BackupSettings settings;

void load_settings(void);
void save_settings(void);

#endif /* SETTINGS_H */
