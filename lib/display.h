#pragma once
#include "common.h"

void        lPrintError       (const char *format, ...);
void        lDisplayVal       (lVal *v);
const char *lReturnDisplayVal (lVal *v);
void        lDisplayErrorVal  (lVal *v);
void        lWriteVal         (lVal *v);
void        lWriteTree        (lTree *t);
