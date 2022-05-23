/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "getmsecs.h"

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
