#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "editor.h"

// Widget pentru SearchBar
typedef struct _SearchBar
{
    GtkWidget *search_entry;
    GtkWidget *sbutton;
    GtkWidget *nbutton;
    GtkWidget *qbutton;
    GtkWidget *text_view;
} SearchBar;

// Functie care cauta un cuvant in buffer
void find(GtkTextView *text_view, const gchar *text, GtkTextIter *iter)
{
    // Inceputul si sfarsitul cuvantului
    GtkTextIter mstart, mend;

    gboolean found; // gasit sau nu
    GtkTextBuffer *buffer; // buffer
    GtkTextMark *last_pos; // ultima pozitie

    // Obtinem bufferul
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Caută textul înainte de poziția specificată de iter
    found = gtk_text_iter_forward_search(iter, text, 0, &mstart, &mend, NULL);

    // Daca l-am gasit
    if (found)
    {
        // Selectam cuvantul
        gtk_text_buffer_select_range(buffer, &mstart, &mend);

        // Creeaza o marca numita "last_pos" la sfarsitul intervalului gasit
        last_pos = gtk_text_buffer_create_mark(buffer, "last_pos",
                                               &mend, FALSE);
        // Deruleaza GtkTextView astfel incat marca "last_pos" s fie vizibila pe ecran
        gtk_text_view_scroll_mark_onscreen(text_view, last_pos);
    }
}

// Afiseaza SearchBar
void find_menu_selected(GtkWidget *widget, SearchBar *sbar)
{
    gtk_widget_show(sbar->search_entry);
    gtk_widget_show(sbar->sbutton);
    gtk_widget_show(sbar->nbutton);
    gtk_widget_show(sbar->qbutton);
}

// Ascunde SearchBar
void close_button_clicked (GtkWidget *close_button, SearchBar *sbar)
{
  gtk_widget_hide(sbar->search_entry);
  gtk_widget_hide(sbar->sbutton);
  gtk_widget_hide(sbar->nbutton);
  gtk_widget_hide(sbar->qbutton);
}

// Cauta cuvantul in buffer
void search_button_clicked(GtkWidget *search_button, SearchBar *sbar)
{
    const gchar *text;
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    // Obtine textul din campul de cautare
    text = gtk_entry_get_text(GTK_ENTRY(sbar->search_entry));

    // Obtine bufferul
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sbar->text_view));
    gtk_text_buffer_get_start_iter(buffer, &iter);

    // Cauta cuvantul
    find(GTK_TEXT_VIEW(sbar->text_view), text, &iter);
}

// Cauta urmatorul cuvant
void next_button_clicked(GtkWidget *next_button, SearchBar *sbar)
{
    const gchar *text;
    GtkTextBuffer *buffer;
    GtkTextMark *last_pos;
    GtkTextIter iter;

    // Obtine textul din campul de cautare
    text = gtk_entry_get_text(GTK_ENTRY(sbar->search_entry));

    // Obtine bufferul
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(sbar->text_view));

    // Obtine ultima pozitie
    last_pos = gtk_text_buffer_get_mark(buffer, "last_pos");
    if (last_pos == NULL)
        return;

    // Obtine iter
    gtk_text_buffer_get_iter_at_mark(buffer, &iter, last_pos);

    // Cauta cuvantul
    find(GTK_TEXT_VIEW(sbar->text_view), text, &iter);
}

// Cream iconita aplicatiei
GdkPixbuf *create_pixbuf(const char *filename)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;
    pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (!pixbuf)
    {
        fprintf(stderr, "%s: %s\n",
                PROGNAME,
                error->message);
        g_error_free(error);
    }

    return pixbuf;
}

// Seteaza titlul ferestrei
void gtk_notepad_set_title(const char *filename)
{
    // Copiem numele fisierului
    char *fn = malloc(strlen(filename) + 1);
    strcpy(fn, filename);

    int i = 0;
    int index = 0;

    // Cautam ultimul slash din numele fisierului
    char *slash = strrchr(filename, '/');
    if (slash)
    {
        // Daca exista, atunci calculam indexul
        index = (int)(slash - filename);

        // Stergem tot ce e inainte de slash
        for (i = 0; i <= index; i++, fn++)
            ;
    }

    // Alocam memorie pentru titlu
    char *_title = malloc(strlen(" - Editor") + strlen(fn) + 1);

    // Copiem numele fisierului in titlu
    strcpy(_title, fn);

    // Adaugam " - Editor" la final
    if (slash)
        for (i = 0; i <= index; i++, fn--)
            ;

    strcat(_title, " - Editor");

    // Setam titlul ferestrei
    gtk_window_set_title(gwindow, _title);

    // Eliberam memoria
    free(_title);
    free(fn);
}

// Fereastra de dialog pentru confirmare salvare
int gtk_notepad_ask_save_cancel(void)
{
    GtkWidget *dialog;

    // Creaza fereastra de dialog
    dialog = gtk_message_dialog_new(gwindow,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_QUESTION,
                                    GTK_BUTTONS_YES_NO,
                                    "Fisierul a fost modificat. Doriti sa il salvati din nou?");

    // Adauga butoanele
    gtk_dialog_add_button(GTK_DIALOG(dialog),
                          "Anuleaza", GTK_RESPONSE_CANCEL);

    // Seteaza titlul ferestrei
    gtk_window_set_title(GTK_WINDOW(dialog), "Fisierul a fost modificat");

    // Deschide fereastra de dialog
    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    // In urma raspunsului, inchide fereastra
    gtk_widget_destroy(dialog);

    // Returneaza raspunsul
    return response;
}

// Creeaza un fisier nou
void gtk_notepad_new(void)
{
    free(loaded_fn);
    loaded_fn = malloc(1);
    loaded_fn[0] = '\0';

    // Reseteaza buffer-ul
    gtk_text_buffer_set_text(buffer, "", -1);

    // Seteaza titlul ferestrei
    gtk_window_set_title(gwindow, "Untitled - Editor");
}

// Deschide un fisier
char gtk_notepad_open_file(const char *filename)
{
    // Deschide fisierul
    FILE *fp = fopen(filename, "rb");

    // Daca nu s-a putut deschide, afiseaza eroarea
    if (!fp)
    {
        fprintf(stderr, "%s: fopen(null): %s\n",
                PROGNAME,
                strerror(errno));
        fclose(fp);

        return 0;
    }
    else
    {
        // Variabila pentru citirea fisierului
        char *buf;

        // Mutam cursorul la finalul fisierului
        fseek(fp, 0, SEEK_END);

        // Aflam dimensiunea fisierului
        long fsize = ftell(fp);

        // Mutam cursorul la inceputul fisierului
        fseek(fp, 0, SEEK_SET);

        // Alocam memorie pentru citirea intregului fisier
        buf = malloc(fsize + 1);

        // Citim fisierul
        fread(buf, fsize, 1, fp);

        // Adaugam terminatorul de sir
        buf[fsize] = '\0';

        // Adaugam textul din buffer in widgetul de text
        gtk_text_buffer_set_text(buffer, buf, -1);

        // Eliberam memoria si inchidem fisierul
        free(buf);
        fclose(fp);

        // Setam titlul ferestrei
        gtk_notepad_set_title(filename);

        // Resetam variabila de modificare
        modified = FALSE;
    }

    return 1;
}

// Deschide un fisier
void gtk_notepad_open(void)
{
    // Daca fisierul a fost modificat, intreaba daca vrea sa salveze
    if (modified == TRUE)
    {
        int response = gtk_notepad_ask_save_cancel();
        switch (response)
        {
        case GTK_RESPONSE_YES:
            gtk_notepad_save();
            break;
        case GTK_RESPONSE_NO:
            break;
        case GTK_RESPONSE_DELETE_EVENT:
        case GTK_RESPONSE_CANCEL:
            return;
            break;

        default:
            return;
            break;
        }
    }

    // Creeaza fereastra de dialog
    GtkWidget *dialog;

    // Tipul ferestrei de dialog
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

    int res;

    // Creeaza fereastra de dialog
    dialog = gtk_file_chooser_dialog_new("Deschide un fisier",
                                         gwindow,
                                         action,
                                         "_Deschide",
                                         GTK_RESPONSE_ACCEPT,
                                         "_Anuleaza",
                                         GTK_RESPONSE_CANCEL,
                                         NULL);

    // Deschide fereastra de dialog
    res = gtk_dialog_run(GTK_DIALOG(dialog));

    // Daca a fost apasat butonul de deschidere
    // atunci deschide fisierul
    char *filename;
    if (res == GTK_RESPONSE_ACCEPT)
    {
        // Extrage numele fisierului
        filename = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(dialog));

        // Deschide fisierul
        if (gtk_notepad_open_file(filename))
        {
            free(loaded_fn);
            loaded_fn = malloc(strlen(filename) + 1);
            strcpy(loaded_fn, filename);
        }

        // Elibereaza memoria
        g_free(filename);
    }

    // Inchide fereastra de dialog
    gtk_widget_destroy(dialog);
}

// Salveaza fisierul
char gtk_notepad_save_file(const char *filename)
{
    // Deschide fisierul
    FILE *fp = fopen(filename, "wb");

    // Daca nu s-a putut deschide, afiseaza eroarea
    if (!fp)
    {
        fprintf(stderr, "%s: fopen(null): %s\n",
                PROGNAME,
                strerror(errno));
        fclose(fp);
        return 0;
    }
    else
    {
        GtkTextIter start, end;
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);

        // Extrage textul din buffer
        char *buf = gtk_text_buffer_get_text(buffer,
                                             &start,
                                             &end,
                                             TRUE);

        // Scrie in fisier
        fwrite(buf, strlen(buf), 1, fp);

        // Elibereaza memoria si inchide fisierul
        free(buf);
        fclose(fp);

        // Seteaza titlul ferestrei
        gtk_notepad_set_title(filename);

        // Resetam variabila de modificare
        modified = FALSE;
    }

    return 1;
}

// Functie pentru decuparea textului
void gtk_notepad_cut(void) {
    gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

// Functie pentru copierea textului
void gtk_notepad_copy(void) {
    gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

// Functie pentru lipirea textului
void gtk_notepad_paste(void) {
    gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}

// Functie pentru stergerea textului
void gtk_notepad_delete(void) {
    gtk_text_buffer_delete_selection(buffer, TRUE, TRUE);
}

// Functie pentru selectarea intregului text
void gtk_notepad_select_all(void) {
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);

    gtk_text_buffer_select_range(buffer, &start, &end);
}


// Salveaza fisierul curent
void gtk_notepad_save(void)
{
    // Daca niciun fisier nu a fost incarcat, atunci
    // deschide fereastra de dialog pentru salvare
    if (NO_FILE_LOADED)
        gtk_notepad_saveas();
    else
        gtk_notepad_save_file(loaded_fn);
}

// Salveaza fisierul curent cu alt nume
void gtk_notepad_saveas(void)
{
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    int res;

    // Creeaza fereastra de dialog pentru salvarea fisierului nou
    dialog = gtk_file_chooser_dialog_new("Salveaza fisierul nou",
                                         gwindow,
                                         action,
                                         "_Salveaza",
                                         GTK_RESPONSE_ACCEPT,
                                         "_Anuleaza",
                                         GTK_RESPONSE_CANCEL,
                                         NULL);

    chooser = GTK_FILE_CHOOSER(dialog);

    // Confirmarea suprascrierii fisierului in dialog
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    // Daca a fost incarcat un fisier, atunci seteaza numele
    if (loaded_fn)
        gtk_file_chooser_set_filename(chooser, loaded_fn);

    // Obtine raspunsul din fereastra de dialog
    res = gtk_dialog_run(GTK_DIALOG(dialog));

    char *filename;
    if (res == GTK_RESPONSE_ACCEPT)
    {
        // Extrage numele fisierului
        filename = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(dialog));

        // Salveaza fisierul
        if (gtk_notepad_save_file(filename))
        {
            free(loaded_fn);
            loaded_fn = malloc(strlen(filename) + 1);
            strcpy(loaded_fn, filename);
        }

        // Elibereaza memoria
        g_free(filename);
    }

    // Inchide fereastra de dialog
    gtk_widget_destroy(dialog);
}

// Menubar (bara de sus)
void setup_menubar(void)
{
    // Initializare menubar
    menubar = gtk_menu_bar_new();

    // Initializare meniu
    filemenu = gtk_menu_new();

    // Initializare iteme
    file = gtk_menu_item_new_with_mnemonic("_File");
    new = gtk_menu_item_new_with_mnemonic("_New");
    open = gtk_menu_item_new_with_mnemonic("_Open");
    save = gtk_menu_item_new_with_mnemonic("_Save");
    saveas = gtk_menu_item_new_with_mnemonic("Save _as...");
    quit = gtk_menu_item_new_with_mnemonic("_Quit");

    editmenu = gtk_menu_new();
    edit = gtk_menu_item_new_with_mnemonic("_Edit");
    cut  = gtk_menu_item_new_with_mnemonic("Cu_t");
    copy = gtk_menu_item_new_with_mnemonic("_Copy");
    paste = gtk_menu_item_new_with_mnemonic("_Paste");
    delete = gtk_menu_item_new_with_mnemonic("_Delete");
    selectall = gtk_menu_item_new_with_mnemonic("_Select All");

    // Adaugam itemele in meniul file
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveas);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu),
                          gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), paste);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), delete);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),
                            gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), selectall);
    

    // Adaum meniul file in menubar
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);

    // Adaugam meniul edit in menubar
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);

    // Adaugam menubar in topbar-ul ferestrei
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 3);

    // Keyboard shortcuts

    // New = CTRL + N
    gtk_widget_add_accelerator(new, "activate", accel, GDK_n,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    // Open = CTRL + O
    gtk_widget_add_accelerator(open, "activate", accel, GDK_o,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    // Save = CTRL + S
    gtk_widget_add_accelerator(save, "activate", accel, GDK_s,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);

    // Save as = CTRL + SHIFT + S
    gtk_widget_add_accelerator(saveas, "activate", accel, GDK_s,
                               GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                               GTK_ACCEL_VISIBLE);

    // Quit = CTRL + Q
    gtk_widget_add_accelerator(quit, "activate", accel, GDK_q,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);


    // Edit menu

    // Cut = CTRL + X
    gtk_widget_add_accelerator(cut, "activate", accel, GDK_x,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    
    // Copy = CTRL + C
    gtk_widget_add_accelerator(copy, "activate", accel, GDK_c,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    
    // Paste = CTRL + V
    gtk_widget_add_accelerator(paste, "activate", accel, GDK_v,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    
    // Delete = DEL
    gtk_widget_add_accelerator(delete, "activate", accel, GDK_KEY_Delete,
                               0,
                               GTK_ACCEL_VISIBLE);

    // Select all = CTRL + A
    gtk_widget_add_accelerator(selectall, "activate", accel, GDK_a,
                               GDK_CONTROL_MASK,
                               GTK_ACCEL_VISIBLE);
    
    // Signal handlers
    // Asculta pentru click pe iteme
    // File menu
    g_signal_connect(new, "activate",
                     (GCallback)gtk_notepad_new, NULL);
    g_signal_connect(open, "activate",
                     (GCallback)gtk_notepad_open, NULL);
    g_signal_connect(save, "activate",
                     (GCallback)gtk_notepad_save, NULL);
    g_signal_connect(saveas, "activate",
                     (GCallback)gtk_notepad_saveas, NULL);

    g_signal_connect(quit, "activate",
                     (GCallback)gtk_main_quit, NULL);
    // Edit menu
    g_signal_connect(cut, "activate",
                             (GCallback)
                             gtk_notepad_cut, NULL);
    g_signal_connect(copy, "activate",
                             (GCallback)
                             gtk_notepad_copy, NULL);
    g_signal_connect(paste, "activate",
                             (GCallback)
                             gtk_notepad_paste, NULL);
    g_signal_connect(delete, "activate",
                             (GCallback)
                             gtk_notepad_delete, NULL);

    g_signal_connect(selectall, "activate",
                             (GCallback)
                             gtk_notepad_select_all, NULL);
    
}

// Apelata cand se modifica textul
void gtk_notepad_text_changed(void)
{
    modified = TRUE;
    // gtk_statusbar_update_lncol();
}

// Functie pentru toggle wrapping
// Wrapping se refera la inceperea unui nou rand
// cand se ajunge la capatul ferestrei
void gtk_text_view_toggle_wrapping(void)
{
    GtkWrapMode mode;
    if (wrapping)
        mode = GTK_WRAP_NONE;
    else
        mode = GTK_WRAP_CHAR;

    wrapping = !wrapping;

    // Seteaza modul de wrapping
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textarea),
                                mode);
}

// Initializare textarea
void setup_textarea(void) {
    // Initializare fereastra de scroll
    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    
    // Modifica setarile pentru scroll
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(
                                   scrolledwindow),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    // Initializare textarea
    textarea = gtk_text_view_new();

    // Adaugam textarea in fereastra de scroll
    gtk_container_add(GTK_CONTAINER(scrolledwindow), textarea);

    // Adaugam fereastra de scroll in fereastra principala
    gtk_box_pack_start(GTK_BOX(vbox),
                       scrolledwindow, TRUE, TRUE, 0);

    // Initializare buffer
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textarea));

    // Signal handlers
    // Asculta pentru schimbarea textului
    g_signal_connect(buffer, "changed",
                     (GCallback) gtk_notepad_text_changed, NULL);
    // g_signal_connect(buffer, "mark_set",
                    //  (GCallback) gtk_statusbar_update_lncol, NULL);

    // Wrapping 
    gtk_text_view_toggle_wrapping();
}

int main(int argc, char *argv[])
{
    // Searchbar
    SearchBar sbar;

    // Numele programului
    PROGNAME = argv[0];

    // Numele fisierului incarcat
    loaded_fn = malloc(1);
    loaded_fn[0] = '\0';

    // Daca a fost dat un argument, atunci incarca fisierul
    if (argc == 2)
    {
        // Deschide fisierul
        FILE *temp = fopen(argv[1], "rb");

        // Daca nu s-a putut deschide, afiseaza eroarea
        if (!temp)
        {
            fprintf(stderr, "%s: failed to open file: %s\n",
                    PROGNAME,
                    strerror(errno));
            return EXIT_FAILURE;
        }

        // Inchide fisierul
        fclose(temp);

        // Elibereaza memoria
        free(loaded_fn);

        // Copiaza numele fisierului
        loaded_fn = malloc(strlen(argv[1]));
        strcpy(loaded_fn, argv[1]);
    }
    else if (argc > 2)
    {
        return EXIT_FAILURE;
    }

    // Initializare GTK
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

    // Afisam iconita aplicatiei
    gtk_window_set_icon(gwindow, create_pixbuf("./icon.png"));

    vbox = gtk_vbox_new(FALSE, 0);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // Folosit pentru keyboard shortcuts
    accel = gtk_accel_group_new();
    gtk_window_add_accel_group(gwindow, accel);

    // Initializare menubar
    setup_menubar();

    // Initializare textarea
    setup_textarea();


    // SEARCHBAR

    // Initializare meniu de optiuni
    GtkWidget *optionsMenu = gtk_menu_new();
    GtkWidget *optionsMi = gtk_menu_item_new_with_mnemonic("_Options");

    // Adauga optiunea de find
    GtkWidget *findMi = gtk_menu_item_new_with_label("Find");

    // Linker pentru find
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(optionsMi), optionsMenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsMenu), findMi);

    // Keyboard shortcut pentru find
    gtk_widget_add_accelerator(findMi, "activate", accel,
                               GDK_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    // Atasare optionsMenu la menubar
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), optionsMi);

    // Setup search bar
    sbar.text_view = textarea;
    sbar.search_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), sbar.search_entry, TRUE, TRUE, 0);

    // Listener pentru butonul de search
    sbar.sbutton = gtk_button_new_with_label("Search");
    gtk_box_pack_start(GTK_BOX(hbox), sbar.sbutton, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(sbar.sbutton), "clicked",
                     G_CALLBACK(search_button_clicked), &sbar);

    // Listener pentru butonul de next
    sbar.nbutton = gtk_button_new_with_label("Next");
    gtk_box_pack_start(GTK_BOX(hbox), sbar.nbutton, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(sbar.nbutton), "clicked",
                     G_CALLBACK(next_button_clicked), &sbar);

    // Listener pentru butonul de close
    sbar.qbutton = gtk_button_new_with_label("Close");
    gtk_box_pack_start(GTK_BOX(hbox), sbar.qbutton, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(sbar.qbutton), "clicked",
                     G_CALLBACK(close_button_clicked), &sbar);

    // Listener pentru meniul de find
    g_signal_connect(G_OBJECT(findMi), "activate",
                     G_CALLBACK(find_menu_selected), &sbar);
    
    if (argc == 2)
        gtk_notepad_open_file(loaded_fn);

    // Signal handler pentru inchiderea ferestrei
    g_signal_connect(window, "destroy",
                     (GCallback)gtk_main_quit, NULL);

    // Afisam toate widget-urile din fereastra
    gtk_widget_show_all(window);

    // Ascunde searchbar-ul la inceput
    gtk_widget_hide(sbar.search_entry);
    gtk_widget_hide(sbar.sbutton);
    gtk_widget_hide(sbar.nbutton);
    gtk_widget_hide(sbar.qbutton);

    // Incepem bucla principala de evenimente
    gtk_main();

    return EXIT_SUCCESS;
}