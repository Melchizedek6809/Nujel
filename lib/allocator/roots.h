#pragma once
#include "../nujel.h"

void lRootsClosurePush(const lClosure *c);
void lRootsClosurePop ();
void lRootsClosureMark();

void lRootsValPush(lVal *c);
void lRootsValPop ();
void lRootsValMark();
