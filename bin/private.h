#ifndef NUJEL_BIN_PRIVATE
#define NUJEL_BIN_PRIVATE

#ifndef NUJEL_AMALGAMATION
#include "../lib/nujel.h"
#endif

#if (!defined(_WIN32)) && (!defined(__wasi__))
#include <termios.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
	#include  <io.h>
	#define access(path,mode) _access(path,mode)
#else
	#include <unistd.h>
#endif

extern lSymbol *lSymError;
extern lSymbol *lSymReplace;
extern lSymbol *lSymAppend;

void lRedefineEnvironment(lClosure *c);
void lRedefineFileHandles(lClosure *c);

void setIOSymbols();
void lOperationsIO   ();
void lOperationsPort ();
void lOperationsNet  ();
void *loadFile(const char *filename, size_t *len);

int  makeDir   (const char *name);

#endif
