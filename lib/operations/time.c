/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "time.h"
#include "../casting.h"
#include "../datatypes/string.h"

#include <time.h>
#include <sys/time.h>

static u64 getMSecs(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
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
	int timestamp = 0;
	const char *format = "%Y-%m-%d %H:%M:%S";

	v = getLArgI(c,v,&timestamp);
	v = getLArgS(c,v,&format);

	char buf[1024];
	time_t ts = timestamp;
	struct tm *info = localtime(&ts);
	strftime(buf,sizeof(buf),format,info);

	return lValString(buf);
}

void lOperationsTime(lClosure *c){
	lAddNativeFunc(c,"time",             "[]",         "Returns unix time",lnfTime);
	lAddNativeFunc(c,"strftime",         "[ts format]","Returns TS as a date using FORMAT (uses strftime)",lnfStrftime);
	lAddNativeFunc(c,"time-milliseconds","[]",         "Returns monotonic msecs",lnfTimeMsecs);
}
