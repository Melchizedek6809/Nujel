/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../type-system.h"
#include "../collection/list.h"
#include "../collection/string.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <time.h>
#ifdef __WATCOMC__

	static void clock_gettime(int type, struct timespec *tv){
		tv->tv_nsec = 0;
		tv->tv_sec = time(NULL);
	}

	#define CLOCK_MONOTONIC 123
#else
	#include <sys/time.h>
#endif

/* Return monotonic time in milliseconds */
u64 getMSecs(){
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return (tv.tv_nsec / 1000000) + (tv.tv_sec * 1000);
}

/* [time] - Return the current unix time */
static lVal *lnfTime(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(time(NULL));
}

/* [time/milliseconds] - Return monotonic msecs */
static lVal *lnfTimeMsecs(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(getMSecs());
}

/* [time/strftime ts format] - eturn TS as a date using FORMAT */
static lVal *lnfStrftime(lClosure *c, lVal *v){
	(void)c;
	const int timestamp = castToInt(lCar(v),time(NULL));
	const char *format  = castToString(lCadr(v),"%Y-%m-%d %H:%M:%S");

	char buf[4096];
	time_t ts = timestamp;
	struct tm *info = localtime(&ts);
	strftime(buf,sizeof(buf),format,info);

	return lValString(buf);
}

void lOperationsTime(lClosure *c){
	lAddNativeFunc(c,"time time/unix",   "[]",         "Return the current unix time",lnfTime);
	lAddNativeFunc(c,"time/strftime",    "[ts format]","Return TS as a date using FORMAT (uses strftime)",lnfStrftime);
	lAddNativeFunc(c,"time/milliseconds","[]",         "Return monotonic msecs",lnfTimeMsecs);
}
