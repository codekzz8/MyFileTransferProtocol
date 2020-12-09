#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

/* Headere */
#include "login.h" //Pentru verificare whitelist/blacklist + autentificare
//#include "encryption.h" //Pentru criptarea parolei (este inclus in header-ul login.h)
//#include "operations.h" //Pentru operarea cu directoare/fisiere (urmeaza a fi introdus in proiectul final)
#include "find.h" //Pentru cautarea unui fisier/afisarea de informatii despre un fisier

/* portul folosit */
#define PORT 2728

extern int errno; /* eroarea returnata de unele apeluri */

/* functie de convertire a adresei IP a clientului in sir de caractere */
char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy(str, inet_ntoa(address.sin_addr));
    /* portul utilizat de client */
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

void commandManager(char comanda[], char path[], char **rezultat)
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
    if (strcmp(comanda, "exit") == 0 || strcmp(comanda, "Exit") == 0)
    {
        strcpy(rezultat[0], "1");
        strcpy(rezultat[1], "[+] Se inchide conexiunea...");
    }
    else if (strcmp(comanda, "listdirs_server") == 0)
    {
        strcpy(rezultat[0], "5");
        strcpy(rezultat[1], "[+] Directoarele de pe server sunt:");
        strcpy(rezultat[2], "    - sv1");
        strcpy(rezultat[3], "    - sv2");
        strcpy(rezultat[4], "    - sv3");
        strcpy(rezultat[5], "    - ...");
    }
    else if (strncmp(comanda, "goto_server", 11) == 0)
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
        strcpy(rezultat[0], "1");
        sprintf(rezultat[1], "[+] Path-ul curent este: %s.", path);
    }
    else if (strncmp(comanda, "find_server", 11) == 0)
    {
        strcpy(rezultat[0], "1");
        sprintf(rezultat[1], "[+] Fisierul cu numele %s a fost gasit!", param);
    }
    else if (strncmp(comanda, "info_server", 11) == 0)
    {
        strcpy(rezultat[0], "1");
        sprintf(rezultat[1], "[+] Se afiseaza toate informatiile despre fisierul cu numele %s!", param);
    }
    else if (strncmp(comanda, "getfile", 7) == 0)
    {
        strcpy(rezultat[0], "1");
        sprintf(rezultat[1], "[+] Se trimite fisierul cu numele %s de pe server!", param);
    }
    else if (strncmp(comanda, "sendfile", 8) == 0)
    {
        strcpy(rezultat[0], "1");
        sprintf(rezultat[1], "[+] Se trimite fisierul cu numele %s pe server!", param);
    }
    else
    {
        strcpy(rezultat[0], "1");
        sprintf(rezultat[1], "[+] Comanda nu exista!");
        printf("%s\n", rezultat[1]);
    }
}

int receiveAndSend(int fd, char path[])
{
    char buffer[200]; /* mesajul */
    int bytes;        /* numarul de octeti cititi/scrisi */
    char msg[200];    //mesajul primit de la client
    char **rezultat = malloc(50 * sizeof(char *));
    int i;
    for (i = 0; i < 50; i++)
        rezultat[i] = malloc(200 * sizeof(char));

    bytes = read(fd, msg, sizeof(buffer));
    if (bytes < 0)
    {
        perror("Eroare la read() de la client.\n");
        return 0;
    }
    printf("[server]Mesajul a fost receptionat...%s\n", msg);

    commandManager(msg, path, rezultat);

    int nrLines;
    if (write(fd, rezultat[0], bytes) < 0)
    {
        perror("[server] Eroare la write() catre client.\n");
        return 0;
    }
    nrLines = rezultat[0][0] - '0';
    for (i = 1; i <= nrLines; i++)
    {
        if (write(fd, rezultat[i], bytes) < 0)
        {
            perror("[server] Eroare la write() catre client.\n");
            return 0;
        }
    }
    printf("[server] Rezultat trimis catre client!\n");
    if (strstr(rezultat[1], "Se inchide conexiunea"))
        return -1;
    return bytes;
}

/* programul */
int main()
{
    struct sockaddr_in server; /* structurile pentru server si clienti */
    struct sockaddr_in from;
    fd_set readfds;    /* multimea descriptorilor de citire */
    fd_set actfds;     /* multimea descriptorilor activi */
    struct timeval tv; /* structura de timp pentru select() */
    int sd, client;    /* descriptori de socket */
    int optval = 1;    /* optiune folosita pentru setsockopt()*/
    int fd;            /* descriptor folosit pentru 
				   parcurgerea listelor de descriptori */
    int nfds;          /* numarul maxim de descriptori */
    int len;           /* lungimea structurii sockaddr_in */

    char paths[10][100]; //path-urile pentru fiecare client
    char status[10][1];  //0 = nu e logat | 1 = s-a logat | 2 = este pe blacklist
    char mesaj[200];
    bool authenticated[10]; //1 = s-a dorit autentificarea | 0 = s-a dorit deconectarea

    /* creare socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server] Eroare la socket().\n");
        return errno;
    }

    /*setam pentru socket optiunea SO_REUSEADDR */
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    /* pregatim structurile de date */
    bzero(&server, sizeof(server));

    /* umplem structura folosita de server */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    /* atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server] Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 5) == -1)
    {
        perror("[server] Eroare la listen().\n");
        return errno;
    }

    /* completam multimea de descriptori de citire */
    FD_ZERO(&actfds);    /* initial, multimea este vida */
    FD_SET(sd, &actfds); /* includem in multime socketul creat */

    tv.tv_sec = 1; /* se va astepta un timp de 1 sec. */
    tv.tv_usec = 0;

    /* valoarea maxima a descriptorilor folositi */
    nfds = sd;

    printf("[server] Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    /* servim in mod concurent clientii... */
    while (1)
    {
        /* ajustam multimea descriptorilor activi (efectiv utilizati) */
        bcopy((char *)&actfds, (char *)&readfds, sizeof(readfds));

        /* apelul select() */
        if (select(nfds + 1, &readfds, NULL, NULL, &tv) < 0)
        {
            perror("[server] Eroare la select().\n");
            return errno;
        }

        /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
        if (FD_ISSET(sd, &readfds))
        {
            /* pregatirea structurii client */
            len = sizeof(from);
            bzero(&from, sizeof(from));

            /* a venit un client, acceptam conexiunea */
            client = accept(sd, (struct sockaddr *)&from, &len);
            strcpy(paths[client], "~");
            status[client][0] = '0';
            authenticated[client] = false;

            /* eroare la acceptarea conexiunii de la un client */
            if (client < 0)
            {
                perror("[server] Eroare la accept().\n");
                continue;
            }

            if (nfds < client) /* ajusteaza valoarea maximului */
                nfds = client;

            /* includem in lista de descriptori activi si acest socket */
            FD_SET(client, &actfds);

            printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
            fflush(stdout);
        }
        /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
        for (fd = 0; fd <= nfds; fd++) /* parcurgem multimea de descriptori */
        {
            /* este un socket de citire pregatit? */
            if (fd != sd && FD_ISSET(fd, &readfds))
            {
                if (status[fd][0] == '0' && authenticated[fd] == false) // Daca se cere deconectarea
                {
                    if (read(fd, mesaj, sizeof(mesaj)) < 0)
                    {
                        perror("[server] Eroare la citire pass.\n");
                        return errno;
                    }
                    if (strcmp(mesaj, "Exit") == 0)
                    {
                        status[fd][0] = '2';
                        goto disconnect;
                    }
                    authenticated[fd] = true;
                }
                if (status[fd][0] == '0') //Daca se cere autentificarea
                {
                    char user[100], pass[100];
                    bzero(user, 100);
                    bzero(pass, 100);
                    if (read(fd, user, sizeof(user)) < 0)
                    {
                        perror("[server] Eroare la citire user.\n");
                        return errno;
                    }
                    if (read(fd, pass, sizeof(pass)) < 0)
                    {
                        perror("[server] Eroare la citire pass.\n");
                        return errno;
                    }
                    char val = getInfo(user, pass);
                    if (write(fd, &val, sizeof val) < 0)
                    {
                        perror("[server] Eroare la scriere rezultat.\n");
                        return errno;
                    }
                    if (val == '1')
                    {
                        status[fd][0] = '1';
                        printf("[server] Utilizatorul %d s-a autentificat cu succes!\n", fd);
                    }
                }
                else if (receiveAndSend(fd, paths[fd]) <= 0)
                {
                disconnect:
                    printf("[server] S-a deconectat clientul cu descriptorul %d.\n", fd);
                    fflush(stdout);
                    close(fd);
                    FD_CLR(fd, &actfds);
                }
            }
        } /* for */
    }     /* while */
} /* main */