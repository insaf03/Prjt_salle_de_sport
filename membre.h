#ifndef MEMBRE_H_INCLUDED
#define MEMBRE_H_INCLUDED
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
typedef struct {
char id[[20];
char nom[20];
char prenom[50];
char centre[50];
char sexe[10];
int jour_naissance;
int mois_naissance;
int annee_naissance;
float poids;
char maladie[50];
char description_maladie[200];
char objectif[100];
char type_coach;
} Membre;
int ajouter_membre(Membre *m);
int modifier_membre(Membre *m);
int supprimer_membre(int id);
int rechercher_membre(int id);

#endif

