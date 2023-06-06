/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#ifndef _MSC_VER
#include <unistd.h>
#endif

/* Add environment key/value pair to tree T */
static lTree *addVar(const char *e, lTree *t) {
    int endOfKey = 0;
    while (e[endOfKey] != '=') {
        endOfKey++;
    }
    int endOfString = endOfKey + 1;
    while (e[endOfString]) {
        endOfString++;
    }
    lSymbol *sym = lSymSL(e, endOfKey);
    lVal v = lValStringNoCopy(&e[endOfKey + 1], endOfString);
    return lTreeInsert(t, sym, v);
}

#if (defined(__MSYS__)) || (defined(__MINGW32__)) || (defined(_WIN32))
#include <windows.h>

/* Windows specific - add Environment args to `environment/variables` */
void initEnvironmentMap(lClosure *c) {
    lTree *t = NULL;
    LPCH env = GetEnvironmentStrings();
    while (*env) {
        t = addVar(env, t);
        while (*env++) {
        }
    }
    lDefineClosureSym(c, lSymS("System/Environment"), lValTree(t));
}

#else
extern char **environ;
/* Add Environment args to `environment/variables` */
void initEnvironmentMap(lClosure *c) {
    lTree *t = NULL;
#ifdef __wasi__
    t = addVar("PATH=", t); // Necessary so that tests don't fail
#endif
    for (int i = 0; environ[i]; i++) {
        t = addVar(environ[i], t);
    }
    lDefineClosureSym(c, lSymS("System/Environment"), lValTree(t));
}
#endif
