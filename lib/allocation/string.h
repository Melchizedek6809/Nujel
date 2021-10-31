#pragma once
#include "../nujel.h"

#define STR_MAX  (1<<14)
extern lString  lStringList [STR_MAX];
extern uint     lStringActive;
extern uint     lStringMax;
extern lString *lStringFFree;

void     lStringInit   ();
lString *lStringAlloc  ();
void     lStringFree   (lString *s);
