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
#include "../include/equipement.h"

// Forward declarations
extern void populate_all_trainers(GtkComboBox *combobox);
extern void populate_combobox_with_centres(GtkComboBox *combobox);

// Helper function to populate combobox with equipment types
void populate_equipment_types(GtkComboBox *combobox)
{
    if (!combobox) return;

    // Get or create list store
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combobox));
    
    if (!store) {
        store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
        g_object_unref(store);
    }

    // Clear existing items
    gtk_list_store_clear(store);

    // Read equipment from equipement.txt and get unique types
    FILE *file = fopen("data/equipement.txt", "r");
    if (!file) {
        return;
    }

    char line[512];
    GHashTable *types_hash = g_hash_table_new(g_str_hash, g_str_equal);
    
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        // Parse line: ID|Nomequip|Typeequip|NomEntraineur|NomCentre|Quantite|Etat
        char *line_copy = g_strdup(line);
        char *fields[7];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;

        // Split by pipe
        while (*p && field_count < 7) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 7) {
            fields[field_count++] = start;
        }

        // Get type (field index 2)
        if (field_count >= 3 && fields[2]) {
            char *type = fields[2];
            while (*type == ' ' || *type == '\t') type++;
            if (strlen(type) > 0 && !g_hash_table_contains(types_hash, type)) {
                g_hash_table_insert(types_hash, g_strdup(type), GINT_TO_POINTER(1));
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, type, -1);
            }
        }

        g_free(line_copy);
    }

    fclose(file);
    g_hash_table_destroy(types_hash);
}

// Helper function to populate combobox with equipment names
void populate_equipment_names(GtkComboBox *combobox)
{
    if (!combobox) return;

    // Get or create list store
    GtkListStore *store = GTK_LIST_STORE(gtk_combo_box_get_model(combobox));
    
    if (!store) {
        store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
        g_object_unref(store);
    }

    // Clear existing items
    gtk_list_store_clear(store);

    // Read equipment from equipement.txt
    FILE *file = fopen("data/equipement.txt", "r");
    if (!file) {
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;

        // Parse line: ID|Nomequip|Typeequip|NomEntraineur|NomCentre|Quantite|Etat
        char *line_copy = g_strdup(line);
        char *fields[7];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;

        // Split by pipe
        while (*p && field_count < 7) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 7) {
            fields[field_count++] = start;
        }

        // Get equipment name (field index 1)
        if (field_count >= 2 && fields[1]) {
            char *nom = fields[1];
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

// Callback for ajouter equipment button
void
on_buttonAjouterNaj_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    // Get all widget values
    GtkWidget *nomequip_w = lookup_widget(window, "nomequip");
    GtkWidget *neufequip_w = lookup_widget(window, "neufequip");
    GtkWidget *pannetype_w = lookup_widget(window, "pannetype");
    GtkWidget *usetype_w = lookup_widget(window, "usetype");
    GtkWidget *typeequip_w = lookup_widget(window, "typeequip");
    GtkWidget *nomentrainequip_w = lookup_widget(window, "nomentrainequip");
    GtkWidget *centreequipe_w = lookup_widget(window, "centreequipe");
    GtkWidget *quantiteequipe_w = lookup_widget(window, "quantiteequipe");
    
    if (!nomequip_w || !typeequip_w || !nomentrainequip_w || !centreequipe_w || !quantiteequipe_w) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Widgets non trouvés.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get values
    const gchar *nomequip = gtk_entry_get_text(GTK_ENTRY(nomequip_w));
    const gchar *typeequip = gtk_entry_get_text(GTK_ENTRY(typeequip_w));
    
    // Get equipment state (neuf, panne, use)
    gchar *etat = g_strdup("");
    if (neufequip_w && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(neufequip_w))) {
        g_free(etat);
        etat = g_strdup("Neuf");
    } else if (pannetype_w && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(pannetype_w))) {
        g_free(etat);
        etat = g_strdup("Panne");
    } else if (usetype_w && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(usetype_w))) {
        g_free(etat);
        etat = g_strdup("Use");
    }
    
    // Get trainer name from combobox
    gchar *nomentraineur = NULL;
    if (GTK_IS_COMBO_BOX(nomentrainequip_w)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomentrainequip_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            nomentraineur = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_child)));
        }
        if (!nomentraineur || strlen(nomentraineur) == 0) {
            nomentraineur = gtk_combo_box_get_active_text(GTK_COMBO_BOX(nomentrainequip_w));
        }
    }
    
    // Get centre name from combobox
    gchar *nomcentre = NULL;
    if (GTK_IS_COMBO_BOX(centreequipe_w)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(centreequipe_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            nomcentre = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_child)));
        }
        if (!nomcentre || strlen(nomcentre) == 0) {
            nomcentre = gtk_combo_box_get_active_text(GTK_COMBO_BOX(centreequipe_w));
        }
    }
    
    gint quantite = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(quantiteequipe_w));
    
    // Validate
    if (!nomequip || strlen(nomequip) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez entrer le nom de l'équipement.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(nomentraineur);
        g_free(nomcentre);
        g_free(etat);
        return;
    }
    
    if (!typeequip || strlen(typeequip) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez entrer le type d'équipement.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        g_free(nomentraineur);
        g_free(nomcentre);
        g_free(etat);
        return;
    }
    
    // Get next equipment ID
    int next_id = 1;
    FILE *file = fopen("data/equipement.txt", "r");
    if (file) {
        char line[512];
        int max_id = 0;
        while (fgets(line, sizeof(line), file)) {
            if (strlen(line) > 0) {
                int id = atoi(line);
                if (id > max_id) max_id = id;
            }
        }
        fclose(file);
        next_id = max_id + 1;
    }
    
    // Save equipment: ID|Nomequip|Typeequip|NomEntraineur|NomCentre|Quantite|Etat
    file = fopen("data/equipement.txt", "a");
    if (file) {
        fprintf(file, "%d|%s|%s|%s|%s|%d|%s\n", 
                next_id,
                nomequip,
                typeequip,
                nomentraineur ? nomentraineur : "",
                nomcentre ? nomcentre : "",
                quantite,
                etat ? etat : "");
        fclose(file);
        
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Équipement ajouté avec succès!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Impossible d'écrire dans le fichier.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
    g_free(nomentraineur);
    g_free(nomcentre);
    g_free(etat);
}

// Callback for clear equipment form
void
on_clearequip_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    // Clear all equipment fields
    GtkWidget *nomequip_w = lookup_widget(window, "nomequip");
    GtkWidget *neufequip_w = lookup_widget(window, "neufequip");
    GtkWidget *pannetype_w = lookup_widget(window, "pannetype");
    GtkWidget *usetype_w = lookup_widget(window, "usetype");
    GtkWidget *typeequip_w = lookup_widget(window, "typeequip");
    GtkWidget *nomentrainequip_w = lookup_widget(window, "nomentrainequip");
    GtkWidget *centreequipe_w = lookup_widget(window, "centreequipe");
    GtkWidget *quantiteequipe_w = lookup_widget(window, "quantiteequipe");
    
    if (nomequip_w && GTK_IS_ENTRY(nomequip_w)) {
        gtk_entry_set_text(GTK_ENTRY(nomequip_w), "");
    }
    if (neufequip_w && GTK_IS_TOGGLE_BUTTON(neufequip_w)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(neufequip_w), FALSE);
    }
    if (pannetype_w && GTK_IS_TOGGLE_BUTTON(pannetype_w)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pannetype_w), FALSE);
    }
    if (usetype_w && GTK_IS_TOGGLE_BUTTON(usetype_w)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(usetype_w), FALSE);
    }
    if (typeequip_w && GTK_IS_ENTRY(typeequip_w)) {
        gtk_entry_set_text(GTK_ENTRY(typeequip_w), "");
    }
    if (nomentrainequip_w && GTK_IS_COMBO_BOX(nomentrainequip_w)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomentrainequip_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            gtk_entry_set_text(GTK_ENTRY(entry_child), "");
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(nomentrainequip_w), -1);
    }
    if (centreequipe_w && GTK_IS_COMBO_BOX(centreequipe_w)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(centreequipe_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            gtk_entry_set_text(GTK_ENTRY(entry_child), "");
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(centreequipe_w), -1);
    }
    if (quantiteequipe_w && GTK_IS_SPIN_BUTTON(quantiteequipe_w)) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(quantiteequipe_w), 0);
    }
}

// Callback for load equipment table
void
on_loadequip_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window, "tableauequip");
    
    if (!treeview) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Impossible de trouver le tableau des équipements.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Get or create list store
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
    
    if (!store) {
        // Create new list store with columns: ID, Nom, Type, Entraineur, Centre, Quantite
        store = gtk_list_store_new(6, 
            G_TYPE_INT,    // ID
            G_TYPE_STRING, // Nom
            G_TYPE_STRING, // Type
            G_TYPE_STRING, // Entraineur
            G_TYPE_STRING, // Centre
            G_TYPE_INT);   // Quantite
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
        
        // Create columns if they don't exist
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "ID", renderer, "text", 0, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Nom", renderer, "text", 1, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Type", renderer, "text", 2, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Entraineur", renderer, "text", 3, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Centre", renderer, "text", 4, NULL);
        gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(treeview), -1, "Quantite", renderer, "text", 5, NULL);
    }
    
    // Clear existing data
    gtk_list_store_clear(store);
    
    // Open file and read data
    FILE *file = fopen("data/equipement.txt", "r");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Aucun équipement trouvé.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;
        
        // Parse line: ID|Nomequip|Typeequip|NomEntraineur|NomCentre|Quantite|Etat
        char *line_copy = g_strdup(line);
        char *fields[7];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;
        
        // Split by pipe (handle both 6 and 7 fields)
        while (*p && field_count < 7) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 7) {
            fields[field_count++] = start;
        }
        
        // Handle both old format (6 fields) and new format (7 fields with Etat)
        if (field_count >= 6) {
            int id = atoi(fields[0]);
            char *nom = fields[1] ? fields[1] : "";
            char *type = fields[2] ? fields[2] : "";
            char *entraineur = fields[3] ? fields[3] : "";
            char *centre = fields[4] ? fields[4] : "";
            int quantite = atoi(fields[5]);
            
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, id,
                1, nom,
                2, type,
                3, entraineur,
                4, centre,
                5, quantite,
                -1);
        }
        
        g_free(line_copy);
    }
    
    fclose(file);
}

// Callback for delete equipment
void
on_deleteequip_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entryIDNsupp_w = lookup_widget(window, "entryIDNsupp");
    
    if (!entryIDNsupp_w) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Champ ID introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entryIDNsupp_w));
    if (!id_text || strlen(id_text) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez entrer un ID à supprimer.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    int id = atoi(id_text);
    if (id <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: ID invalide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    // Confirm deletion
    GtkWidget *confirm_dialog = gtk_message_dialog_new(
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Êtes-vous sûr de vouloir supprimer l'équipement avec l'ID: %d?", id);
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // Read file and remove equipment
        FILE *file = fopen("data/equipement.txt", "r");
        if (!file) {
            GtkWidget *dialog = gtk_message_dialog_new(
                GTK_WINDOW(window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Erreur: Fichier équipement introuvable.");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
        
        char line[512];
        char temp_file[] = "data/equipement.txt.tmp";
        FILE *temp = fopen(temp_file, "w");
        gboolean found = FALSE;
        
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
            
            int file_id = atoi(line);
            if (file_id == id) {
                found = TRUE;
                // Skip this line (delete it)
            } else {
                fprintf(temp, "%s\n", line);
            }
        }
        
        fclose(file);
        fclose(temp);
        
        if (found) {
            remove("data/equipement.txt");
            rename(temp_file, "data/equipement.txt");
            
            // Clear the entry field
            gtk_entry_set_text(GTK_ENTRY(entryIDNsupp_w), "");
            
            GtkWidget *dialog = gtk_message_dialog_new(
                GTK_WINDOW(window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "✓ Équipement supprimé avec succès!\n\nID: %d", id);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        } else {
            remove(temp_file);
            GtkWidget *dialog = gtk_message_dialog_new(
                GTK_WINDOW(window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "✗ Erreur: Équipement non trouvé.\n\nL'équipement avec l'ID %d n'existe peut-être pas.", id);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
}

// Callback for reserving equipment
void
on_reserverequipment_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
   
    GtkWidget *equipstocked_w = lookup_widget(window, "equipstocked");
    GtkWidget *nomequipstocked_w = lookup_widget(window, "nomequipstocked");
    GtkWidget *numerequip_w = lookup_widget(window, "numerequip");
   
    if (equipstocked_w && GTK_IS_COMBO_BOX(equipstocked_w)) {
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(equipstocked_w));
        if (!model) {
            populate_equipment_types(GTK_COMBO_BOX(equipstocked_w));
        } else {
            GtkTreeIter iter;
            if (!gtk_tree_model_get_iter_first(model, &iter)) {
                populate_equipment_types(GTK_COMBO_BOX(equipstocked_w));
            }
        }
    }
   
    if (nomequipstocked_w && GTK_IS_COMBO_BOX(nomequipstocked_w)) {
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(nomequipstocked_w));
        if (!model) {
            populate_equipment_names(GTK_COMBO_BOX(nomequipstocked_w));
        } else {
            GtkTreeIter iter;
            if (!gtk_tree_model_get_iter_first(model, &iter)) {
                populate_equipment_names(GTK_COMBO_BOX(nomequipstocked_w));
            }
        }
    }
   
    if (!equipstocked_w || !nomequipstocked_w || !numerequip_w) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Widgets non trouvés.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
   
    gchar *type_equip = get_widget_text(GTK_WIDGET(equipstocked_w));
    gchar *nom_equip = get_widget_text(GTK_WIDGET(nomequipstocked_w));
    gint num_equip = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(numerequip_w));
   
    if (!type_equip || strlen(type_equip) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez sélectionner le type d'équipement.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (type_equip) g_free(type_equip);
        if (nom_equip) g_free(nom_equip);
        return;
    }
   
    if (!nom_equip || strlen(nom_equip) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez sélectionner le nom de l'équipement.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (type_equip) g_free(type_equip);
        if (nom_equip) g_free(nom_equip);
        return;
    }
   
    if (num_equip <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez entrer une quantité valide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (type_equip) g_free(type_equip);
        if (nom_equip) g_free(nom_equip);
        return;
    }
   
    FILE *file = fopen("data/equipement.txt", "r");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Fichier équipement introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (type_equip) g_free(type_equip);
        if (nom_equip) g_free(nom_equip);
        return;
    }
   
    char line[512];
    gboolean found = FALSE;
    int equip_id = 0;
    gchar *equip_centre = NULL;
    gchar *equip_trainer = NULL;
    int current_quantity = 0;
   
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;
       
        char *line_copy = g_strdup(line);
        char *fields[7];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;
       
        while (*p && field_count < 7) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 7) {
            fields[field_count++] = start;
        }
       
        if (field_count >= 6) {
            char *nom_file = fields[1] ? fields[1] : "";
            char *type_file = fields[2] ? fields[2] : "";
           
            while (*nom_file == ' ' || *nom_file == '\t') nom_file++;
            while (*type_file == ' ' || *type_file == '\t') type_file++;
           
            if (strcmp(nom_file, nom_equip) == 0 && strcmp(type_file, type_equip) == 0) {
                equip_id = atoi(fields[0]);
                current_quantity = atoi(fields[5]);
                if (fields[4]) equip_centre = g_strdup(fields[4]);
                if (fields[3]) equip_trainer = g_strdup(fields[3]);
                found = TRUE;
                break;
            }
        }
       
        g_free(line_copy);
    }
    fclose(file);
   
    if (!found) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Équipement non trouvé.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (type_equip) g_free(type_equip);
        if (nom_equip) g_free(nom_equip);
        return;
    }
   
    if (current_quantity < num_equip) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Quantité insuffisante.\n\nDisponible: %d\nDemandé: %d",
            current_quantity, num_equip);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (type_equip) g_free(type_equip);
        if (nom_equip) g_free(nom_equip);
        if (equip_centre) g_free(equip_centre);
        if (equip_trainer) g_free(equip_trainer);
        return;
    }
   
    file = fopen("data/equipement.txt", "r");
    char temp_file[] = "data/equipement.txt.tmp";
    FILE *temp = fopen(temp_file, "w");
   
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
        char *fields[7];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;
       
        while (*p && field_count < 7) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 7) {
            fields[field_count++] = start;
        }
       
        if (field_count >= 6) {
            int file_id = atoi(fields[0]);
            if (file_id == equip_id) {
                int new_quantity = current_quantity - num_equip;
                if (field_count >= 7 && fields[6]) {
                    fprintf(temp, "%s|%s|%s|%s|%s|%d|%s\n",
                            fields[0], fields[1], fields[2], fields[3], fields[4],
                            new_quantity, fields[6]);
                } else {
                    fprintf(temp, "%s|%s|%s|%s|%s|%d\n",
                            fields[0], fields[1], fields[2], fields[3], fields[4],
                            new_quantity);
                }
            } else {
                fprintf(temp, "%s\n", line);
            }
        } else {
            fprintf(temp, "%s\n", line);
        }
       
        g_free(line_copy);
    }
   
    fclose(file);
    fclose(temp);
    remove("data/equipement.txt");
    rename(temp_file, "data/equipement.txt");
   
    FILE *reserv_file = fopen("data/equipment_reservations.txt", "a");
    if (reserv_file) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char date_str[20];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
       
        int next_reserv_id = 1;
        FILE *check_file = fopen("data/equipment_reservations.txt", "r");
        if (check_file) {
            char check_line[512];
            int max_id = 0;
            while (fgets(check_line, sizeof(check_line), check_file)) {
                if (strlen(check_line) > 0) {
                    int id = atoi(check_line);
                    if (id > max_id) max_id = id;
                }
            }
            fclose(check_file);
            next_reserv_id = max_id + 1;
        }
       
        fprintf(reserv_file, "%d|%d|%s|%s|%d|%s|%s|%s|%s|%d|%s\n",
                next_reserv_id,
                g_current_member_id > 0 ? g_current_member_id : 0,
                strlen(g_current_member_cin) > 0 ? g_current_member_cin : "",
                strlen(g_current_member_email) > 0 ? g_current_member_email : "",
                equip_id,
                nom_equip ? nom_equip : "",
                type_equip ? type_equip : "",
                equip_centre ? equip_centre : "",
                equip_trainer ? equip_trainer : "",
                num_equip,
                date_str);
        fclose(reserv_file);
       
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "✓ Équipement réservé avec succès!\n\nÉquipement: %s\nType: %s\nQuantité: %d\nCentre: %s\n\nLa relation entraîneur-équipement a été enregistrée.",
            nom_equip, type_equip, num_equip, equip_centre ? equip_centre : "N/A");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
       
        if (equipstocked_w) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(equipstocked_w));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(equipstocked_w), -1);
        }
        if (nomequipstocked_w) {
            GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomequipstocked_w));
            if (entry_child && GTK_IS_ENTRY(entry_child)) {
                gtk_entry_set_text(GTK_ENTRY(entry_child), "");
            }
            gtk_combo_box_set_active(GTK_COMBO_BOX(nomequipstocked_w), -1);
        }
        if (numerequip_w) {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(numerequip_w), 0);
        }
    }
   
    if (type_equip) g_free(type_equip);
    if (nom_equip) g_free(nom_equip);
    if (equip_centre) g_free(equip_centre);
    if (equip_trainer) g_free(equip_trainer);
}

// Callback for clear fields reserve equipment
void
on_clearfieldsreserveequipement_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    GtkWidget *equipstocked_w = lookup_widget(window, "equipstocked");
    GtkWidget *nomequipstocked_w = lookup_widget(window, "nomequipstocked");
    GtkWidget *numerequip_w = lookup_widget(window, "numerequip");
    
    if (equipstocked_w) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(equipstocked_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            gtk_entry_set_text(GTK_ENTRY(entry_child), "");
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(equipstocked_w), -1);
    }
    if (nomequipstocked_w) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomequipstocked_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            gtk_entry_set_text(GTK_ENTRY(entry_child), "");
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(nomequipstocked_w), -1);
    }
    if (numerequip_w) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(numerequip_w), 0);
    }
}

// Callback for search equipment by ID for modify
void
on_buttonRechNmod_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entryIDNmod_w = lookup_widget(window, "entryIDNmod");
    
    if (!entryIDNmod_w) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Champ ID introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entryIDNmod_w));
    if (!id_text || strlen(id_text) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez entrer un ID.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    int id = atoi(id_text);
    if (id <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: ID invalide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    FILE *file = fopen("data/equipement.txt", "r");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Fichier équipement introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    char line[512];
    gboolean found = FALSE;
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        if (len == 0) continue;
        
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
            found = TRUE;
            
            char *fields[7];
            int field_count = 0;
            char *p = line_copy;
            char *start = p;
            
            while (*p && field_count < 7) {
                if (*p == '|') {
                    *p = '\0';
                    fields[field_count++] = start;
                    start = p + 1;
                }
                p++;
            }
            if (field_count < 7) {
                fields[field_count++] = start;
            }
            
            GtkWidget *typeequipupdate_w = lookup_widget(window, "typeequipupdate");
            GtkWidget *spinbuttonQuantNmod_w = lookup_widget(window, "spinbuttonQuantNmod");
            GtkWidget *nomentrainrespo_w = lookup_widget(window, "nomentrainrespo");
            GtkWidget *centreupdateequip_w = lookup_widget(window, "centreupdateequip");
            GtkWidget *checkbutton28_w = lookup_widget(window, "checkbutton28");
            GtkWidget *checkbutton29_w = lookup_widget(window, "checkbutton29");
            GtkWidget *checkbutton30_w = lookup_widget(window, "checkbutton30");
            
            if (field_count >= 3 && fields[2] && typeequipupdate_w) {
                char *type = fields[2];
                while (*type == ' ' || *type == '\t') type++;
                gtk_entry_set_text(GTK_ENTRY(typeequipupdate_w), type);
            }
            
            if (field_count >= 6 && fields[5] && spinbuttonQuantNmod_w) {
                int quantite = atoi(fields[5]);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbuttonQuantNmod_w), quantite);
            }
            
            if (field_count >= 4 && fields[3] && nomentrainrespo_w) {
                char *trainer = fields[3];
                while (*trainer == ' ' || *trainer == '\t') trainer++;
                if (GTK_IS_COMBO_BOX(nomentrainrespo_w)) {
                    GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomentrainrespo_w));
                    if (entry_child && GTK_IS_ENTRY(entry_child)) {
                        gtk_entry_set_text(GTK_ENTRY(entry_child), trainer);
                    }
                }
            }
            
            if (field_count >= 5 && fields[4] && centreupdateequip_w) {
                char *centre = fields[4];
                while (*centre == ' ' || *centre == '\t') centre++;
                if (GTK_IS_COMBO_BOX(centreupdateequip_w)) {
                    GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(centreupdateequip_w));
                    if (entry_child && GTK_IS_ENTRY(entry_child)) {
                        gtk_entry_set_text(GTK_ENTRY(entry_child), centre);
                    }
                }
            }
            
            if (field_count >= 7 && fields[6]) {
                char *etat = fields[6];
                while (*etat == ' ' || *etat == '\t') etat++;
                
                if (checkbutton28_w) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton28_w), strcmp(etat, "Neuf") == 0);
                if (checkbutton29_w) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton29_w), strcmp(etat, "Panne") == 0);
                if (checkbutton30_w) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton30_w), strcmp(etat, "Use") == 0);
            }
            
            GtkWidget *dialog = gtk_message_dialog_new(
                GTK_WINDOW(window),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "✓ Équipement trouvé et chargé.");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            
            g_free(line_copy);
            break;
        }
        
        g_free(line_copy);
    }
    fclose(file);
    
    if (!found) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "✗ Aucun équipement trouvé avec l'ID: %d", id);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

// Callback for save/modify equipment
void
on_buttonEnrNmod_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entryIDNmod_w = lookup_widget(window, "entryIDNmod");
    
    if (!entryIDNmod_w) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Champ ID introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    const gchar *id_text = gtk_entry_get_text(GTK_ENTRY(entryIDNmod_w));
    if (!id_text || strlen(id_text) == 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Veuillez rechercher un équipement d'abord.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    int id = atoi(id_text);
    if (id <= 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: ID invalide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    GtkWidget *typeequipupdate_w = lookup_widget(window, "typeequipupdate");
    GtkWidget *spinbuttonQuantNmod_w = lookup_widget(window, "spinbuttonQuantNmod");
    GtkWidget *nomentrainrespo_w = lookup_widget(window, "nomentrainrespo");
    GtkWidget *centreupdateequip_w = lookup_widget(window, "centreupdateequip");
    GtkWidget *checkbutton28_w = lookup_widget(window, "checkbutton28");
    GtkWidget *checkbutton29_w = lookup_widget(window, "checkbutton29");
    GtkWidget *checkbutton30_w = lookup_widget(window, "checkbutton30");
    
    const gchar *typeequip = typeequipupdate_w && GTK_IS_ENTRY(typeequipupdate_w) ? gtk_entry_get_text(GTK_ENTRY(typeequipupdate_w)) : "";
    gint quantite = spinbuttonQuantNmod_w ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbuttonQuantNmod_w)) : 0;
    gchar *nomentraineur = get_widget_text(GTK_WIDGET(nomentrainrespo_w));
    gchar *nomcentre = get_widget_text(GTK_WIDGET(centreupdateequip_w));
    
    gchar *etat = g_strdup("");
    if (checkbutton28_w && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton28_w))) {
        g_free(etat);
        etat = g_strdup("Neuf");
    } else if (checkbutton29_w && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton29_w))) {
        g_free(etat);
        etat = g_strdup("Panne");
    } else if (checkbutton30_w && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton30_w))) {
        g_free(etat);
        etat = g_strdup("Use");
    }
    
    FILE *file = fopen("data/equipement.txt", "r");
    if (!file) {
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Erreur: Fichier équipement introuvable.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        if (nomentraineur) g_free(nomentraineur);
        if (nomcentre) g_free(nomcentre);
        if (etat) g_free(etat);
        return;
    }
    
    char line[512];
    char temp_file[] = "data/equipement.txt.tmp";
    FILE *temp = fopen(temp_file, "w");
    gboolean found = FALSE;
    gchar *nomequip_old = NULL;
    
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
        char *fields[7];
        int field_count = 0;
        char *p = line_copy;
        char *start = p;
        
        while (*p && field_count < 7) {
            if (*p == '|') {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if (field_count < 7) {
            fields[field_count++] = start;
        }
        
        int file_id = atoi(fields[0]);
        if (file_id == id) {
            found = TRUE;
            nomequip_old = g_strdup(fields[1] ? fields[1] : "");
            fprintf(temp, "%d|%s|%s|%s|%s|%d|%s\n",
                    id,
                    nomequip_old,
                    typeequip && strlen(typeequip) > 0 ? typeequip : (fields[2] ? fields[2] : ""),
                    nomentraineur && strlen(nomentraineur) > 0 ? nomentraineur : (fields[3] ? fields[3] : ""),
                    nomcentre && strlen(nomcentre) > 0 ? nomcentre : (fields[4] ? fields[4] : ""),
                    quantite > 0 ? quantite : atoi(fields[5] ? fields[5] : "0"),
                    etat && strlen(etat) > 0 ? etat : (fields[6] ? fields[6] : ""));
        } else {
            fprintf(temp, "%s\n", line);
        }
        
        g_free(line_copy);
    }
    
    fclose(file);
    fclose(temp);
    
    if (found) {
        remove("data/equipement.txt");
        rename(temp_file, "data/equipement.txt");
        
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "✓ Équipement modifié avec succès!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        remove(temp_file);
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "✗ Erreur: Équipement non trouvé.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
    if (nomentraineur) g_free(nomentraineur);
    if (nomcentre) g_free(nomcentre);
    if (etat) g_free(etat);
    if (nomequip_old) g_free(nomequip_old);
}

// Callback for cancel/clear update equipment form
void
on_buttonAnnNmod_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    GtkWidget *entryIDNmod_w = lookup_widget(window, "entryIDNmod");
    GtkWidget *typeequipupdate_w = lookup_widget(window, "typeequipupdate");
    GtkWidget *spinbuttonQuantNmod_w = lookup_widget(window, "spinbuttonQuantNmod");
    GtkWidget *nomentrainrespo_w = lookup_widget(window, "nomentrainrespo");
    GtkWidget *centreupdateequip_w = lookup_widget(window, "centreupdateequip");
    GtkWidget *checkbutton28_w = lookup_widget(window, "checkbutton28");
    GtkWidget *checkbutton29_w = lookup_widget(window, "checkbutton29");
    GtkWidget *checkbutton30_w = lookup_widget(window, "checkbutton30");
    
    if (entryIDNmod_w && GTK_IS_ENTRY(entryIDNmod_w)) {
        gtk_entry_set_text(GTK_ENTRY(entryIDNmod_w), "");
    }
    if (typeequipupdate_w && GTK_IS_ENTRY(typeequipupdate_w)) {
        gtk_entry_set_text(GTK_ENTRY(typeequipupdate_w), "");
    }
    if (spinbuttonQuantNmod_w && GTK_IS_SPIN_BUTTON(spinbuttonQuantNmod_w)) {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbuttonQuantNmod_w), 0);
    }
    if (nomentrainrespo_w && GTK_IS_COMBO_BOX(nomentrainrespo_w)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(nomentrainrespo_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            gtk_entry_set_text(GTK_ENTRY(entry_child), "");
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(nomentrainrespo_w), -1);
    }
    if (centreupdateequip_w && GTK_IS_COMBO_BOX(centreupdateequip_w)) {
        GtkWidget *entry_child = gtk_bin_get_child(GTK_BIN(centreupdateequip_w));
        if (entry_child && GTK_IS_ENTRY(entry_child)) {
            gtk_entry_set_text(GTK_ENTRY(entry_child), "");
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(centreupdateequip_w), -1);
    }
    if (checkbutton28_w && GTK_IS_TOGGLE_BUTTON(checkbutton28_w)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton28_w), FALSE);
    }
    if (checkbutton29_w && GTK_IS_TOGGLE_BUTTON(checkbutton29_w)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton29_w), FALSE);
    }
    if (checkbutton30_w && GTK_IS_TOGGLE_BUTTON(checkbutton30_w)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton30_w), FALSE);
    }
}

// Callback for equipstocked changed - populate equipment types
void
on_equipstocked_changed(GtkComboBox *combobox, gpointer user_data)
{
    if (combobox) {
        GtkTreeModel *model = gtk_combo_box_get_model(combobox);
        if (!model) {
            populate_equipment_types(combobox);
        } else {
            GtkTreeIter iter;
            gboolean has_items = gtk_tree_model_get_iter_first(model, &iter);
            if (!has_items) {
                populate_equipment_types(combobox);
            }
        }
    }
}

// Callback for nomequipstocked changed - populate equipment names
void
on_nomequipstocked_changed(GtkComboBox *combobox, gpointer user_data)
{
    if (combobox) {
        GtkTreeModel *model = gtk_combo_box_get_model(combobox);
        if (!model) {
            populate_equipment_names(combobox);
        } else {
            GtkTreeIter iter;
            gboolean has_items = gtk_tree_model_get_iter_first(model, &iter);
            if (!has_items) {
                populate_equipment_names(combobox);
            }
        }
    }
}

// Toggle button callbacks for equipment forms
void
on_neufequip_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_pannetype_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_usetype_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_checkbutton29_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

void
on_checkbutton30_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
    (void)togglebutton;
    (void)user_data;
}

