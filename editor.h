#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#define NO_FILE_LOADED (loaded_fn[0] == '\0')

char *PROGNAME;
char *title = "Untitled - Editor";
char *icon  = "gtk-notepad.png";
char *loaded_fn = NULL;
int errno;

// Main window
GtkWidget *window, *vbox, *hbox;
GtkWindow *gwindow;
GtkAccelGroup *accel = NULL;

GdkAtom atom;
GtkClipboard *clipboard;

// Menubar
GtkWidget *menubar, *sep;

// File menu
GtkWidget *filemenu, *file;
GtkWidget *new, *open, *save, *saveas, *quit;

// Edit menu
GtkWidget *editmenu, *edit;
GtkWidget *cut, *copy, *paste, *delete;
GtkWidget *selectall;

// GtkTextView
GtkWidget *textarea;
GtkTextBuffer *buffer;
GtkWidget *scrolledwindow;
gboolean modified = FALSE;
char wrapping = 1; // Wrapping will be set to 0 by default

char gtk_notepad_open_file(const char*);
void gtk_notepad_save();
void gtk_notepad_saveas(void);