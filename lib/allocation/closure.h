#ifndef NUJEL_LIB_ALLOC_CLOSURE
#define NUJEL_LIB_ALLOC_CLOSURE
#include "../nujel.h"

#define CLO_MAX (1<<16)
extern lClosure lClosureList[CLO_MAX];
extern uint     lClosureMax;
extern uint     lClosureActive;
extern lClosure *lClosureFFree;

void      lClosureInit      ();
lClosure *lClosureAlloc     ();
void      lClosureFree      (lClosure *c);
int       lClosureID        (const lClosure *n);

#endif
