#include "cours.h"
#include <string.h>

int ajouter_cours(char *filename, cours c)
{
    FILE *f = fopen(filename, "a");
    if (f != NULL)
    {
        fprintf(f, "%d %s %s %d %d %s %s %d %d\n",
                c.id, c.nom_cours, c.jour, c.h_debut, c.h_fin,
                c.entraineur, c.niveau, c.capacite_max, c.inscrits);

        fclose(f);
        return 1;
    }
    return 0;
}

int modifier_cours(char *filename, int id, cours nouv)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("tmp.txt", "w");
    int tr = 0;
    cours c;

    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%d %49s %19s %d %d %49s %19s %d %d\n",
                      &c.id, c.nom_cours, c.jour, &c.h_debut, &c.h_fin,
                      c.entraineur, c.niveau, &c.capacite_max, &c.inscrits) != EOF)
        {
            if (c.id == id)
            {
                fprintf(f2, "%d %s %s %d %d %s %s %d %d\n",
                        nouv.id, nouv.nom_cours, nouv.jour,
                        nouv.h_debut, nouv.h_fin,
                        nouv.entraineur, nouv.niveau,
                        nouv.capacite_max, nouv.inscrits);

                tr = 1;
            }
            else
            {
                fprintf(f2, "%d %s %s %d %d %s %s %d %d\n",
                        c.id, c.nom_cours, c.jour, c.h_debut, c.h_fin,
                        c.entraineur, c.niveau, c.capacite_max, c.inscrits);
            }
        }
    }

    fclose(f);
    fclose(f2);
    remove(filename);
    rename("tmp.txt", filename);

    return tr;
}

int supprimer_cours(char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("tmp.txt", "w");
    int tr = 0;
    cours c;

    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%d %49s %19s %d %d %49s %19s %d %d\n",
                      &c.id, c.nom_cours, c.jour, &c.h_debut, &c.h_fin,
                      c.entraineur, c.niveau, &c.capacite_max, &c.inscrits) != EOF)
        {
            if (c.id == id)
                tr = 1;
            else
                fprintf(f2, "%d %s %s %d %d %s %s %d %d\n",
                        c.id, c.nom_cours, c.jour, c.h_debut, c.h_fin,
                        c.entraineur, c.niveau, c.capacite_max, c.inscrits);
        }
    }

    fclose(f);
    fclose(f2);
    remove(filename);
    rename("tmp.txt", filename);

    return tr;
}

cours chercher_cours(char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    cours c;
    int tr = 0;

    if (f != NULL)
    {
        while (fscanf(f, "%d %49s %19s %d %d %49s %19s %d %d\n",
                      &c.id, c.nom_cours, c.jour, &c.h_debut, &c.h_fin,
                      c.entraineur, c.niveau, &c.capacite_max, &c.inscrits) != EOF && tr == 0)
        {
            if (c.id == id)
                tr = 1;
        }
        fclose(f);
    }

    if (!tr)
        c.id = -1;

    return c;
}

// ---- Fonction membre : inscription ----

int inscrire_membre(char *filename, int id)
{
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("tmp.txt", "w");
    int tr = 0;
    cours c;

    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%d %49s %19s %d %d %49s %19s %d %d\n",
                      &c.id, c.nom_cours, c.jour, &c.h_debut, &c.h_fin,
                      c.entraineur, c.niveau, &c.capacite_max, &c.inscrits) != EOF)
        {
            if (c.id == id)
            {
                if (c.inscrits < c.capacite_max)
                {
                    c.inscrits++;
                    tr = 1;  // inscription réussie
                }
            }

            fprintf(f2, "%d %s %s %d %d %s %s %d %d\n",
                    c.id, c.nom_cours, c.jour, c.h_debut, c.h_fin,
                    c.entraineur, c.niveau, c.capacite_max, c.inscrits);
        }
    }

    fclose(f);
    fclose(f2);
    remove(filename);
    rename("tmp.txt", filename);

    return tr;
}
