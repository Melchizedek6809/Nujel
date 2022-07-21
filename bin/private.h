#ifndef NUJEL_BIN_PRIVATE
#define NUJEL_BIN_PRIVATE

#ifndef NUJEL_AMALGAMATION
#include "../lib/nujel.h"
#endif

#include <stdio.h>

void initEnvironmentMap(lClosure *c);
void setIOSymbols();
void lOperationsIO(lClosure *c);
void lOperationsReadline(lClosure *c);
void lOperationsPort(lClosure *c);
void lOperationsInit(lClosure *c);

int  makeDir   (const char *name);
const char *tempFilename();

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#endif
