#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>

/* Global GTK widgets shared across modules */
extern GtkWidget    *window;
extern GtkWidget    *progress_bar;
extern GtkWidget    *status_label;
extern GtkListStore *items_list;
extern GtkWidget    *treeview;
extern GtkWidget    *dest_entry_global;
extern gboolean      backup_running;
extern guint         timer_id;

void create_main_window(void);

#endif /* UI_H */
