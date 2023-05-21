#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "editor.h"


int main(int argc, char* argv[]) {
    // Numele programului
    PROGNAME = argv[0];

    gtk_init(&argc, &argv);

    // Titlul ferestrei curente
    char title[10] = "Editor";

    // Extrage clipboard-ul principal
    atom = gdk_atom_intern("CLIPBOARD", TRUE);
    clipboard = gtk_clipboard_get(atom);

    // Creeaza fereastra
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gwindow = GTK_WINDOW(window);

    // Seteaza titlul ferestrei
    gtk_window_set_title(gwindow, title);

    // Seteaza dimensiunea ferestrei
    gtk_window_set_default_size(gwindow, 1280, 720);

    // Muta fereastra pe centru
    gtk_window_set_position(gwindow, GTK_WIN_POS_CENTER);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Folosit pentru keyboard shortcuts
    accel = gtk_accel_group_new();
    gtk_window_add_accel_group(gwindow, accel);

    // Signal handler pentru inchiderea ferestrei
    g_signal_connect(window, "destroy",
                     (GCallback) gtk_main_quit, NULL);

    // Afisam toate widget-urile din fereastra
    gtk_widget_show_all(window);

    // Incepem bucla principala de evenimente
    gtk_main();

    return EXIT_SUCCESS;
}
