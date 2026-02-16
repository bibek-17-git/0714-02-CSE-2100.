#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>

void create_main_window(void);
void update_status(const char *message, double progress);
void show_notification_msg(const char *title, const char *msg);

#endif
