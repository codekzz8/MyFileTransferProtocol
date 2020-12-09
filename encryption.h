#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void secure(char string[])
{
    int key = 37;
    int i;
    for (i = 0; string[i]; i++)
    {
        string[i] ^= key;
    }
}