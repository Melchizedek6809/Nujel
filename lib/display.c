/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "display.h"
#include "misc/pf.h"

char dispWriteBuf[1<<18];

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>

	EM_JS(void, wasmConsoleLog, (const char *str), {
		console.log(UTF8ToString(str));
	});

	EM_JS(void, wasmConsoleError, (const char *str), {
		console.error(UTF8ToString(str));
	});
#endif

/* Display v on the default channel, most likely stdout */
void lDisplayVal(lVal *v){
	char *end = spf(dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],"%v",v);
	#ifdef __EMSCRIPTEN__
	wasmConsoleLog(dispWriteBuf);
	#endif
	fwrite(dispWriteBuf, end - dispWriteBuf, 1, stdout);
}

/* Display v on the default channel, most likely stdout */
const char *lReturnDisplayVal(lVal *v){
	spf(dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],"%v",v);
	return dispWriteBuf;
}

/* Display v on the error channel, most likely stderr */
void lDisplayErrorVal(lVal *v){
	char *end = spf(dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],"%V",v);
	#ifdef __EMSCRIPTEN__
	wasmConsoleError(dispWriteBuf);
	#endif
	fwrite(dispWriteBuf, end - dispWriteBuf, 1, stderr);
}

/* Write a machine-readable presentation of v to stdout */
void lWriteVal(lVal *v){
	char *end = spf(dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],"%V",v);
	#ifdef __EMSCRIPTEN__
	wasmConsoleLog(dispWriteBuf);
	#endif
	fwrite(dispWriteBuf, end - dispWriteBuf, 1, stdout);
}
