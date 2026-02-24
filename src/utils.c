#include "utils.h"

void strcpy_s(char *dest, const char *source, size_t size) {

    while (*source != '\0' && size > 1) {
        *dest = *source;

        size = size - 1;

        source++;
        dest++;
    }
    *dest = '\0';
}

char upper_char(char c) {
    if ('A' <= c && c <= 'Z')
        return c;
    else if ('a' <= c && c <= 'z')
        return c + 'A' - 'a';
    else 
        return c;
}

char lower_char(char c) {
    if ('a' <= c && c <= 'z')
        return c;
    else if ('A' <= c && c <= 'Z')
        return c + 'a' - 'A';
    else 
        return c;
}

int strprefix(char *s, char *pref) {
    int res;

    while (*s != '\0' &&
           *pref != '\0' &&
            lower_char(*s) == lower_char(*pref)) {
        s++;
        pref++;
    }

    if (*pref == '\0') {
        res = 1;
    } else {
        res = 0;
    }
    return res;
}