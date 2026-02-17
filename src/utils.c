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