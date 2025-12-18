#ifndef EQUIPEMENT_H_INCLUDED
#define EQUIPEMENT_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char nom[100];
    char type[50];
    char nom_entraineur[100];
    char nom_centre[100];
    int quantite_disponible;
    char etat[20];
    char ch_vip[200]; 
    char ch_deb[200];
    char ch_snr[200];
} Equipement;

// CRUD Operations
int ajouter_equipement(const char *filename, Equipement e);
int modifier_equipement(const char *filename, int id, Equipement nouv);
int supprimer_equipement(const char *filename, int id);
Equipement chercher_equipement(const char *filename, int id);
int lister_equipements(const char *filename, Equipement *liste, int max);

#endif

