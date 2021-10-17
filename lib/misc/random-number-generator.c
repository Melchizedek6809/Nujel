/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "random-number-generator.h"

u64 RNGValue;

void seedRNG(u64 seed){
	RNGValue = seed;
}

u64 getRNGSeed(){
	return RNGValue;
}

u64 rngValR(){
	RNGValue = ((RNGValue * 1103515245)) + 12345;
	return ((RNGValue&0xFFFF)<<16) | ((RNGValue>>16)&0xFFFF);
}

float rngValf(){
	return (float)rngValR() / ((float)0xffffffff);
}

u64 rngValA(u64 mask){
	return rngValR() & mask;
}

u64 rngValM(u64 max){
	return rngValR() % max;
}

i64 rngValMM(i64 min,i64 max){
	return rngValM(max - min) + min;
}
