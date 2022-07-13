#ifndef NUJEL_BIN_PRIVATE
#define NUJEL_BIN_PRIVATE

#include "../nujel.h"


void initEnvironmentMap(lClosure *c);
void lOperationsIO(lClosure *c);
void setIOSymbols();
void lOperationsReadline(lClosure *c);

void *loadFile (const char *filename, size_t *len);
void  saveFile (const char *filename, const void *buf, size_t len);

int  makeDir   (const char *name);
const char *tempFilename();

#endif
