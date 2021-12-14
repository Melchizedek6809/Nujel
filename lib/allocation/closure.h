#pragma once
#include "../nujel.h"

#define CLO_MAX (1<<18)
extern lClosure lClosureList[CLO_MAX];
extern uint     lClosureMax;
extern uint     lClosureActive;
extern lClosure *lClosureFFree;

void      lClosureInit      ();
lClosure *lClosureAlloc     ();
void      lClosureFree      (lClosure *c);
int       lClosureID        (const lClosure *n);
