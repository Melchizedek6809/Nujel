#ifndef NUJEL_LIB_COLLECTION_STRING
#define NUJEL_LIB_COLLECTION_STRING
#include "../nujel.h"

lString *lStringNew       (const char *str, uint len);
lString *lStringDup       (      lString *s);
int      lStringLength    (const lString *s);
lVal    *lValString       (const char *s);
lVal    *lValStringLen    (const char *s, int len);
lVal    *lValStringNoCopy (const char *s, int len);
lVal    *lValStringError  (const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd);

#endif
