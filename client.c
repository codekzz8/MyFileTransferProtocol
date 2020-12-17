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
#include "operations.h" //Pentru operarea cu directoare/fisiere (functionalitatile urmeaza a fi introduse in proiectul final)
#include "find.h"       //Pentru cautarea unui fisier/afisarea de informatii despre un fisier

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

void printCommands()
{
    printf("----------------------------------------------------------------------\n");
    printf("[+] Comenzi disponibile:\n");
    printf("    - help                         // Afisare comenzi\n");

    printf("    - cpwd                         // Afisare path client\n");
    printf("    - spwd                         // Afisare path server\n");

    printf("    - clistdirs                    // Afisare directoare client\n");
    printf("    - slistdirs                    // Afisare directoare server\n");

    printf("    - cmkdir 'nume_director'       // Creare director client\n");
    printf("    - smkdir 'nume_director'       // Creare director server\n");
    printf("    - crmdir 'nume_director'       // Stergere director client\n");
    printf("    - srmdir 'nume_director'       // Stergere director server\n");

    printf("    - ctouch 'nume_fisier'         // Creare fisier client\n");
    printf("    - stouch 'nume_fisier'         // Creare fisier server\n");
    printf("    - crm 'nume_fisier'            // Stergere fisier client\n");
    printf("    - srm 'nume_fisier'            // Stergere fisier server\n");

    printf("    - cgoto 'nume_director'        // Schimbare director client\n");
    printf("    - sgoto 'nume_director'        // Schimbare director server\n");

    printf("    - cfind 'nume_fisier'          // Cautare fisier client\n");
    printf("    - sfind 'nume_fisier'          // Cautare fisier server\n");

    printf("    - cinfo 'nume_fisier'          // Afisare informatii fisier client\n");
    printf("    - sinfo'nume_fisier'           // Afisare informatii fisier server\n");

    printf("    - getfile 'nume_fisier'        // Transfer fisier server -> client\n");
    printf("    - sendfile 'nume_fisier'       // Transfer fisier client -> server\n");

    printf("    - exit                         // Terminarea conexiunii\n");
    printf("----------------------------------------------------------------------\n");
}

bool clientCommand(char comanda[], char path[])
{
    int i;
    char rez[200], param[100], currentPath[100];
    int length = 0;
    bool pr = false;
    bzero(param, 100);
    bzero(rez, 200);
    bzero(currentPath, 100);

    for (i = 0; comanda[i]; i++)
    {
        if (pr == true)
            param[length++] = comanda[i];
        if (comanda[i] == ' ')
            pr = true;
    }
    param[length] = '\0';

    if (strcmp(comanda, "cpwd") == 0)
    {
        printf("[+] Path-ul curent este:\n");
        printf("    %s\n", path);
        return true;
    }
    else if (strcmp(comanda, "clistdirs") == 0)
    {
        char **rezultat = malloc(50 * sizeof(char *));
        int i;
        for (i = 0; i < 50; i++)
            rezultat[i] = malloc(200 * sizeof(char));

        listdir(path, 0, rezultat, 1);
        int lineNr = atoi(rezultat[0]);
        printf("%d\n", lineNr);
        sprintf(rezultat[1], "[+] Directoarele de pe client sunt:\n");
        printf("%s", rezultat[1]);
        for (i = 2; i <= lineNr; i++)
        {
            printf("%s\n", rezultat[i]);
        }
        for (i = 0; i < 50; i++)
            free(rezultat[i]);
        free(rezultat);
        rezultat = NULL;
        return true;
    }
    else if (strncmp(comanda, "cmkdir", 6) == 0) //123
    {
        strcpy(currentPath, path);
        strcat(currentPath, "/");
        strcat(currentPath, param);
        int pid = fork();
        if (pid == 0) //proces copil in care se executa comanda "mkdir 'nume_dir'"
        {
            execlp("mkdir", "mkdir", currentPath, NULL);
            exit(0);
        }
        //proces parinte
        wait(NULL);
        char dir_name[20];
        bzero(dir_name, 20);
        if (strchr(comanda, '/'))
            strcpy(dir_name, strrchr(param, '/') + 1);
        else
            strcpy(dir_name, param);
        printf("[+] Directorul cu numele %s a fost creat cu succes!\n", dir_name);
        return true;
    }
    else if (strncmp(comanda, "crmdir", 6) == 0) //123
    {
        strcpy(currentPath, path);
        strcat(currentPath, "/");
        strcat(currentPath, param);
        int pid = fork();
        if (pid == 0) //proces copil in care se executa comanda "mkdir 'nume_dir'"
        {
            execlp("rmdir", "rmdir", currentPath, NULL);
            exit(0);
        }
        //proces parinte
        wait(NULL);
        char dir_name[20];
        bzero(dir_name, 20);
        if (strchr(comanda, '/'))
            strcpy(dir_name, strrchr(param, '/') + 1);
        else
            strcpy(dir_name, param);
        printf("[+] Directorul cu numele %s a fost sters cu succes!\n", dir_name);
        return true;
    }
    else if (strncmp(comanda, "ctouch", 6) == 0)
    {
        strcpy(currentPath, path);
        strcat(currentPath, "/");
        strcat(currentPath, param);

        int pid = fork();
        if (pid == 0)
        {
            execlp("touch", "touch", currentPath, NULL);
            exit(0);
        }
        else
        {
            wait(NULL);
        }
        printf("[+] Fisierul cu numele %s a fost creat cu succes!\n", param);
        return true;
    }
    else if (strncmp(comanda, "crm", 3) == 0)
    {
        strcpy(currentPath, path);
        strcat(currentPath, "/");
        strcat(currentPath, param);
        int pid = fork();
        if (pid == 0) //proces copil in care se executa comanda "mkdir 'nume_dir'"
        {
            execlp("rm", "rm", currentPath, NULL);
            exit(0);
        }
        //proces parinte
        wait(NULL);
        char dir_name[20];
        bzero(dir_name, 20);
        if (strchr(comanda, '/'))
            strcpy(dir_name, strrchr(param, '/') + 1);
        else
            strcpy(dir_name, param);
        printf("[+] Directorul cu numele %s a fost sters cu succes!\n", dir_name);
        return true;
    }
    else if (strncmp(comanda, "cgoto", 5) == 0)
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

        printf("[+] Path-ul curent este:\n");
        printf("    %s\n", path);
        return true;
    }
    else if (strncmp(comanda, "cfind", 5) == 0)
    {
        printf("[+] Fisierul cu numele %s nu a fost gasit!\n", param);
        return true;
    }
    else if (strncmp(comanda, "cinfo", 5) == 0)
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

    /* Dupa autentificare, se afla directorul curent al clientului */
    int pipefd[2];
    pipe(pipefd);
    int pid = fork();
    if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);
        close(pipefd[1]);
        execlp("pwd", "pwd", NULL);
        exit(0);
    }
    else
    {
        wait(NULL);
        char pwd[100];
        bzero(pwd, 100);
        close(pipefd[1]);
        if (read(pipefd[0], pwd, sizeof(pwd)) > 0)
        {
            strcpy(path, pwd);
            path[strlen(path) - 1] = '\0';
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
                int nrLines = atoi(msg), i;
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
