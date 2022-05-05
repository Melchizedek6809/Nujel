#ifndef NUJEL_LIB_COMMON
#define NUJEL_LIB_COMMON

#include "common.h"

extern char dispWriteBuf[1<<16];

void        lPrintError       (const char *format, ...);
void        lDisplayErrorVal  (lVal *v);
void        lWriteVal         (lVal *v);

#endif
