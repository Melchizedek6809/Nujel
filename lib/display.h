#ifndef NUJEL_LIB_COMMON
#define NUJEL_LIB_COMMON

#include "common.h"

extern char dispWriteBuf[1<<18];

void        lPrintError       (const char *format, ...);
void        lDisplayVal       (lVal *v);
const char *lReturnDisplayVal (lVal *v);
void        lDisplayErrorVal  (lVal *v);
void        lWriteVal         (lVal *v);
void        lWriteTree        (lTree *t);

#endif
