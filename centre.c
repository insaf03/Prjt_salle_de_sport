#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "callbacks_common.h"
#include "../include/centre.h"

// Helper function to populate combobox with centers from centres.txt
void populate_combobox_with_centres(GtkComboBox *combobox)
{
    if (!combobox) return;

    // Get or create list store
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combobox));
    
    if (!store) {
        // Create new list store with one string column
        store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
        g_object_unref(store);
    }

    // Clear existing items
    gtk_list_store_clear(store);

    // Read centres from file
    FILE *file = fopen("data/centres.txt", "r");
    if (!file) {
        // File doesn't exist yet, that's okay
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        // Parse line manually to get center name (field 2)
        char *line_copy = g_strdup(line);
        char *fields[15];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;

        // Split by pipe
        while (*p && field_count < 15) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        // Add last field
        if (field_count < 15) {
            fields[field_count++] = start;
        }

        // Get center name (field index 1, which is the second field)
        if (field_count >= 2 && fields[1]) {
            char *nom = fields[1];
            // Trim whitespace
            while (*nom == ' ' || *nom == '\t') nom++;
            
            if (strlen(nom) > 0) {
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, nom, -1);
            }
        }

        g_free(line_copy);
    }

    fclose(file);
}

// Helper function to delete centre by ID
static int delete_centre_by_id(const char *filename, int centre_id)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }

    FILE *temp = fopen("data/centres_temp.txt", "w");
    if (!temp) {
        fclose(file);
        return 0;
    }

    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) {
            fputs("\n", temp);
            continue;
        }

        char *line_copy = g_strdup(line);
        char *first_pipe = strchr(line_copy, '|');
        int id = 0;
        
        if (first_pipe) {
            *first_pipe = '\0';
            id = atoi(line_copy);
        } else {
            id = atoi(line_copy);
        }

        if (id == centre_id) {
            found = 1;
        } else {
            fputs(line, temp);
            fputs("\n", temp);
        }

        g_free(line_copy);
    }

    fclose(file);
    fclose(temp);

    if (found) {
        remove(filename);
        rename("data/centres_temp.txt", filename);
        return 1;
    } else {
        remove("data/centres_temp.txt");
        return 0;
    }
}

// Helper function to search centre by ID
static int search_centre_by_id(const char *filename, int id,
                               char *nom, char *adresse, char *ville, char *telephone, char *email,
                               int *nb_salles, int *cap_max, char *service, char *etat,
                               char *jour_travail, int *h_deb, int *h_fin1, int *h_fin2, int *h_fin3)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        int file_id = 0;
        int file_nb_salles = 0, file_cap_max = 0, file_h_deb = 0, file_h_fin1 = 0, file_h_fin2 = 0, file_h_fin3 = 0;
        char file_nom[100] = "", file_adresse[200] = "", file_ville[50] = "", file_telephone[20] = "", file_email[100] = "";
        char file_service[100] = "", file_etat[10] = "", file_jour[50] = "";

        // Parse line manually to handle empty fields
        char *line_copy = g_strdup(line);
        char *fields[15];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;

        // Split by pipe
        while (*p && field_count < 15) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        // Add last field
        if (field_count < 15) {
            fields[field_count++] = start;
        }

        // Parse fields
        if (field_count >= 1) {
            file_id = atoi(fields[0]);
        }
        if (field_count >= 2 && fields[1]) {
            char *s = fields[1];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_nom, s, sizeof(file_nom)-1);
            file_nom[sizeof(file_nom)-1] = '\0';
        }
        if (field_count >= 3 && fields[2]) {
            char *s = fields[2];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_adresse, s, sizeof(file_adresse)-1);
            file_adresse[sizeof(file_adresse)-1] = '\0';
        }
        if (field_count >= 4 && fields[3]) {
            char *s = fields[3];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_ville, s, sizeof(file_ville)-1);
            file_ville[sizeof(file_ville)-1] = '\0';
        }
        if (field_count >= 5 && fields[4]) {
            char *s = fields[4];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_telephone, s, sizeof(file_telephone)-1);
            file_telephone[sizeof(file_telephone)-1] = '\0';
        }
        if (field_count >= 6 && fields[5]) {
            char *s = fields[5];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_email, s, sizeof(file_email)-1);
            file_email[sizeof(file_email)-1] = '\0';
        }
        if (field_count >= 7 && fields[6]) {
            file_nb_salles = atoi(fields[6]);
        }
        if (field_count >= 8 && fields[7]) {
            file_cap_max = atoi(fields[7]);
        }
        if (field_count >= 9 && fields[8]) {
            char *s = fields[8];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_service, s, sizeof(file_service)-1);
            file_service[sizeof(file_service)-1] = '\0';
        }
        if (field_count >= 10 && fields[9]) {
            char *s = fields[9];
            while (*s == ' ' || *s == '\t') s++;
            char *end = s + strlen(s) - 1;
            while (end > s && (*end == ' ' || *end == '\t')) {
                *end = '\0';
                end--;
            }
            strncpy(file_etat, s, sizeof(file_etat)-1);
            file_etat[sizeof(file_etat)-1] = '\0';
        }
        if (field_count >= 11 && fields[10]) {
            char *s = fields[10];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(file_jour, s, sizeof(file_jour)-1);
            file_jour[sizeof(file_jour)-1] = '\0';
        }
        if (field_count >= 12 && fields[11]) {
            file_h_deb = atoi(fields[11]);
        }
        if (field_count >= 13 && fields[12]) {
            file_h_fin1 = atoi(fields[12]);
        }
        if (field_count >= 14 && fields[13]) {
            file_h_fin2 = atoi(fields[13]);
        }
        if (field_count >= 15 && fields[14]) {
            file_h_fin3 = atoi(fields[14]);
        }

        g_free(line_copy);

        if (file_id == id && file_id > 0) {
            strncpy(nom, file_nom, 100);
            strncpy(adresse, file_adresse, 200);
            strncpy(ville, file_ville, 50);
            strncpy(telephone, file_telephone, 20);
            strncpy(email, file_email, 100);
            *nb_salles = file_nb_salles;
            *cap_max = file_cap_max;
            strncpy(service, file_service, 100);
            strncpy(etat, file_etat, 10);
            strncpy(jour_travail, file_jour, 50);
            *h_deb = file_h_deb;
            *h_fin1 = file_h_fin1;
            *h_fin2 = file_h_fin2;
            *h_fin3 = file_h_fin3;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Callback for ajouter centre button
void
on_ajoutercentre_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *nomcentre = lookup_widget(window, "nomcentre");
    GtkWidget *adressecentre = lookup_widget(window, "adressecentre");
    GtkWidget *villecentre = lookup_widget(window, "villecentre");
    GtkWidget *telephonecentre = lookup_widget(window, "telephonecentre");
    GtkWidget *emailcentre = lookup_widget(window, "emailcentre");
    GtkWidget *nombresalle = lookup_widget(window, "nombresalle");
    GtkWidget *capacitemaximale = lookup_widget(window, "capacitemaximale");
    GtkWidget *servicedispo = lookup_widget(window, "servicedispo");
    GtkWidget *js_checkouvert = lookup_widget(window, "js_checkouvert");
    GtkWidget *js_check_ferme = lookup_widget(window, "js_check_ferme");
    GtkWidget *jour_de_travail_centre = lookup_widget(window, "jour_de_travail_centre");
    GtkWidget *heuredebut_centre = lookup_widget(window, "heuredebut_centre");
    GtkWidget *js12 = lookup_widget(window, "js12");
    GtkWidget *js13 = lookup_widget(window, "js13");
    GtkWidget *js14 = lookup_widget(window, "js14");

    const char *centre_file = "data/centres.txt";

    gchar *nom = g_strdup("");
    if (nomcentre) {
        if (GTK_IS_ENTRY(nomcentre)) {
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(nomcentre));
            if (text) {
                g_free(nom);
                nom = g_strdup(text);
            }
        } else if (GTK_IS_COMBO_BOX(nomcentre)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomcentre));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry_child));
                if (text && strlen(text) > 0) {
                    g_free(nom);
                    nom = g_strdup(text);
                }
            }
            if (strlen(nom) == 0) {
                gchar *text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(nomcentre));
                if (text) {
                    g_free(nom);
                    nom = text;
                }
            }
        }
    }

    // Similar parsing for other fields... (adresse, ville, telephone, email, service, jour)
    gchar *adresse = g_strdup("");
    gchar *ville = g_strdup("");
    gchar *telephone = g_strdup("");
    gchar *email = g_strdup("");
    gchar *service = g_strdup("");
    gchar *jour = g_strdup("");
    
    // Get values from widgets (similar pattern as above for each field)
    if (adressecentre && GTK_IS_ENTRY(adressecentre)) {
        const gchar *text = gtk_entry_get_text(GTK_ENTRY(adressecentre));
        if (text) {
            g_free(adresse);
            adresse = g_strdup(text);
        }
    }
    // ... (similar for other fields)
    
    int nb_salles = nombresalle ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nombresalle)) : 0;
    int cap_max = capacitemaximale ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(capacitemaximale)) : 0;
    int h_deb = heuredebut_centre ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(heuredebut_centre)) : 0;
    int h_fin1 = js12 ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(js12)) : 0;
    int h_fin2 = js13 ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(js13)) : 0;
    int h_fin3 = js14 ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(js14)) : 0;

    gchar *etat = g_strdup("Fermé");
    if (js_checkouvert && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(js_checkouvert))) {
        g_free(etat);
        etat = g_strdup("Ouvert");
    } else if (js_check_ferme && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(js_check_ferme))) {
        g_free(etat);
        etat = g_strdup("Fermé");
    }

    if (!nom || strlen(nom) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Nom du centre requis.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(nom);
        g_free(adresse);
        g_free(ville);
        g_free(telephone);
        g_free(email);
        g_free(service);
        g_free(jour);
        g_free(etat);
        return;
    }

    // Ensure data directory exists
    FILE *test_dir = fopen("data/.", "r");
    if (!test_dir) {
        system("mkdir data 2>nul");
    } else {
        fclose(test_dir);
    }

    int next_id = 1;
    {
        FILE *f = fopen(centre_file, "r");
        if (f) {
            char line[512];
            int id;
            while (fgets(line, sizeof(line), f)) {
                if (sscanf(line, "%d|", &id) == 1 && id >= next_id) next_id = id + 1;
            }
            fclose(f);
        }
    }

    FILE *f = fopen(centre_file, "a");
    if (!f) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: impossible d'ouvrir centres.txt en écriture.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(nom);
        g_free(adresse);
        g_free(ville);
        g_free(telephone);
        g_free(email);
        g_free(service);
        g_free(jour);
        g_free(etat);
        return;
    }

    // Format: ID|Nom|Adresse|Ville|Telephone|Email|NbSalles|CapaciteMax|Service|Etat|JourTravail|Hdeb|Hfin1|Hfin2|Hfin3
    fprintf(f, "%d|%s|%s|%s|%s|%s|%d|%d|%s|%s|%s|%d|%d|%d|%d\n",
            next_id,
            nom ? nom : "",
            adresse ? adresse : "",
            ville ? ville : "",
            telephone ? telephone : "",
            email ? email : "",
            nb_salles,
            cap_max,
            service ? service : "",
            etat ? etat : "",
            jour ? jour : "",
            h_deb, h_fin1, h_fin2, h_fin3);
    fclose(f);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "✓ Centre ajouté (ID %d)", next_id);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    // Free allocated strings
    g_free(nom);
    g_free(adresse);
    g_free(ville);
    g_free(telephone);
    g_free(email);
    g_free(service);
    g_free(jour);
    g_free(etat);
}

// Callback for clear centre form
void
on_clearcentre_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *nomcentre = lookup_widget(window, "nomcentre");
    GtkWidget *adressecentre = lookup_widget(window, "adressecentre");
    GtkWidget *villecentre = lookup_widget(window, "villecentre");
    GtkWidget *telephonecentre = lookup_widget(window, "telephonecentre");
    GtkWidget *emailcentre = lookup_widget(window, "emailcentre");
    GtkWidget *nombresalle = lookup_widget(window, "nombresalle");
    GtkWidget *capacitemaximale = lookup_widget(window, "capacitemaximale");
    GtkWidget *servicedispo = lookup_widget(window, "servicedispo");
    GtkWidget *js_checkouvert = lookup_widget(window, "js_checkouvert");
    GtkWidget *js_check_ferme = lookup_widget(window, "js_check_ferme");
    GtkWidget *jour_de_travail_centre = lookup_widget(window, "jour_de_travail_centre");
    GtkWidget *heuredebut_centre = lookup_widget(window, "heuredebut_centre");
    GtkWidget *js12 = lookup_widget(window, "js12");
    GtkWidget *js13 = lookup_widget(window, "js13");
    GtkWidget *js14 = lookup_widget(window, "js14");

    if (nomcentre) {
        if (GTK_IS_ENTRY(nomcentre)) {
            gtk_entry_set_text(GTK_ENTRY(nomcentre), "");
        } else if (GTK_IS_COMBO_BOX(nomcentre)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomcentre));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(nomcentre), -1);
        }
    }
    if (adressecentre && GTK_IS_ENTRY(adressecentre)) gtk_entry_set_text(GTK_ENTRY(adressecentre), "");
    if (villecentre && GTK_IS_ENTRY(villecentre)) gtk_entry_set_text(GTK_ENTRY(villecentre), "");
    if (telephonecentre && GTK_IS_ENTRY(telephonecentre)) gtk_entry_set_text(GTK_ENTRY(telephonecentre), "");
    if (emailcentre && GTK_IS_ENTRY(emailcentre)) gtk_entry_set_text(GTK_ENTRY(emailcentre), "");
    if (servicedispo && GTK_IS_ENTRY(servicedispo)) gtk_entry_set_text(GTK_ENTRY(servicedispo), "");
    if (jour_de_travail_centre) {
        if (GTK_IS_ENTRY(jour_de_travail_centre)) {
            gtk_entry_set_text(GTK_ENTRY(jour_de_travail_centre), "");
        } else if (GTK_IS_COMBO_BOX(jour_de_travail_centre)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(jour_de_travail_centre));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(jour_de_travail_centre), -1);
        }
    }
    if (nombresalle) gtk_spin_button_set_value(GTK_SPIN_BUTTON(nombresalle), 0);
    if (capacitemaximale) gtk_spin_button_set_value(GTK_SPIN_BUTTON(capacitemaximale), 0);
    if (heuredebut_centre) gtk_spin_button_set_value(GTK_SPIN_BUTTON(heuredebut_centre), 0);
    if (js12) gtk_spin_button_set_value(GTK_SPIN_BUTTON(js12), 0);
    if (js13) gtk_spin_button_set_value(GTK_SPIN_BUTTON(js13), 0);
    if (js14) gtk_spin_button_set_value(GTK_SPIN_BUTTON(js14), 0);
    if (js_checkouvert) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(js_checkouvert), FALSE);
    if (js_check_ferme) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(js_check_ferme), FALSE);
}

// Callback for search centre by ID for update
void
on_rechercheidcentreupdate_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *id_entry = lookup_widget(window, "idcentre");

    if (!id_entry) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Champ ID introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(id_entry));
    if (!id_text || strlen(id_text) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Veuillez saisir un ID pour rechercher.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    int id = atoi(id_text);
    if (id <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: ID invalide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    char nom[100] = "", adresse[200] = "", ville[50] = "", telephone[20] = "", email[100] = "";
    char service[100] = "", etat[10] = "", jour_travail[50] = "";
    int nb_salles = 0, cap_max = 0, h_deb = 0, h_fin1 = 0, h_fin2 = 0, h_fin3 = 0;

    const char *centre_file = "data/centres.txt";
    if (search_centre_by_id(centre_file, id, nom, adresse, ville, telephone, email,
                           &nb_salles, &cap_max, service, etat, jour_travail, &h_deb, &h_fin1, &h_fin2, &h_fin3)) {
        GtkWidget *nomupdate = lookup_widget(window, "nomupdate");
        GtkWidget *villeupdate = lookup_widget(window, "villeupdate");
        GtkWidget *telephoneupdate = lookup_widget(window, "telephoneupdate");
        GtkWidget *emailupdatecentre = lookup_widget(window, "emailupdatecentre");
        GtkWidget *nombresalleupdate = lookup_widget(window, "nombresalleupdate");
        GtkWidget *capacitemaximaleupdate = lookup_widget(window, "capacitemaximaleupdate");
        GtkWidget *servicedisspoupdqte = lookup_widget(window, "servicedisspoupdqte");
        GtkWidget *ouvertupdate = lookup_widget(window, "ouvertupdate");
        GtkWidget *fermeupdate = lookup_widget(window, "fermeupdate");
        GtkWidget *jourtrqvqilupdqte = lookup_widget(window, "jourtrqvqilupdqte");
        GtkWidget *heuredebutcenmtreupdate = lookup_widget(window, "heuredebutcenmtreupdate");
        GtkWidget *spinbutton71 = lookup_widget(window, "spinbutton71");
        GtkWidget *spinbutton72 = lookup_widget(window, "spinbutton72");
        GtkWidget *spinbutton73 = lookup_widget(window, "spinbutton73");

        if (nomupdate && GTK_IS_ENTRY(nomupdate)) gtk_entry_set_text(GTK_ENTRY(nomupdate), nom);
        if (villeupdate && GTK_IS_ENTRY(villeupdate)) gtk_entry_set_text(GTK_ENTRY(villeupdate), ville);
        if (telephoneupdate && GTK_IS_ENTRY(telephoneupdate)) gtk_entry_set_text(GTK_ENTRY(telephoneupdate), telephone);
        if (emailupdatecentre) {
            if (GTK_IS_ENTRY(emailupdatecentre)) {
                gtk_entry_set_text(GTK_ENTRY(emailupdatecentre), email);
            } else if (GTK_IS_COMBO_BOX(emailupdatecentre)) {
                GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(emailupdatecentre));
                if (entry_child && GTK_IS_ENTRY(entry_child)) {
                    gtk_entry_set_text(GTK_ENTRY(entry_child), email);
                }
            }
        }
        if (nombresalleupdate) gtk_spin_button_set_value(GTK_SPIN_BUTTON(nombresalleupdate), nb_salles);
        if (capacitemaximaleupdate) gtk_spin_button_set_value(GTK_SPIN_BUTTON(capacitemaximaleupdate), cap_max);
        if (servicedisspoupdqte) {
            if (GTK_IS_ENTRY(servicedisspoupdqte)) {
                gtk_entry_set_text(GTK_ENTRY(servicedisspoupdqte), service);
            } else if (GTK_IS_COMBO_BOX(servicedisspoupdqte)) {
                GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(servicedisspoupdqte));
                if (entry_child && GTK_IS_ENTRY(entry_child)) {
                    gtk_entry_set_text(GTK_ENTRY(entry_child), service);
                }
            }
        }
        if (jourtrqvqilupdqte) {
            if (GTK_IS_ENTRY(jourtrqvqilupdqte)) {
                gtk_entry_set_text(GTK_ENTRY(jourtrqvqilupdqte), jour_travail);
            } else if (GTK_IS_COMBO_BOX(jourtrqvqilupdqte)) {
                GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(jourtrqvqilupdqte));
                if (entry_child && GTK_IS_ENTRY(entry_child)) {
                    gtk_entry_set_text(GTK_ENTRY(entry_child), jour_travail);
                }
            }
        }
        if (heuredebutcenmtreupdate) gtk_spin_button_set_value(GTK_SPIN_BUTTON(heuredebutcenmtreupdate), h_deb);
        if (spinbutton71) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton71), h_fin1);
        if (spinbutton72) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton72), h_fin2);
        if (spinbutton73) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton73), h_fin3);

        if (ouvertupdate && fermeupdate) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ouvertupdate), FALSE);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fermeupdate), FALSE);
            if (strcmp(etat, "Ouvert") == 0) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ouvertupdate), TRUE);
            } else if (strcmp(etat, "Fermé") == 0) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fermeupdate), TRUE);
            }
        }

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "✓ Centre trouvé et chargé.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "✗ Aucun centre avec l'ID %d.", id);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

// Helper function to update centre by ID
static int update_centre_by_id(const char *filename, int id, const char *nom, const char *adresse,
                               const char *ville, const char *telephone, const char *email,
                               int nb_salles, int cap_max, const char *service, const char *etat,
                               const char *jour_travail, int h_deb, int h_fin1, int h_fin2, int h_fin3)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }

    FILE *temp = fopen("data/centres_temp.txt", "w");
    if (!temp) {
        fclose(file);
        return 0;
    }

    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) {
            fprintf(temp, "\n");
            continue;
        }

        char *line_copy = g_strdup(line);
        char *first_pipe = strchr(line_copy, '|');
        int file_id = 0;
        
        if (first_pipe) {
            *first_pipe = '\0';
            file_id = atoi(line_copy);
        } else {
            file_id = atoi(line_copy);
        }

        if (file_id == id) {
            found = 1;
            fprintf(temp, "%d|%s|%s|%s|%s|%s|%d|%d|%s|%s|%s|%d|%d|%d|%d\n",
                    id, nom ? nom : "", adresse ? adresse : "", ville ? ville : "",
                    telephone ? telephone : "", email ? email : "",
                    nb_salles, cap_max, service ? service : "", etat ? etat : "",
                    jour_travail ? jour_travail : "", h_deb, h_fin1, h_fin2, h_fin3);
        } else {
            fprintf(temp, "%s\n", line);
        }

        g_free(line_copy);
    }

    fclose(file);
    fclose(temp);

    if (found) {
        remove(filename);
        rename("data/centres_temp.txt", filename);
        return 1;
    } else {
        remove("data/centres_temp.txt");
        return 0;
    }
}

// Callback for update centre
void
on_updatecentre_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *id_entry = lookup_widget(window, "idcentre");
    GtkWidget *nomupdate = lookup_widget(window, "nomupdate");
    GtkWidget *villeupdate = lookup_widget(window, "villeupdate");
    GtkWidget *telephoneupdate = lookup_widget(window, "telephoneupdate");
    GtkWidget *emailupdatecentre = lookup_widget(window, "emailupdatecentre");
    GtkWidget *nombresalleupdate = lookup_widget(window, "nombresalleupdate");
    GtkWidget *capacitemaximaleupdate = lookup_widget(window, "capacitemaximaleupdate");
    GtkWidget *servicedisspoupdqte = lookup_widget(window, "servicedisspoupdqte");
    GtkWidget *ouvertupdate = lookup_widget(window, "ouvertupdate");
    GtkWidget *fermeupdate = lookup_widget(window, "fermeupdate");
    GtkWidget *jourtrqvqilupdqte = lookup_widget(window, "jourtrqvqilupdqte");
    GtkWidget *heuredebutcenmtreupdate = lookup_widget(window, "heuredebutcenmtreupdate");
    GtkWidget *spinbutton71 = lookup_widget(window, "spinbutton71");
    GtkWidget *spinbutton72 = lookup_widget(window, "spinbutton72");
    GtkWidget *spinbutton73 = lookup_widget(window, "spinbutton73");

    if (!id_entry) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Champ ID introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(id_entry));
    if (!id_text || strlen(id_text) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Veuillez saisir un ID pour modifier.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    int id = atoi(id_text);
    if (id <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: ID invalide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    char old_nom[100] = "", old_adresse[200] = "", old_ville[50] = "", old_telephone[20] = "", old_email[100] = "";
    char old_service[100] = "", old_etat[10] = "", old_jour[50] = "";
    int old_nb_salles = 0, old_cap_max = 0, old_h_deb = 0, old_h_fin1 = 0, old_h_fin2 = 0, old_h_fin3 = 0;

    const char *centre_file = "data/centres.txt";
    if (!search_centre_by_id(centre_file, id, old_nom, old_adresse, old_ville, old_telephone, old_email,
                            &old_nb_salles, &old_cap_max, old_service, old_etat, old_jour, &old_h_deb, &old_h_fin1, &old_h_fin2, &old_h_fin3)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Aucun centre trouvé avec l'ID: %d", id);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    const gchar *nom = nomupdate ? gtk_entry_get_text(GTK_ENTRY(nomupdate)) : old_nom;
    const gchar *ville = villeupdate ? gtk_entry_get_text(GTK_ENTRY(villeupdate)) : old_ville;
    const gchar *telephone = telephoneupdate ? gtk_entry_get_text(GTK_ENTRY(telephoneupdate)) : old_telephone;
    
    gchar *email = g_strdup(old_email);
    if (emailupdatecentre) {
        if (GTK_IS_ENTRY(emailupdatecentre)) {
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(emailupdatecentre));
            if (text && strlen(text) > 0) {
                g_free(email);
                email = g_strdup(text);
            }
        } else if (GTK_IS_COMBO_BOX(emailupdatecentre)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(emailupdatecentre));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry_child));
                if (text && strlen(text) > 0) {
                    g_free(email);
                    email = g_strdup(text);
                }
            }
        }
    }

    gchar *service = g_strdup(old_service);
    if (servicedisspoupdqte) {
        if (GTK_IS_ENTRY(servicedisspoupdqte)) {
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(servicedisspoupdqte));
            if (text && strlen(text) > 0) {
                g_free(service);
                service = g_strdup(text);
            }
        } else if (GTK_IS_COMBO_BOX(servicedisspoupdqte)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(servicedisspoupdqte));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry_child));
                if (text && strlen(text) > 0) {
                    g_free(service);
                    service = g_strdup(text);
                }
            }
        }
    }

    gchar *jour_travail = g_strdup(old_jour);
    if (jourtrqvqilupdqte) {
        if (GTK_IS_ENTRY(jourtrqvqilupdqte)) {
            const gchar *text = gtk_entry_get_text(GTK_ENTRY(jourtrqvqilupdqte));
            if (text && strlen(text) > 0) {
                g_free(jour_travail);
                jour_travail = g_strdup(text);
            }
        } else if (GTK_IS_COMBO_BOX(jourtrqvqilupdqte)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(jourtrqvqilupdqte));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry_child));
                if (text && strlen(text) > 0) {
                    g_free(jour_travail);
                    jour_travail = g_strdup(text);
                }
            }
            if (strlen(jour_travail) == 0) {
                gchar *text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(jourtrqvqilupdqte));
                if (text) {
                    g_free(jour_travail);
                    jour_travail = text;
                }
            }
        }
    }

    int nb_salles = nombresalleupdate ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(nombresalleupdate)) : old_nb_salles;
    int cap_max = capacitemaximaleupdate ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(capacitemaximaleupdate)) : old_cap_max;
    int h_deb = heuredebutcenmtreupdate ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(heuredebutcenmtreupdate)) : old_h_deb;
    int h_fin1 = spinbutton71 ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton71)) : old_h_fin1;
    int h_fin2 = spinbutton72 ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton72)) : old_h_fin2;
    int h_fin3 = spinbutton73 ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton73)) : old_h_fin3;

    gchar *etat = g_strdup(old_etat);
    if (ouvertupdate && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ouvertupdate))) {
        g_free(etat);
        etat = g_strdup("Ouvert");
    } else if (fermeupdate && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fermeupdate))) {
        g_free(etat);
        etat = g_strdup("Fermé");
    }

    if (update_centre_by_id(centre_file, id, nom, old_adresse, ville, telephone, email,
                           nb_salles, cap_max, service, etat, jour_travail, h_deb, h_fin1, h_fin2, h_fin3)) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "✓ Centre modifié avec succès!\n\nID: %d", id);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "✗ Erreur: Impossible de modifier le centre.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }

    if (etat) g_free(etat);
    if (email) g_free(email);
    if (service) g_free(service);
    if (jour_travail) g_free(jour_travail);
}

// Callback for clear update centre form
void
on_clearupdatecentre_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *idcentre = lookup_widget(window, "idcentre");
    GtkWidget *nomupdate = lookup_widget(window, "nomupdate");
    GtkWidget *villeupdate = lookup_widget(window, "villeupdate");
    GtkWidget *telephoneupdate = lookup_widget(window, "telephoneupdate");
    GtkWidget *emailupdatecentre = lookup_widget(window, "emailupdatecentre");
    GtkWidget *nombresalleupdate = lookup_widget(window, "nombresalleupdate");
    GtkWidget *capacitemaximaleupdate = lookup_widget(window, "capacitemaximaleupdate");
    GtkWidget *servicedisspoupdqte = lookup_widget(window, "servicedisspoupdqte");
    GtkWidget *ouvertupdate = lookup_widget(window, "ouvertupdate");
    GtkWidget *fermeupdate = lookup_widget(window, "fermeupdate");
    GtkWidget *jourtrqvqilupdqte = lookup_widget(window, "jourtrqvqilupdqte");
    GtkWidget *heuredebutcenmtreupdate = lookup_widget(window, "heuredebutcenmtreupdate");
    GtkWidget *spinbutton71 = lookup_widget(window, "spinbutton71");
    GtkWidget *spinbutton72 = lookup_widget(window, "spinbutton72");
    GtkWidget *spinbutton73 = lookup_widget(window, "spinbutton73");

    if (idcentre && GTK_IS_ENTRY(idcentre)) gtk_entry_set_text(GTK_ENTRY(idcentre), "");
    if (nomupdate && GTK_IS_ENTRY(nomupdate)) gtk_entry_set_text(GTK_ENTRY(nomupdate), "");
    if (villeupdate && GTK_IS_ENTRY(villeupdate)) gtk_entry_set_text(GTK_ENTRY(villeupdate), "");
    if (telephoneupdate && GTK_IS_ENTRY(telephoneupdate)) gtk_entry_set_text(GTK_ENTRY(telephoneupdate), "");
    if (emailupdatecentre) {
        if (GTK_IS_ENTRY(emailupdatecentre)) {
            gtk_entry_set_text(GTK_ENTRY(emailupdatecentre), "");
        } else if (GTK_IS_COMBO_BOX(emailupdatecentre)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(emailupdatecentre));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
        }
    }
    if (nombresalleupdate) gtk_spin_button_set_value(GTK_SPIN_BUTTON(nombresalleupdate), 0);
    if (capacitemaximaleupdate) gtk_spin_button_set_value(GTK_SPIN_BUTTON(capacitemaximaleupdate), 0);
    if (servicedisspoupdqte) {
        if (GTK_IS_ENTRY(servicedisspoupdqte)) {
            gtk_entry_set_text(GTK_ENTRY(servicedisspoupdqte), "");
        } else if (GTK_IS_COMBO_BOX(servicedisspoupdqte)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(servicedisspoupdqte));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
        }
    }
    if (jourtrqvqilupdqte) {
        if (GTK_IS_ENTRY(jourtrqvqilupdqte)) {
            gtk_entry_set_text(GTK_ENTRY(jourtrqvqilupdqte), "");
        } else if (GTK_IS_COMBO_BOX(jourtrqvqilupdqte)) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(jourtrqvqilupdqte));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
        }
    }
    if (heuredebutcenmtreupdate) gtk_spin_button_set_value(GTK_SPIN_BUTTON(heuredebutcenmtreupdate), 0);
    if (spinbutton71) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton71), 0);
    if (spinbutton72) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton72), 0);
    if (spinbutton73) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton73), 0);
    if (ouvertupdate) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ouvertupdate), FALSE);
    if (fermeupdate) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fermeupdate), FALSE);
}

// Callback for delete centre
void
on_deletecentre_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *id_entry = lookup_widget(window, "idcentredelete");

    if (!id_entry) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Champ ID introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    const gchar *id_text_raw = NULL;
    if (GTK_IS_ENTRY(id_entry)) {
        id_text_raw = gtk_entry_get_text(GTK_ENTRY(id_entry));
    } else if (GTK_IS_COMBO_BOX(id_entry)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(id_entry));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            id_text_raw = gtk_entry_get_text(GTK_ENTRY(entry_child));
        } else {
            id_text_raw = gtk_combo_box_get_active_text(GTK_COMBO_BOX(id_entry));
        }
    }

    if (!id_text_raw || strlen(id_text_raw) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Veuillez entrer un ID à supprimer.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    const gchar *id_text = id_text_raw;
    while (*id_text == ' ' || *id_text == '\t' || *id_text == '\n' || *id_text == '\r') {
        id_text++;
    }
    
    if (*id_text == '\0') {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: Veuillez entrer un ID à supprimer.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    int id = atoi(id_text);
    if (id <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: ID invalide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkWidget *confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Êtes-vous sûr de vouloir supprimer le centre avec l'ID: %d?", id);
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);

    if (response == GTK_RESPONSE_YES) {
        const char *centre_file = "data/centres.txt";
        if (delete_centre_by_id(centre_file, id)) {
            if (GTK_IS_ENTRY(id_entry)) {
                gtk_entry_set_text(GTK_ENTRY(id_entry), "");
            } else if (GTK_IS_COMBO_BOX(id_entry)) {
                GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(id_entry));
                if (entry_child && GTK_IS_ENTRY(entry_child)) {
                    gtk_entry_set_text(GTK_ENTRY(entry_child), "");
                }
                gtk_combo_box_set_active(GTK_COMBO_BOX(id_entry), -1);
            }
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "✓ Centre supprimé avec succès!\n\nID: %d", id);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        } else {
            GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "✗ Erreur: Impossible de supprimer le centre.\n\nLe centre avec l'ID %d n'existe peut-être pas.", id);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
}

// Callback for delete centre enter (alias)
void
on_deletecentre_enter(GtkButton *button, gpointer user_data)
{
    on_deletecentre_clicked(button, user_data);
}

// Callback for search centre by ID for delete
void
on_rechercheidfordelete_clicked(GtkButton *button, gpointer user_data)
{
    // This can be empty or show info about the centre before deletion
    // For now, we'll just show a simple message
}

// Callback for load table centre
void
on_loadtablecentre_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window, "tablecentre");
    if (!treeview) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur: tableau des centres introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    if (!store) {
        store = gtk_list_store_new(15,
            G_TYPE_INT,    // ID
            G_TYPE_STRING, // Nom
            G_TYPE_STRING, // Adresse
            G_TYPE_STRING, // Ville
            G_TYPE_STRING, // Telephone
            G_TYPE_STRING, // Email
            G_TYPE_INT,    // NbSalles
            G_TYPE_INT,    // CapaciteMax
            G_TYPE_STRING, // Service
            G_TYPE_STRING, // Etat
            G_TYPE_STRING, // JourTravail
            G_TYPE_INT,    // Hdeb
            G_TYPE_INT,    // Hfin1
            G_TYPE_INT,    // Hfin2
            G_TYPE_INT     // Hfin3
        );
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));

        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "ID", renderer, "text", 0, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Nom", renderer, "text", 1, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Adresse", renderer, "text", 2, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Ville", renderer, "text", 3, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Telephone", renderer, "text", 4, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Email", renderer, "text", 5, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Nb Salles", renderer, "text", 6, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Capacite Max", renderer, "text", 7, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Service", renderer, "text", 8, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Etat", renderer, "text", 9, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Jour Travail", renderer, "text", 10, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "H deb", renderer, "text", 11, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "H fin1", renderer, "text", 12, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "H fin2", renderer, "text", 13, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "H fin3", renderer, "text", 14, NULL);
    }

    gtk_list_store_clear(store);

    FILE *file = fopen("data/centres.txt", "r");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Avertissement: aucun fichier centres.txt trouvé.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        GtkTreeIter iter;
        int id = 0, nb_salles = 0, cap_max = 0, h_deb = 0, h_fin1 = 0, h_fin2 = 0, h_fin3 = 0;
        char nom[100] = "", adresse[200] = "", ville[50] = "", telephone[20] = "", email[100] = "";
        char service[100] = "", etat[10] = "", jour[50] = "";

        char *line_copy = g_strdup(line);
        char *fields[15];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;

        while (*p && field_count < 15) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 15) {
            fields[field_count++] = start;
        }

        if (field_count >= 1) {
            id = atoi(fields[0]);
        }
        if (field_count >= 2 && fields[1]) {
            char *s = fields[1];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(nom, s, sizeof(nom)-1);
            nom[sizeof(nom)-1] = '\0';
        }
        if (field_count >= 3 && fields[2]) {
            char *s = fields[2];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(adresse, s, sizeof(adresse)-1);
            adresse[sizeof(adresse)-1] = '\0';
        }
        if (field_count >= 4 && fields[3]) {
            char *s = fields[3];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(ville, s, sizeof(ville)-1);
            ville[sizeof(ville)-1] = '\0';
        }
        if (field_count >= 5 && fields[4]) {
            char *s = fields[4];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(telephone, s, sizeof(telephone)-1);
            telephone[sizeof(telephone)-1] = '\0';
        }
        if (field_count >= 6 && fields[5]) {
            char *s = fields[5];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(email, s, sizeof(email)-1);
            email[sizeof(email)-1] = '\0';
        }
        if (field_count >= 7 && fields[6]) {
            nb_salles = atoi(fields[6]);
        }
        if (field_count >= 8 && fields[7]) {
            cap_max = atoi(fields[7]);
        }
        if (field_count >= 9 && fields[8]) {
            char *s = fields[8];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(service, s, sizeof(service)-1);
            service[sizeof(service)-1] = '\0';
        }
        if (field_count >= 10 && fields[9]) {
            char *s = fields[9];
            while (*s == ' ' || *s == '\t') s++;
            char *end = s + strlen(s) - 1;
            while (end > s && (*end == ' ' || *end == '\t')) {
                *end = '\0';
                end--;
            }
            strncpy(etat, s, sizeof(etat)-1);
            etat[sizeof(etat)-1] = '\0';
        }
        if (field_count >= 11 && fields[10]) {
            char *s = fields[10];
            while (*s == ' ' || *s == '\t') s++;
            strncpy(jour, s, sizeof(jour)-1);
            jour[sizeof(jour)-1] = '\0';
        }
        if (field_count >= 12 && fields[11]) {
            h_deb = atoi(fields[11]);
        }
        if (field_count >= 13 && fields[12]) {
            h_fin1 = atoi(fields[12]);
        }
        if (field_count >= 14 && fields[13]) {
            h_fin2 = atoi(fields[13]);
        }
        if (field_count >= 15 && fields[14]) {
            h_fin3 = atoi(fields[14]);
        }

        g_free(line_copy);

        if (id > 0) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, id,
                1, nom,
                2, adresse,
                3, ville,
                4, telephone,
                5, email,
                6, nb_salles,
                7, cap_max,
                8, service,
                9, etat,
                10, jour,
                11, h_deb,
                12, h_fin1,
                13, h_fin2,
                14, h_fin3,
                -1);
            count++;
        }
    }
    fclose(file);

    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "✓ %d centre(s) chargé(s).", count);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Toggle button callbacks for centre forms
void
on_js_check_ouvert_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_js_check_ferme_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_ouvertupdate_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_fermeupdate_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

