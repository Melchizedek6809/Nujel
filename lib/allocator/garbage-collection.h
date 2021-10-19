#pragma once
#include "../nujel.h"

extern int lGCRuns;

void lValGCMark    (lVal *v);
void lClosureGCMark(const lClosure *c);
void lStringGCMark (const lString *v);
void lArrayGCMark  (const lArray *v);
void lNFuncGCMark  (const lNFunc *f);

void lGarbageCollect();
void lGarbageCollectForce();
