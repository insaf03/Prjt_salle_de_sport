#ifndef MEMBRE_H
#define MEMBRE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    char cin[100];
    char nom[100];
    char prenom[100];
    char email[100];
    char password[100];
    char date_naissance[100];
    char centre[100];
    char objectif[100];
    int abonnement;
    int role; 
} membre;
typedef struct {
     char nom[50];
     char nom_entrai[50];
     char niveau[50];
     char jour[50];
     }

int ajouter_membre(char *filename, membre m);
int modifier_membre(char *filename, int id, membre nouv);
int supprimer_membre(char *filename, int id);
membre chercher_membre(char *filename, int id);

int get_next_membre_id(char *filename);
int membre_existe(char *filename, char *cin, char *email);
int authentifier_membre(char *filename, char *email, char *password, int *role);
void set_current_user(int id, const char *cin, const char *email);
void clear_current_user(void);


#endif
