/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

static lVal lnfArrLength(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireArray(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lArray *arr = car.vArray;
    return lValInt(arr->length);
}

static lVal lnfArrLengthSet(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireArray(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lArray *arr = car.vArray;
    lVal lenVal = requireNaturalInt(lCadr(v));
    if (unlikely(lenVal.type == ltException)) {
        return lenVal;
    }
    const int length = lenVal.vInt;
    lVal *newData = realloc(arr->data, length * sizeof(lVal));
    if (unlikely(newData == NULL)) {
        free(newData);
        return lValException("out-of-memory", "(array/allocate) couldn't allocate its array", v);
    }
    arr->data = newData;
    if (length > arr->length) {
        memset(&arr->data[arr->length], 0, (((size_t)length) - arr->length) * sizeof(lVal *));
    }
    arr->length = length;
    return car;
}

static lVal lnfArrSet(lClosure *c, lVal v) {
    (void)c;
    lVal car = requireMutableArray(lCar(v));
    if (unlikely(car.type == ltException)) {
        return car;
    }
    lArray *arr = car.vArray;
    lVal keyV = requireInt(lCadr(v));
    if (unlikely(keyV.type == ltException)) {
        return keyV;
    }
    const int key = keyV.vInt;
    if ((key < 0) || (key >= arr->length)) {
        return lValException("out-of-bounds", "(array/set!) index provided is out of bounds", v);
    }
    const lVal vt = lCddr(v);
    if (vt.type != ltPair) {
        return lValException("type-mismatch", "(array/set!) needs a third argument", v);
    }
    arr->data[key] = vt.vList->car;
    return car;
}

static lVal lnfArrAllocate(lClosure *c, lVal v) {
    (void)c;
    lVal lenV = requireNaturalInt(lCar(v));
    if (unlikely(lenV.type == ltException)) {
        return lenV;
    }
    const int len = lenV.vInt;
    lVal r = lValAlloc(ltArray, lArrayAlloc(len));
    if (unlikely(len && (r.vArray->data == NULL))) {
        return lValException("out-of-memory", "(array/allocate] couldn't allocate its array", v);
    }
    return r;
}

/* Return the length of the list V */
static int lListLength(lVal v) {
    int i = 0;
    for (lVal n = v; (n.type == ltPair) && (n.vList->car.type != ltNil); n = n.vList->cdr) {
        i++;
    }
    return i;
}

lVal lnfArrNew(lClosure *c, lVal v) {
    (void)c;
    int length = lListLength(v);
    lVal r = lValAlloc(ltArray, lArrayAlloc(length));
    int key = 0;
    for (lVal n = v; n.type == ltPair; n = n.vList->cdr) {
        if (unlikely(key >= length)) {
            break;
        }
        r.vArray->data[key++] = n.vList->car;
    }
    return r;
}

void lOperationsArray(lClosure *c) {
    lAddNativeFunc(c, "array/new", "args", "Create a new array from ...ARGS", lnfArrNew);
    lAddNativeFunc(c, "array/allocate", "(size)", "Allocate a new array of SIZE", lnfArrAllocate);
    lAddNativeFunc(c, "array/length", "(array)", "Return length of ARRAY", lnfArrLength);
    lAddNativeFunc(c, "array/length!", "(array size)", "Set a new LENGTH for ARRAY", lnfArrLengthSet);
    lAddNativeFunc(c, "array/set!", "(array index value)", "Set ARRAY at INDEX to &...VALUES", lnfArrSet);
}
