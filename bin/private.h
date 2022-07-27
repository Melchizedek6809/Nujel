#ifndef NUJEL_BIN_PRIVATE
#define NUJEL_BIN_PRIVATE

#ifndef NUJEL_AMALGAMATION
#include "../lib/nujel.h"
#endif

#include <stdio.h>

#ifdef __WATCOMC__
	#include  <io.h>
#elif defined(_MSC_VER)
	#include  <io.h>
	#define access(path,mode) _access(path,mode)
#else
	#include <unistd.h>
#endif

extern lSymbol *lSymError;
extern lSymbol *lSymReplace;
extern lSymbol *lSymTruncate;
extern lSymbol *lSymMustTruncat;
extern lSymbol *lSymTruncateReplace;
extern lSymbol *lSymUpdate;
extern lSymbol *lSymCanUpdate;
extern lSymbol *lSymAppend;

void initEnvironmentMap(lClosure *c);
void setIOSymbols();
void lOperationsIO(lClosure *c);
void lOperationsPort(lClosure *c);
void lOperationsInit(lClosure *c);

int  makeDir   (const char *name);
ssize_t getdelim(char **buf, size_t *bufsiz, int delimiter, FILE *fp);
ssize_t getline(char **buf, size_t *bufsiz, FILE *fp);

#endif
