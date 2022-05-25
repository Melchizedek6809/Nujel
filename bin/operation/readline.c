/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "readline.h"
#include "../misc.h"

#include <stdlib.h>
#include <string.h>

#if defined(__MINGW32__) || defined(__MSYS__)
	#include <windows.h>
	#include <shlobj.h>
#endif
#include "../../vendor/getline/getline.h"

#define BUF_SIZE (1 << 14)
static char *bestline(const char *prompt){
	char *buf = NULL;
	size_t bufsize = 0;

	if(prompt){
		fputs(prompt, stdout);
	}
	const int64_t ret = getline(&buf,&bufsize,stdin);

	if(ret >= 0){
		buf[MIN(bufsize-1,(size_t)ret)] = 0;
		return buf;
	}else{
		return NULL;
	}
}

static lVal *lnfReadline(lClosure *c, lVal *v){
	const char *prompt = NULL;
	lVal *car = lCar(v);
	if(car){
		lString *promptS = requireString(c, lCar(v));
		prompt = promptS->data;
	}
	char *line = bestline(prompt);
	return lValStringNoCopy(line, strnlen(line, BUF_SIZE));
}

void lOperationsReadline(lClosure *c){
	lAddNativeFunc(c,"readline", "[prompt]", "Read a line of input in a user friendly way after writing PROMPT", lnfReadline);
}
