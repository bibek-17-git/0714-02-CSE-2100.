#include <gtk/gtk.h>
#include "../include/ui.h"
#include "../include/backup.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    load_settings();
    create_main_window();

    gtk_main();
    return 0;
}
