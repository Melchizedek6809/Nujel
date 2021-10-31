/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "time.h"
#include "../type-system.h"
#include "../collection/list.h"
#include "../collection/string.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <time.h>
#include <sys/time.h>

u64 getMSecs(){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return (tv.tv_nsec / 1000000) + (tv.tv_sec * 1000);
}

static lVal *lnfTime(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(time(NULL));
}

static lVal *lnfTimeMsecs(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(getMSecs());
}

static lVal *lnfStrftime(lClosure *c, lVal *v){
	(void)c;
	const int timestamp = castToInt(lCar(v),time(NULL));
	const char *format = castToString(lCadr(v),"%Y-%m-%d %H:%M:%S");

	char buf[4096];
	time_t ts = timestamp;
	struct tm *info = localtime(&ts);
	strftime(buf,sizeof(buf),format,info);

	return lValString(buf);
}

void lOperationsTime(lClosure *c){
	lAddNativeFunc(c,"time",             "[]",         "Returns unix time",lnfTime);
	lAddNativeFunc(c,"strftime",         "[ts format]","Returns TS as a date using FORMAT (uses strftime)",lnfStrftime);
	lAddNativeFunc(c,"time/milliseconds","[]",         "Returns monotonic msecs",lnfTimeMsecs);
}
