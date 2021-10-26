#pragma once
#include "../nujel.h"

char *lSWriteTree(lTree *v, char *buf, char *bufEnd, int indentLevel, bool display);
char *lSWriteVal (lVal  *v, char *buf, char *bufEnd, int indentLevel, bool display);
