/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#include <stdio.h>
#include <stdlib.h>

extern u8 stdlib_no_data[];
extern u8 binlib_no_data[];
lClosure *mainClosure;

/* Return a new root Closure, with all native functions in place */
static lClosure *createRootClosure() {
    lClosure *c = lNewRoot();
    lOperationsIO(c);
    lOperationsPort(c);
    lOperationsInit(c);
    mainClosure = lLoad(c, (const char *)binlib_no_data);
    return c;
}

/* Initialize the Nujel context with an stdlib as well
 * as parsing arguments passed to the runtime */
int initNujel(int argc, char *argv[], lClosure *c) {
    lVal volatile ret = NIL;
    initEnvironmentMap(c);
    for (int i = argc - 1; i >= 0; i--) {
        ret = lCons(lValString(argv[i]), ret);
    }
    lLambda(c, ret, lGetClosureSym(c, lSymS("init")));
    return 0;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    // printf("%s",stdlib_no_data); return 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    lInit();
    setIOSymbols();

    return initNujel(argc, argv, createRootClosure());
}
