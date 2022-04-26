// strtok/strtok_r
// strsep
// strlen
// strcpy
// atoi
// strpbrk
// strcmp

#include "string.h"

int strcmp(const char *s1, const char *s2)
{
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0')
    {
        if (s1[i] != s2[i])
        {
            return s1[i] - s2[i];
        }
        i++;
    }
    if (s1[i] == '\0' && s2[i] == '\0')
    {
        return 0;
    }
    else if (s1[i] == '\0')
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

char *strcpy(char *restrict dest, const char *src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';

    return dest;
}

int strlen(const char *str)
{
    int i = 0;
    while (str[i] != '\0')
    {
        i++;
    }
    return i - 1;
}