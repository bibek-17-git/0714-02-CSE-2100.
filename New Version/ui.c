#include <gtk/gtk.h>
#include "../include/ui.h"
#include "../include/backup.h"

static GtkWidget *window;
static GtkWidget *progress_bar;
static GtkWidget *status_label;

/* ================= STATUS ================= */

void update_status(const char *message, double progress)
{
    gtk_label_set_text(GTK_LABEL(status_label), message);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), progress);
}

void show_notification_msg(const char *title, const char *msg)
{
    if (!settings.show_notifications)
        return;

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

/* ================= UI ================= */

void create_main_window(void)
{
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Smart Backup Utility");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *label = gtk_label_new("SMART BACKUP UTILITY");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(box), progress_bar, FALSE, FALSE, 0);

    status_label = gtk_label_new("Ready");
    gtk_box_pack_start(GTK_BOX(box), status_label, FALSE, FALSE, 0);

    GtkWidget *button = gtk_button_new_with_label("Start Backup");
    g_signal_connect(button, "clicked", G_CALLBACK(perform_backup), NULL);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
}
