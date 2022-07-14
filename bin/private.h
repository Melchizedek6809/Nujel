#ifndef NUJEL_BIN_PRIVATE
#define NUJEL_BIN_PRIVATE

#ifndef NUJEL_AMALGAMATION
#include "../lib/nujel.h"
#endif

void initEnvironmentMap(lClosure *c);
void lOperationsIO(lClosure *c);
void setIOSymbols();
void lOperationsReadline(lClosure *c);

void *loadFile (const char *filename, size_t *len);
void  saveFile (const char *filename, const void *buf, size_t len);

int  makeDir   (const char *name);
const char *tempFilename();

ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#endif
