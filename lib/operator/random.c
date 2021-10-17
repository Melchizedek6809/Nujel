/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "random.h"
#include "../misc/random-number-generator.h"
#include "../type-system.h"
#include "../types/list.h"
#include "../types/native-function.h"
#include "../types/val.h"

static lVal *lnfRandom(lClosure *c, lVal *v){
	(void)c;
	const int n = castToInt(lCar(v),0);
	if(n == 0){
		return lValInt(rngValR());
	}else{
		return lValInt(rngValR() % n);
	}
}

static lVal *lnfRandomSeedGet(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(RNGValue);
}

static lVal *lnfRandomSeedSet(lClosure *c, lVal *v){
	(void)c;
	seedRNG(castToInt(lCar(v),0));
	return NULL;
}

void lOperationsRandom(lClosure *c){
	lAddNativeFunc(c,"random",      "[&max]", "Return a random value from 0 to MAX ot INT_MAX if MAX is #nil",lnfRandom);
	lAddNativeFunc(c,"random-seed", "[]",     "Get the RNG Seed",lnfRandomSeedGet);
	lAddNativeFunc(c,"random-seed!","[seed]", "Set the RNG Seed to SEED",lnfRandomSeedSet);
}
