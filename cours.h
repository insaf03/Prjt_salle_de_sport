#ifndef COURS_H_INCLUDED
#define COURS_H_INCLUDED

#include <stdio.h>

typedef struct
{
    int id;                     // identifiant unique du cours
    char nom_cours[50];
    char jour[20];
    int h_debut;
    int h_fin;
    char entraineur[50];
    char niveau[20];
    int capacite_max;
    int inscrits;              // nombre actuel de membres inscrits
} cours;

// Gestion admin
int ajouter_cours(char *filename, cours c);
int modifier_cours(char *filename, int id, cours nouv);
int supprimer_cours(char *filename, int id);
cours chercher_cours(char *filename, int id);

// Gestion membre
int inscrire_membre(char *filename, int id);

#endif // COURS_H_INCLUDED
