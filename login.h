#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "encryption.h"
#include <sqlite3.h>

char getInfo(char user[], char pass[])
{
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_open("database.db", &db);
    if (rc != SQLITE_OK)
    {
        perror("Baza de date nu a putut fi deschisa!\n");
        return '0';
    }
    char sqlCmd[100], *err_msg = 0;

    sprintf(sqlCmd, "SELECT username FROM blacklist WHERE username = '%s';", user);
    rc = sqlite3_prepare_v2(db, sqlCmd, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        perror("Eroare la pregatirea interogarii!\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '0';
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) //Daca username-ul este in blacklist
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '2';
    }

    sprintf(sqlCmd, "SELECT username FROM whitelist WHERE username = '%s';", user);
    rc = sqlite3_prepare_v2(db, sqlCmd, strlen(sqlCmd), &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        perror("Eroare la pregatirea interogarii!\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '0';
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) //Daca username-ul este in whitelist
    {
        sprintf(sqlCmd, "SELECT password FROM whitelist WHERE password = '%s';", pass);
        rc = sqlite3_prepare_v2(db, sqlCmd, -1, &stmt, NULL);
        if (rc != SQLITE_OK)
        {
            perror("Eroare la pregatirea interogarii!\n");
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return '0';
        }
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) //Daca parola este corecta
        {
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return '1';
        }
    }
    sqlite3_close(db);
    return '0'; // Parola gresita
}

char insertAccount(char user[], char pass[])
{
    sqlite3 *db;
    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_open("database.db", &db);
    if (rc != SQLITE_OK)
    {
        perror("Baza de date nu a putut fi deschisa!\n");
        return '0';
    }
    char sqlCmd[100], *err_msg = 0;

    sprintf(sqlCmd, "SELECT username FROM blacklist WHERE username = '%s';", user);
    rc = sqlite3_prepare_v2(db, sqlCmd, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        perror("Eroare la pregatirea interogarii!\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '0';
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) //Daca username-ul exista in blacklist
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '2';
    }

    sprintf(sqlCmd, "SELECT username FROM whitelist WHERE username = '%s';", user);
    rc = sqlite3_prepare_v2(db, sqlCmd, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        perror("Eroare la pregatirea interogarii!\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '0';
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) //Daca username-ul exista in whitelist
    {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return '2';
    }

    sprintf(sqlCmd, "INSERT INTO whitelist VALUES('%s', '%s');", user, pass);
    rc = sqlite3_exec(db, sqlCmd, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        sqlite3_close(db);
        return '0';
    }
    sqlite3_close(db);
    return '1';
}

void create_account(int sd)
{
    char value;
    char username[100], parola[100];
    while (1)
    {
        printf("---------------------------------------------\n");
        printf("[+] Va rugam sa va introduceti datele cerute...\n");
    create:
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
            printf("[+] Username-ul deja exista! Introduceti un username diferit...\n");
            fflush(stdout);
            goto create;
        }
        else if (value == '0')
        {
            printf("---------------------------------------------\n");
            printf("[+] Contul nu a putut fi creat! Mai incercati...\n");
            fflush(stdout);
            goto create;
        }
        else
            break;
    }
    printf("---------------------------------------------\n");
    printf("[+] Contul a fost creat cu succes!\n");
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