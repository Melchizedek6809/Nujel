/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Checks if A is greater than B, returns 0 if the two values can't be compared
 | or if they are equal.
 */
i64 lValGreater(const lVal a, const lVal b) {
    if (unlikely(a.type != b.type)) {
        if ((a.type == ltInt) && (b.type == ltFloat)) {
            return (((float)a.vInt) < b.vFloat) ? -1 : (((float)a.vInt) > b.vFloat) ? 1 : 0;
        } else if ((a.type == ltFloat) && (b.type == ltInt)) {
            return (a.vFloat < ((float)b.vInt)) ? -1 : (a.vFloat > ((float)b.vInt)) ? 1 : 0;
        }
        return 0;
    }
    switch (a.type) {
    default:
        return 0;
    case ltKeyword:
    case ltSymbol: {
        const uint alen = strnlen(a.vSymbol->c, sizeof(a.vSymbol->c));
        const uint blen = strnlen(b.vSymbol->c, sizeof(b.vSymbol->c));
        const uint len = MIN(alen, blen);
        const char *ab = a.vSymbol->c;
        const char *bb = b.vSymbol->c;
        for (uint i = 0; i < len; i++) {
            const u8 ac = *ab++;
            const u8 bc = *bb++;
            if (ac != bc) {
                return ac - bc;
            }
        }
        return alen - blen;
    }
    case ltInt:
        return a.vInt - b.vInt;
    case ltFloat:
        return a.vFloat < b.vFloat ? -1 : 1;
    case ltString: {
        const uint alen = lBufferLength(a.vString);
        const uint blen = lBufferLength(b.vString);
        const uint len = MIN(alen, blen);
        const char *ab = a.vString->data;
        const char *bb = b.vString->data;
        for (uint i = 0; i < len; i++) {
            const u8 ac = *ab++;
            const u8 bc = *bb++;
            if (ac != bc) {
                return ac - bc;
            }
        }
        return alen - blen;
    }
    }
}

/* Check two values for equality */
bool lValEqual(const lVal a, const lVal b) {
    if (unlikely(a.type != b.type)) {
        if ((a.type == ltInt) && (b.type == ltFloat)) {
            return ((float)a.vInt) == b.vFloat;
        } else if ((a.type == ltFloat) && (b.type == ltInt)) {
            return a.vFloat == ((float)b.vInt);
        }
        return false;
    } else if (unlikely(a.type == ltString)) {
        const uint alen = lBufferLength(a.vString);
        const uint blen = lBufferLength(b.vString);
        return (alen == blen) && (strncmp(a.vString->data, b.vString->data, alen) == 0);
    }
#ifdef __wasm__
    else if (unlikely(a.type == ltBool)) {
        return a.vBool == b.vBool;
    }
#endif
    return a.vPointer == b.vPointer;
}
