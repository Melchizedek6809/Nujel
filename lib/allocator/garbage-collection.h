#pragma once
#include "../nujel.h"

extern int lGCRuns;

void lValGCMark    (lVal *v);
void lClosureGCMark(const lClosure *c);
void lArrayGCMark  (lArray *v);
void lNFuncGCMark  (lNFunc *f);

void lGarbageCollect();
void lGarbageCollectForce();
