#ifndef NUJEL_LIB_ALLOC_MARKER
#define NUJEL_LIB_ALLOC_MARKER
#include "../nujel.h"

void lSetStartOfStack(void *p);
void lGCMarkStack    (void *endOfStack);

#endif
