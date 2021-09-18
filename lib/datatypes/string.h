#pragma once
#include "../nujel.h"

typedef struct {
	const char *buf,*data,*bufEnd;
	u16 nextFree;
	u16 flags;
} lString;
#define lfHeapAlloc (16)

#define STR_MAX  (1<<14)
#define STR_MASK ((STR_MAX)-1)

extern lString  lStringList [STR_MAX];
extern uint     lStringActive;
extern uint     lStringMax;

#define lStrNull(val)  (((val->vCdr & STR_MASK) == 0) || (lStringList[val->vCdr & STR_MASK].data == NULL))
#define lStr(val)      lStringList[val->vCdr & STR_MASK]
#define lStrData(val)  lStr(val).data
#define lStrBuf(val)   lStr(val).buf
#define lStrEnd(val)   lStr(val).bufEnd
#define lStrFlags(val) lStr(val).flags

void  lInitStr      ();
uint  lStringAlloc  ();
void  lStringFree   (uint v);
uint  lStringNew    (const char *str, uint len);
uint  lStringDup    (uint i);
int   lStringLength (const lString *s);
lVal *lValString    (const char *s);
char *lSWriteVal  (lVal *v, char *buf, char *bufEnd, int indentLevel, bool display);
