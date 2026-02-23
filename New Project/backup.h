#ifndef BACKUP_H
#define BACKUP_H

#include <glib.h>

gboolean perform_backup(gpointer data);
gboolean auto_backup_timer(gpointer data);

void on_backup_now(GtkWidget *widget, gpointer data);

#endif /* BACKUP_H */
