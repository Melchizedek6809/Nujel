/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "random.h"
#include "../random-number-generator.h"
#include "../datatypes/native-function.h"

static lVal *lnfRandom(lClosure *c, lVal *v){
	int n = 0;
	v = getLArgI(c,v,&n);
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
	if(v != NULL){
		int n = 0;
		v = getLArgI(c,v,&n);
		seedRNG(n);
	}
	return NULL;
}

void lOperationsRandom(lClosure *c){
	lAddNativeFunc(c,"random",      "[&max]", "Return a random value from 0 to MAX ot INT_MAX if MAX is #nil",lnfRandom);
	lAddNativeFunc(c,"random-seed", "[]",     "Get the RNG Seed",lnfRandomSeedGet);
	lAddNativeFunc(c,"random-seed!","[seed]", "Set the RNG Seed to SEED",lnfRandomSeedSet);
}
