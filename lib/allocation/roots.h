#pragma once
#include "../nujel.h"

lClosure *lRootsClosurePush(lClosure *c);
lClosure *lRootsClosurePop ();
void      lRootsClosureMark();

lVal *lRootsValPush(lVal *c);
lVal *lRootsValPop ();
void  lRootsValMark();

lString *lRootsStringPush(lString *s);
lString *lRootsStringPop ();
void     lRootsStringMark();

extern uint rootsClosureSP;
extern uint rootsValSP;
extern uint rootsStringSP;
