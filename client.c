#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

/* Headere */
#include "login.h" //Pentru verificare whitelist/blacklist + autentificare
//#include "encryption.h" //Pentru criptarea parolei (este inclus in header-ul login.h)
//#include "operations.h" //Pentru operarea cu directoare/fisiere (functionalitatile urmeaza a fi introduse in proiectul final)
#include "find.h" //Pentru cautarea unui fisier/afisarea de informatii despre un fisier

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

void printCommands()
{
    printf("---------------------------------------------\n");
    printf("[+] Comenzi disponibile:\n");
    printf("    - help                         // Afisare comenzi\n");
    printf("    - listdirs_client              // Afisare directoare\n");
    printf("    - listdirs_server              // Afisare directoare\n");
    printf("    - goto_client 'nume_director'  // Schimbare director client\n");
    printf("    - goto_server 'nume_director'  // Schimbare director server\n");
    printf("    - find_client 'nume_fisier'    // Cautare fisier client\n");
    printf("    - find_server 'nume_fisier'    // Cautare fisier server\n");
    printf("    - info_client 'nume_fisier'    // Afisare informatii fisier client\n");
    printf("    - info_server 'nume_fisier'    // Afisare informatii fisier server\n");
    printf("    - getfile 'nume_fisier'        // Transfer fisier server -> client\n");
    printf("    - sendfile 'nume_fisier'       // Transfer fisier client -> server\n");
    printf("    - exit                         // Terminarea conexiunii\n");
    printf("---------------------------------------------\n");
}

bool clientCommand(char comanda[], char path[])
{
    int i;
    char rez[200], param[100], currentPath[100];
    int length = 0;
    bool pr = false;
    bzero(param, 100);
    bzero(rez, 200);

    for (i = 0; comanda[i]; i++)
    {
        if (pr == true)
            param[length++] = comanda[i];
        if (comanda[i] == ' ')
            pr = true;
    }
    param[length] = '\0';

    if (strcmp(comanda, "listdirs_client") == 0)
    {
        printf("[+] Directoarele de pe client sunt:\n");
        printf("    - cl1\n");
        printf("    - cl2\n");
        printf("    - cl3\n");
        printf("    - ...\n");
        return true;
    }
    else if (strncmp(comanda, "goto_client", 11) == 0)
    {
        if (strcmp(param, "~") == 0)
            strcpy(path, "~");
        else if (strcmp(param, "..") == 0)
        {
            for (i = strlen(path) - 1; i >= 0; i--)
            {
                if (path[i] == '/')
                    break;
            }
            strcpy(path + i, "");
        }
        else
        {
            strcat(path, "/");
            strcat(path, param);
        }
        printf("[+] Path-ul curent este: %s.\n", path);
        return true;
    }
    else if (strncmp(comanda, "find_client", 11) == 0)
    {
        printf("[+] Fisierul cu numele %s nu a fost gasit!\n", param);
        return true;
    }
    else if (strncmp(comanda, "info_client", 11) == 0)
    {
        printf("[+] Se afiseaza toate informatiile despre fisierul cu numele %s!\n", param);
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    int sd;                    // descriptorul de socket
    struct sockaddr_in server; // structura folosita pentru conectare
    char msg[200];             // mesajul trimis
    char buff[8192];

    char path[100];
    strcpy(path, "~");

    char username[100], parola[100];
    char option[200];

    int bytes_read;

    FILE *output;

    /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[client] Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);

    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    printf("-=/ MyFileTransferProtocol \\=-\n");
    printf("---------------------------------------------\n");
    printf("[+] Introduceti cifra corespunzatoare urmatoarelor optiuni:\n");
    printf("[+] 1. Autentificare\n");
    printf("[+] 2. Exit\n");
    while (1)
    {
        printf("[+] Optiune: ");
        fflush(stdout);
        read(0, option, 200);
        option[strlen(option) - 1] = '\0';

        if (strcmp(option, "Autentificare") == 0)
        {
            if (write(sd, option, 200) <= 0)
            {
                perror("[+] Eroare la write() spre server.\n");
                return errno;
            }
            authenticate(sd, server);
            break;
        }
        else if (strcmp(option, "Exit") == 0)
        {
            if (write(sd, option, 200) <= 0)
            {
                perror("[+] Eroare la write() spre server.\n");
                return errno;
            }
            bytes_read = read(sd, msg, 200);
            if (bytes_read < 0)
            {
                perror("[+] Eroare la read() de la server.\n");
                return errno;
            }
            int nrLines = msg[0] - '0', i;
            for (i = 0; i < nrLines; i++)
            {
                bytes_read = read(sd, msg, 200);
                if (bytes_read < 0)
                {
                    perror("[+] Eroare la read() de la server.\n");
                    return errno;
                }
                printf("%s\n", msg);
            }
            close(sd);
            return 0;
        }
        else
        {
            printf("[+] Comanda nu exista!\n");
        }
    }

    printCommands();

    while (1)
    {
        /* citirea mesajului */
        bzero(msg, 200);
        printf("[+] ");
        fflush(stdout);
        read(0, msg, 200);
        msg[strlen(msg) - 1] = '\0';

        if (strcmp(msg, "help") == 0)
            printCommands();
        else
        {
            if (!clientCommand(msg, path))
            {
                if (write(sd, msg, 200) <= 0)
                {
                    perror("[+] Eroare la write() spre server.\n");
                    return errno;
                }

                bytes_read = read(sd, msg, 200);
                if (bytes_read < 0)
                {
                    perror("[+] Eroare la read() de la server.\n");
                    return errno;
                }
                int nrLines = msg[0] - '0', i;
                for (i = 0; i < nrLines; i++)
                {
                    bytes_read = read(sd, msg, 200);
                    if (bytes_read < 0)
                    {
                        perror("[+] Eroare la read() de la server.\n");
                        return errno;
                    }
                    printf("%s\n", msg);
                }
                if (strstr(msg, "Se inchide conexiunea"))
                {
                    close(sd);
                    return 0;
                }
            }
        }
    }
}
