#pragma once
#include "../nujel.h"

struct lString{
	const char *buf,*bufEnd;
	union {
		const char *data;
		lString *nextFree;
	};
	u16 flags;
};
#define STR_MAX  (1<<14)

extern lString  lStringList [STR_MAX];
extern uint     lStringActive;
extern uint     lStringMax;
extern lString *lStringFFree;

void     lInitStr      ();
lString *lStringAlloc  ();
void     lStringFree   (lString *s);
lString *lStringNew    (const char *str, uint len);
lString *lStringDup    (lString *s);
int      lStringLength (const lString *s);
lVal    *lValString    (const char *s);