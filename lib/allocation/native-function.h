#ifndef NUJEL_LIB_ALLOC_NFUNC
#define NUJEL_LIB_ALLOC_NFUNC
#include "../nujel.h"

#define NFN_MAX (1<<10)
extern lNFunc   lNFuncList  [NFN_MAX];
extern uint     lNFuncMax;
extern uint     lNFuncActive;

void      lNativeFunctionsInit();
void      lNFuncFree          (uint i);
lNFunc   *lNFuncAlloc         ();
int       lNFuncID            (const lNFunc *n);

#endif
