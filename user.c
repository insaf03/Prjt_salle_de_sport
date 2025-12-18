#include "membre.h"
#include <string.h>

int ajouter_membre(char *filename, membre m)
{
    FILE *f = fopen(filename, "a");
    if(f != NULL)
    {
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
                m.id, m.cin, m.nom, m.prenom, m.email, m.password,
                m.genre, m.date_naissance, m.centre, m.objectif,
                m.abonnement, m.role);
        fclose(f);
        return 1;
    }
    else return 0;
}

int modifier_membre(char *filename, int id, membre nouv)
{
    int tr = 0;
    membre m;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");
   
    if(f != NULL && f2 != NULL)
    {
        char line[512];
        while(fgets(line, sizeof(line), f) != NULL)
        {
            // Parse the line
            if(sscanf(line, "%d|", &m.id) == 1)
            {
                if(m.id == id)
                {
                    fprintf(f2, "%d|%s|%s|%s|%s|%s|%s|%s|%s|%s|%d|%d\n",
                            nouv.id, nouv.cin, nouv.nom, nouv.prenom,
                            nouv.email, nouv.password, nouv.genre,
                            nouv.date_naissance, nouv.centre, nouv.objectif,
                            nouv.abonnement, nouv.role);
                    tr = 1;
                }
                else
                {
                    fputs(line, f2);
                }
            }
            else
            {
                fputs(line, f2);
            }
        }
    }
   
    fclose(f);
    fclose(f2);
    remove(filename);
    rename("nouv.txt", filename);
    return tr;
}

int supprimer_membre(char *filename, int id)
{
    int tr = 0;
    membre m;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("nouv.txt", "w");
   
    if(f != NULL && f2 != NULL)
    {
        char line[512];
        while(fgets(line, sizeof(line), f) != NULL)
        {
            if(sscanf(line, "%d|", &m.id) == 1)
            {
                if(m.id == id)
                    tr = 1;
                else
                    fputs(line, f2);
            }
            else
            {
                fputs(line, f2);
            }
        }
    }
   
    fclose(f);
    fclose(f2);
    remove(filename);
    rename("nouv.txt", filename);
    return tr;
}

membre chercher_membre(char *filename, int id)
{
    membre m;
    int tr = 0;
    FILE *f = fopen(filename, "r");
   
    m.id = -1; // Initialize to indicate not found
   
    if(f != NULL)
    {
        char line[512];
        while(tr == 0 && fgets(line, sizeof(line), f) != NULL)
        {
            // Remove newline
            size_t len = strlen(line);
            if(len > 0 && line[len-1] == '\n')
                line[len-1] = '\0';
           
            // Parse the line manually
            char *fields[13];
            int field_count = 0;
            char *line_copy = strdup(line);
            char *p = line_copy;
            char *start = p;
           
            while(*p && field_count < 13)
            {
                if(*p == '|')
                {
                    *p = '\0';
                    fields[field_count++] = start;
                    start = p + 1;
                }
                p++;
            }
            if(field_count < 13)
                fields[field_count++] = start;
           
            // Check if ID matches
            if(field_count >= 12)
            {
                int file_id = atoi(fields[0]);
                if(file_id == id)
                {
                    m.id = file_id;
                    strncpy(m.cin, fields[1], sizeof(m.cin) - 1);
                    strncpy(m.nom, fields[2], sizeof(m.nom) - 1);
                    strncpy(m.prenom, fields[3], sizeof(m.prenom) - 1);
                    strncpy(m.email, fields[4], sizeof(m.email) - 1);
                    strncpy(m.password, fields[5], sizeof(m.password) - 1);
                    strncpy(m.genre, fields[6], sizeof(m.genre) - 1);
                    strncpy(m.date_naissance, fields[7], sizeof(m.date_naissance) - 1);
                    strncpy(m.centre, fields[8], sizeof(m.centre) - 1);
                    strncpy(m.objectif, fields[9], sizeof(m.objectif) - 1);
                    m.abonnement = atoi(fields[10]);
                    m.role = atoi(fields[11]);
                    tr = 1;
                }
            }
           
            free(line_copy);
        }
    }
   
    fclose(f);
    return m;
}

int get_next_membre_id(char *filename)
{
    FILE *f = fopen(filename, "r");
    if(!f)
        return 1; // First member
   
    int max_id = 0;
    int id;
    char line[512];
   
    while(fgets(line, sizeof(line), f))
    {
        if(sscanf(line, "%d|", &id) == 1)
        {
            if(id > max_id)
                max_id = id;
        }
    }
   
    fclose(f);
    return max_id + 1;
}

int membre_existe(char *filename, char *cin, char *email)
{
    FILE *f = fopen(filename, "r");
    if(!f)
        return 0;
   
    char line[512];
    char file_cin[100], file_email[100];
   
    while(fgets(line, sizeof(line), f))
    {
        if(sscanf(line, "%*d|%99[^|]|%*[^|]|%*[^|]|%99[^|]|", file_cin, file_email) == 2)
        {
            if((cin && strcmp(file_cin, cin) == 0) ||
               (email && strcmp(file_email, email) == 0))
            {
                fclose(f);
                return 1;
            }
        }
    }
   
    fclose(f);
    return 0;
}

int authentifier_membre(char *filename, char *email, char *password, int *role)
{
    FILE *f = fopen(filename, "r");
    if(!f)
        return 0;
   
    char line[512];
    while(fgets(line, sizeof(line), f))
    {
        char file_email[100], file_password[100];
        int file_role;
       
        // Parse line to get email, password, and role
        char *fields[13];
        int field_count = 0;
        char *line_copy = strdup(line);
        char *p = line_copy;
        char *start = p;
       
        while(*p && field_count < 13)
        {
            if(*p == '|')
            {
                *p = '\0';
                fields[field_count++] = start;
                start = p + 1;
            }
            p++;
        }
        if(field_count < 13)
            fields[field_count++] = start;
       
        if(field_count >= 12)
        {
            strncpy(file_email, fields[4], sizeof(file_email) - 1);
            strncpy(file_password, fields[5], sizeof(file_password) - 1);
            file_role = atoi(fields[11]);
           
            if(strcmp(file_email, email) == 0 &&
               strcmp(file_password, password) == 0)
            {
                *role = file_role;
                free(line_copy);
                fclose(f);
                return 1;
            }
        }
       
        free(line_copy);
    }
   
    fclose(f);
    return 0;
}
ben othmen Insaf <benothmeninsaf099@gmail.com>
	
10:43 (il y a 2 minutes)
	
	
À moi
void set_current_user(int id, const char *cin, const char *email);

/* Réinitialiser (déconnexion) */
void clear_current_user(void);



void set_current_user(int id, const char *cin, const char *email)
{
    g_current_member_id = id;

    if (cin)
        strncpy(g_current_member_cin, cin, sizeof(g_current_member_cin) - 1);
    else
        g_current_member_cin[0] = '\0';

    if (email)
        strncpy(g_current_member_email, email, sizeof(g_current_member_email) - 1);
    else
        g_current_member_email[0] = '\0';
}
