#ifndef NUJEL_LIB_ALLOC_MARKER
#define NUJEL_LIB_ALLOC_MARKER
#include "../nujel.h"

extern void *checkForVal;

void lSetStartOfStack(void *p);
void lGCMarkStack    (void *endOfStack);

#endif
