#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <direct.h>

#ifdef _WIN32
    #define stat  _stat
    #define mkdir _mkdir
    #define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#include "ui.h"
#include "settings.h"
#include "utils.h"
#include "backup.h"
#include "fileops.h"

/* ------------------------------------------------------------------ */
/*  Global widget definitions (declared extern in ui.h)                */
/* ------------------------------------------------------------------ */
GtkWidget    *window           = NULL;
GtkWidget    *progress_bar     = NULL;
GtkWidget    *status_label     = NULL;
GtkListStore *items_list       = NULL;
GtkWidget    *treeview         = NULL;
GtkWidget    *dest_entry_global = NULL;
gboolean      backup_running   = FALSE;
guint         timer_id         = 0;

/* ------------------------------------------------------------------ */
/*  Settings dialog                                                     */
/* ------------------------------------------------------------------ */
void on_settings(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Backup Settings",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "_Save",   GTK_RESPONSE_ACCEPT,
        "_Cancel", GTK_RESPONSE_REJECT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid    = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
    gtk_container_add(GTK_CONTAINER(content), grid);

    int row = 0;

    /* Destination */
    GtkWidget *dest_label = gtk_label_new("Backup Destination:");
    gtk_widget_set_halign(dest_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), dest_label, 0, row, 1, 1);

    GtkWidget *dest_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_grid_attach(GTK_GRID(grid), dest_box, 1, row, 2, 1);

    dest_entry_global = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(dest_entry_global), settings.backup_destination);
    gtk_widget_set_hexpand(dest_entry_global, TRUE);
    gtk_box_pack_start(GTK_BOX(dest_box), dest_entry_global, TRUE, TRUE, 0);

    GtkWidget *browse_btn = gtk_button_new_with_label("Browse...");
    g_signal_connect(browse_btn, "clicked", G_CALLBACK(on_select_backup_folder), NULL);
    gtk_box_pack_start(GTK_BOX(dest_box), browse_btn, FALSE, FALSE, 0);
    row++;

    /* Auto backup */
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Auto Backup:"), 0, row, 1, 1);
    GtkWidget *auto_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(auto_switch), settings.auto_backup);
    gtk_grid_attach(GTK_GRID(grid), auto_switch, 1, row, 1, 1);
    row++;

    /* Interval */
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Interval (seconds):"), 0, row, 1, 1);
    GtkWidget *interval_spin = gtk_spin_button_new_with_range(60, 86400, 60);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(interval_spin), settings.backup_interval);
    gtk_grid_attach(GTK_GRID(grid), interval_spin, 1, row, 1, 1);
    row++;

    /* Max copies */
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Max Copies:"), 0, row, 1, 1);
    GtkWidget *copies_spin = gtk_spin_button_new_with_range(1, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(copies_spin), settings.max_copies);
    gtk_grid_attach(GTK_GRID(grid), copies_spin, 1, row, 1, 1);
    row++;

    /* Subfolders */
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Include Subfolders:"), 0, row, 1, 1);
    GtkWidget *sub_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(sub_switch), settings.backup_subfolders);
    gtk_grid_attach(GTK_GRID(grid), sub_switch, 1, row, 1, 1);
    row++;

    /* Hidden files */
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Include Hidden:"), 0, row, 1, 1);
    GtkWidget *hidden_switch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(hidden_switch), settings.include_hidden);
    gtk_grid_attach(GTK_GRID(grid), hidden_switch, 1, row, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        strcpy(settings.backup_destination, gtk_entry_get_text(GTK_ENTRY(dest_entry_global)));
        settings.auto_backup       = gtk_switch_get_active(GTK_SWITCH(auto_switch));
        settings.backup_interval   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(interval_spin));
        settings.max_copies        = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(copies_spin));
        settings.backup_subfolders = gtk_switch_get_active(GTK_SWITCH(sub_switch));
        settings.include_hidden    = gtk_switch_get_active(GTK_SWITCH(hidden_switch));

        save_settings();
        mkdir(settings.backup_destination);

        if (timer_id) { g_source_remove(timer_id); timer_id = 0; }
        if (settings.auto_backup)
            timer_id = g_timeout_add_seconds(settings.backup_interval, auto_backup_timer, NULL);

        update_status("Settings saved", 0.0);
    }

    gtk_widget_destroy(dialog);
}

/* ------------------------------------------------------------------ */
/*  Log viewer dialog                                                   */
/* ------------------------------------------------------------------ */
void on_view_log(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Backup History",
        GTK_WINDOW(window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Close", GTK_RESPONSE_CLOSE,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled), 10);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled), textview);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled, TRUE, TRUE, 0);

    /* Find latest backup log */
    DIR *dir = opendir(settings.backup_destination);
    if (dir) {
        struct dirent *entry;
        char latest_log[MAX_PATH] = "";
        time_t latest_time = 0;

        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, "Backup_", 7) == 0) {
                char log_path[MAX_PATH];
                sprintf(log_path, "%s\\%s\\backup_log.txt",
                        settings.backup_destination, entry->d_name);

                struct stat st;
                if (stat(log_path, &st) == 0 && st.st_mtime > latest_time) {
                    latest_time = st.st_mtime;
                    strcpy(latest_log, log_path);
                }
            }
        }
        closedir(dir);

        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

        if (strlen(latest_log) > 0) {
            FILE *log = fopen(latest_log, "r");
            if (log) {
                GString *content = g_string_new("");
                char line[256];
                while (fgets(line, sizeof(line), log))
                    g_string_append(content, line);
                fclose(log);
                gtk_text_buffer_set_text(buffer, content->str, -1);
                g_string_free(content, TRUE);
            }
        } else {
            gtk_text_buffer_set_text(buffer, "No backup logs found", -1);
        }
    }

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/* ------------------------------------------------------------------ */
/*  Main window                                                         */
/* ------------------------------------------------------------------ */
void create_main_window(void) {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Smart Backup Utility");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    /* Header */
    GtkWidget *header = gtk_label_new("SMART BACKUP UTILITY");
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    /* Progress bar */
    progress_bar = gtk_progress_bar_new();
    gtk_box_pack_start(GTK_BOX(main_box), progress_bar, FALSE, FALSE, 0);

    /* Status label */
    status_label = gtk_label_new("Ready");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(main_box), status_label, FALSE, FALSE, 0);

    /* File list (scrolled tree view) */
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);

    items_list = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    treeview   = gtk_tree_view_new_with_model(GTK_TREE_MODEL(items_list));

    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column;

    column = gtk_tree_view_column_new_with_attributes("File/Folder", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    column = gtk_tree_view_column_new_with_attributes("Size",     renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    column = gtk_tree_view_column_new_with_attributes("Modified", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    gtk_container_add(GTK_CONTAINER(scrolled), treeview);

    /* File action buttons */
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 0);

    GtkWidget *btn;

    btn = gtk_button_new_with_label("Add Files/Folders");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_select_files_folders), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), btn, TRUE, TRUE, 0);

    btn = gtk_button_new_with_label("Add Folder Only");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_select_folder_only), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), btn, TRUE, TRUE, 0);

    btn = gtk_button_new_with_label("Remove Selected");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_remove_items), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), btn, TRUE, TRUE, 0);

    btn = gtk_button_new_with_label("Clear All");
    g_signal_connect(btn, "clicked", G_CALLBACK(on_clear_all), NULL);
    gtk_box_pack_start(GTK_BOX(button_box), btn, TRUE, TRUE, 0);

    /* Main action buttons */
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(action_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(action_box, 10);
    gtk_box_pack_start(GTK_BOX(main_box), action_box, FALSE, FALSE, 0);

    btn = gtk_button_new_with_label("Start Backup");
    gtk_widget_set_size_request(btn, 140, 40);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_backup_now), NULL);
    gtk_box_pack_start(GTK_BOX(action_box), btn, FALSE, FALSE, 0);

    btn = gtk_button_new_with_label("Settings");
    gtk_widget_set_size_request(btn, 140, 40);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_settings), NULL);
    gtk_box_pack_start(GTK_BOX(action_box), btn, FALSE, FALSE, 0);

    btn = gtk_button_new_with_label("View Log");
    gtk_widget_set_size_request(btn, 140, 40);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_view_log), NULL);
    gtk_box_pack_start(GTK_BOX(action_box), btn, FALSE, FALSE, 0);

    gtk_widget_show_all(window);
}
