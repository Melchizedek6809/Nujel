#ifndef NUJEL_VENDOR_GETLINE
#define NUJEL_VENDOR_GETLINE
#include <stdio.h>
#include <stdint.h>

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#endif
