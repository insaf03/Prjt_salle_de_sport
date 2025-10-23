#ifndef COURS_H_INCLUDED

#define COURS_H_INCLUDED



#include <gtk/gtk.h>



#define COURS_FILE "data/cours.txt"



typedef struct {

char id[32];

char nom[100];

char type[50];

char jour[20];

char heure[10];

char entraineur[100];

} Cours;



int ajouter_cours( Cours *c);

int modifier_cours( Cours *c);

int supprimer_cours( char *id);

void ajouter_cours(

void remplir_treeview_from_file(GtkTreeView *treeview);



#endif // COURS_H_INCLUDED


