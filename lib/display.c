/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "display.h"
#include "api.h"

#include <stdio.h>

char dispWriteBuf[1<<18];

/* Display v on the default channel, most likely stdout */
void lDisplayVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	printf("%s",dispWriteBuf);
}

/* Display v on the default channel, most likely stdout */
const char *lReturnDisplayVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	return dispWriteBuf;
}

/* Display v on the error channel, most likely stderr */
void lDisplayErrorVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	fprintf(stderr,"%s",dispWriteBuf);
}

/* Write a machine-readable presentation of v to stdout */
void lWriteVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,false);
	printf("%s\n",dispWriteBuf);
}

/* Write a machine-readable presentation of t to stdout */
void lWriteTree(lTree *t){
	lSWriteTree(t, dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,false);
	printf("%s\n",dispWriteBuf);
}
