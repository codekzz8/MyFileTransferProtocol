#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "encryption.h"

char *getusername(char string[])
{
    char rez[100];
    int i, len = 0;
    for (i = 0; string[i]; i++)
    {
        if (string[i] == '#')
            break;
        rez[i] = string[i];
    }
    rez[i] = '\0';
    return strdup(rez);
}

char *getpassword(char string[])
{
    char rez[100];
    int i, len = 0;
    bool flag = false;
    for (i = 0; string[i]; i++)
    {
        if (flag == true)
            rez[len++] = string[i];
        if (string[i] == '#')
            flag = true;
    }
    rez[len] = '\0';
    return strdup(rez);
}

char getInfo(char user[], char pass[])
{
    FILE *whitelist;
    FILE *blacklist;

    whitelist = fopen("whitelist.txt", "r");
    blacklist = fopen("blacklist.txt", "r");

    if (whitelist == NULL || blacklist == NULL)
    {
        perror("[server] Eroare la deschidere fisiere.\n");
        return errno;
    }

    char line[100];
    size_t len = 0;
    ssize_t bytes;
    int i;
    while (fgets(line, sizeof(line), blacklist) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (strcmp(line, user) == 0)
            return '2'; // User-ul este blacklisted!
        bzero(line, 100);
    }
    fclose(blacklist);
    char password[50];
    while (fgets(line, sizeof(line), whitelist) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        if (strcmp(user, getusername(line)) == 0)
        {
            strcpy(password, getpassword(line));
            //secure(pass);
            if (strcmp(password, pass) == 0)
                return '1'; // Parola corecta
        }
        bzero(line, 100);
        bzero(password, 50);
    }
    fclose(whitelist);
    return '0'; // Parola gresita
}

void authenticate(int sd)
{
    char value;
    char username[100], parola[100];
    while (1)
    {
        printf("---------------------------------------------\n");
        printf("[+] Va rugam sa va autentificati la server...\n");
    autentificare:
        printf("[+] username: ");
        fflush(stdout);
        bzero(username, 100);
        read(0, username, 100);
        username[strlen(username) - 1] = '\0';

        if (write(sd, username, sizeof(username)) < 0)
        {
            perror("[+] Eroare la scriere user spre server.\n");
            return;
        }

        printf("[+] parola: ");
        fflush(stdout);
        bzero(parola, 100);
        read(0, parola, 100);
        parola[strlen(parola) - 1] = '\0';

        secure(parola);
        if (write(sd, parola, sizeof(parola)) < 0)
        {
            perror("[+] Eroare la scriere pass spre server.\n");
            return;
        }

        if (read(sd, &value, sizeof(value)) < 0)
        {
            perror("[+] Eroare la primire rezultat.\n");
            return;
        }

        if (value == '2')
        {
            printf("---------------------------------------------\n");
            printf("[+] User-ul se afla pe blacklist! Incercati sa va autentificati cu alt cont...\n");
            fflush(stdout);
            goto autentificare;
        }
        else if (value == '0')
        {
            printf("---------------------------------------------\n");
            printf("[+] Autentificare esuata! Mai incercati...\n");
            fflush(stdout);
            goto autentificare;
        }
        else
            break;
    }
    printf("---------------------------------------------\n");
    printf("[+] Autentificare realizata cu succes!\n");
}