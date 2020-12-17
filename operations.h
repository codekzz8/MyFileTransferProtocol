#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

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