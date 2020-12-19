#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>

void listdir(char path[], int space, char **rezultat, int lineNr) //lineNr este initial 0
{
    DIR *dir;
    struct dirent *dir_curent;

    if (!(dir = opendir(path)))
        return;

    while ((dir_curent = readdir(dir)) != NULL)
    {
        if (dir_curent->d_type == DT_DIR)
        {
            char functionPath[1000];
            if (strcmp(dir_curent->d_name, ".") == 0 || strcmp(dir_curent->d_name, "..") == 0)
                continue;
            snprintf(functionPath, sizeof(functionPath), "%s/%s", path, dir_curent->d_name);
            lineNr++;
            sprintf(rezultat[lineNr], "    - [%s]", dir_curent->d_name); //Director
        }
        else
        {
            lineNr++;
            sprintf(rezultat[lineNr], "    - %s", dir_curent->d_name); //Fisier
        }
    }
    closedir(dir);
    sprintf(rezultat[0], "%d", lineNr);
}

void mystat(const char *fileName, char **result)
{
    int r = 50, c = 200, i, j;
    //result = malloc(r * sizeof(char *));
    //for(i = 0; i < r; i++)
    //result[i] = malloc(c * sizeof(char));

    //Daca fisierul nu exista
    if (access(fileName, F_OK) == -1)
    {
        strcpy(result[0], "1");
        snprintf(result[1], 200, "    Fisierul cu numele '%s' nu exista!", fileName);
        //snprintf(result[1], 200, "[Lungime raspuns: %d]", strlen(result[2]));
        return;
    }

    struct stat file_stats;
    stat(fileName, &file_stats);
    strcpy(result[0], "9"); //numarul de linii al rezultatului

    strcpy(result[1], "    Tipul fisierului: ");
    switch (file_stats.st_mode & S_IFMT)
    {
    case S_IFBLK:
        strcat(result[1], "Block special");
        break;
    case S_IFCHR:
        strcat(result[1], "Character special");
        break;
    case S_IFIFO:
        strcat(result[1], "FIFO special");
        break;
    case S_IFREG:
        strcat(result[1], "Regular");
        break;
    case S_IFDIR:
        strcat(result[1], "Directory");
        break;
    case S_IFLNK:
        strcat(result[1], "Symbolic link");
        break;
    case S_IFSOCK:
        strcat(result[1], "Socket");
        break;
    }

    snprintf(result[3], 200, "    Numarul de link-uri hard ale fisierului: %ld", file_stats.st_nlink);
    snprintf(result[2], 200, "    Dimensiunea fisierului: %d octeti", (int)file_stats.st_size);
    snprintf(result[4], 200, "    Numarul de block-uri alocate fisierului: %d", (int)file_stats.st_blocks);

    //PERMISIUNI
    strcpy(result[5], "    File permissions:");
    strcpy(result[6], (S_ISDIR(file_stats.st_mode)) ? "    d" : "    -");
    strcat(result[6], (file_stats.st_mode & S_IRUSR) ? "r" : "-");
    strcat(result[6], (file_stats.st_mode & S_IWUSR) ? "w" : "-");
    strcat(result[6], (file_stats.st_mode & S_IXUSR) ? "x" : "-");
    strcat(result[6], (file_stats.st_mode & S_IRGRP) ? "r" : "-");
    strcat(result[6], (file_stats.st_mode & S_IWGRP) ? "w" : "-");
    strcat(result[6], (file_stats.st_mode & S_IXGRP) ? "x" : "-");
    strcat(result[6], (file_stats.st_mode & S_IROTH) ? "r" : "-");
    strcat(result[6], (file_stats.st_mode & S_IWOTH) ? "w" : "-");
    strcat(result[6], (file_stats.st_mode & S_IXOTH) ? "x" : "-");

    //DATE
    snprintf(result[7], 200, "    Data ultimei accesari: %s", ctime(&file_stats.st_atime));
    snprintf(result[8], 200, "    Data ultimei modificari: %s", ctime(&file_stats.st_mtime));
    snprintf(result[9], 200, "    Data ultimei actualizari a statusului: %s", ctime(&file_stats.st_ctime));
}

void findDir(const char *start, const char *dirName, char **result)
{
    DIR *dir;
    struct dirent *dir_curent;
    if (!(dir = opendir(start)))
        return;
    strcpy(result[0], "1");
    strcpy(result[1], "    Directorul cu numele '");
    strcat(result[1], dirName);
    strcat(result[1], "' nu a putut fi gasit!");

    while ((dir_curent = readdir(dir)) != NULL)
    {
        char path[300];
        if (strcmp(dir_curent->d_name, ".") && strcmp(dir_curent->d_name, ".."))
        {
            if (strcmp(dir_curent->d_name, dirName))
            {
                sprintf(path, "%s/%s", start, dir_curent->d_name);
                findDir(path, dirName, result);
            }
            else
            {
                strcpy(result[0], "1");
                strcpy(result[1], "Director gasit");
                return;
            }
        }
    }
    closedir(dir);
}

void myfind(const char *start, const char *fileName, char **result)
{
    char **fileInfo = malloc(50 * sizeof(char *));
    int i;
    for (i = 0; i < 50; i++)
        fileInfo[i] = malloc(200 * sizeof(char));
    DIR *dir;
    struct dirent *dir_curent;
    if (!(dir = opendir(start)))
        return;

    strcpy(result[0], "1");
    strcpy(result[1], "    Fisierul cu numele '");
    strcat(result[1], fileName);
    strcat(result[1], "' nu a putut fi gasit!");

    while ((dir_curent = readdir(dir)) != NULL)
    {
        if (dir_curent->d_type == DT_DIR)
        {
            char path[300];
            if (strcmp(dir_curent->d_name, ".") && strcmp(dir_curent->d_name, ".."))
            {
                //snprintf(path, sizeof(path), "%s/%s", start, dir_curent->d_name);
                sprintf(path, "%s/%s", start, dir_curent->d_name);
                myfind(path, fileName, result);
            }
        }
        else
        {
            if (strcmp(dir_curent->d_name, fileName) == 0)
            {
                char filePath[1000];
                snprintf(filePath, 1000, "%s/%s", start, fileName);
                mystat(filePath, fileInfo);
                strcpy(result[0], "12");
                strcpy(result[1], "    Fisierul cu numele '");
                strcat(result[1], fileName);
                strcat(result[1], "' a fost gasit!");
                strcpy(result[2], "    Acesta are path-ul : ");
                strcat(result[2], start);
                strcpy(result[3], " ");
                strcpy(result[4], fileInfo[1]);
                strcpy(result[5], fileInfo[2]);
                strcpy(result[6], fileInfo[3]);
                strcpy(result[7], fileInfo[4]);
                strcpy(result[8], fileInfo[5]);
                strcpy(result[9], fileInfo[6]);
                strcpy(result[10], fileInfo[7]);
                strcpy(result[11], fileInfo[8]);
                strcpy(result[12], fileInfo[9]);
                return;
            }
        }
    }
    closedir(dir);
}

int fsize(const char *filename)
{
    struct stat st;

    if (stat(filename, &st) == 0)
        return (int)st.st_size;

    return -1;
}

int fpermissions(const char *filename)
{
    struct stat file_stats;

    struct stat st;
    stat(filename, &st);
    int perm = 0;

    if (st.st_mode & S_IXOTH)
        perm += 1;
    if (st.st_mode & S_IWOTH)
        perm += 2;
    if (st.st_mode & S_IROTH)
        perm += 4;

    if (st.st_mode & S_IXGRP)
        perm = 10 + perm;
    if (st.st_mode & S_IWGRP)
        perm = 20 + perm;
    if (st.st_mode & S_IRGRP)
        perm = 40 + perm;

    if (st.st_mode & S_IXUSR)
        perm = 100 + perm;
    if (st.st_mode & S_IWUSR)
        perm = 200 + perm;
    if (st.st_mode & S_IRUSR)
        perm = 400 + perm;

    return perm;
}

void sendfile(const char *fileName, int to)
{
    char buff[8192];

    FILE *input;
    input = fopen(fileName, "rb");
    if (input == NULL)
    {
        perror("Eroare la deschidere fisier.\n");
        return;
    }

    size_t read_bytes, write_bytes;
    int nrBlocks = fsize(fileName);
    if (nrBlocks % 8192 != 0)
        nrBlocks = nrBlocks / 8192 + 1;
    else
        nrBlocks = nrBlocks / 8192;

    sprintf(buff, "%d", nrBlocks);
    if (write(to, buff, sizeof(buff)) < 0)
    {
        return;
    }
    bzero(buff, sizeof(buff));

    int i;
    for (i = 0; i < nrBlocks; i++)
    {
        read_bytes = fread(buff, 1, sizeof(buff), input);
        if (read_bytes < 0)
        {
            perror("Eroare la citire din fisier.\n");
            return;
        }
        write_bytes = write(to, buff, read_bytes);
        if (write_bytes < 0)
        {
            perror("Eroare la scriere catre client.\n");
            return;
        }
        bzero(buff, sizeof(buff));
    }
    fclose(input);

    sprintf(buff, "%d", fpermissions(fileName));
    write_bytes = write(to, buff, sizeof(buff));
    if (write_bytes < 0)
    {
        perror("Eroare la scriere catre client.\n");
        return;
    }
}