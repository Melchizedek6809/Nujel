/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../collection/string.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <time.h>
#ifdef __WATCOMC__
	#include <dos.h>
	#include "../misc/pf.h"

	static void clock_gettime(int type, struct timespec *tv){
		struct dostime_t dtime;
		_dos_gettime( &dtime );

		tv->tv_nsec =  dtime.hsecond * 10000000l;
		tv->tv_sec  = time(NULL);
	}

	#define CLOCK_MONOTONIC 123
#else
	#include <sys/time.h>
#endif

#ifdef __MINGW32__
	#include <pthread_time.h>
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
