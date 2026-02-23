#include <gtk/gtk.h>
#include "utils.h"
#include "ui.h"
#include "settings.h"

void update_status(const char *message, double progress) {
    gtk_label_set_text(GTK_LABEL(status_label), message);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), progress);

    /* Flush GTK event queue so the UI updates immediately */
    while (gtk_events_pending()) gtk_main_iteration();
}

void show_notification_msg(const char *title, const char *msg) {
    if (!settings.show_notifications) return;

    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", msg);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
