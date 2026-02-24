#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>

void strcpy_s(char *dest, const char *source, size_t size);

char upper_char(char c);

char lower_char(char c);

int strprefix(char *s, char *pref);

#endif /* _UTILS_H_ */