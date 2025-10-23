#ifndef Centre_H_INCLUDED
#define Centre_H_INCLUDED

#include<stdio.h>
typedef struct{
char Nom du centre [50];
char adresse [100];
char telephone[20];
char horaire[50];
int id_entraineur;
}centre;

void ajouter_centres(centre *c );
void supprimer_centres(centre*c);
void modifier_centres(centre*c);
void rechercher_centres(centre*c);
void afficher_centres(centre*c);

#endif
