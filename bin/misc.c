/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "misc.h"

#ifndef COSMOPOLITAN_H_
	#include <stdio.h>
	#include <stdlib.h>
#endif

void *loadFile(const char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	u8 *buf = NULL;

	fp = fopen(filename,"rb");
	if(fp == NULL){return NULL;}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	buf = malloc(filelen+1);
	if(buf == NULL){return NULL;}

	readlen = 0;
	while(readlen < filelen){
		read = fread(buf+readlen,1,filelen-readlen,fp);
		if(read == 0){
			free(buf);
			return NULL;
		}
		readlen += read;
	}
	fclose(fp);
	buf[filelen] = 0;

	*len = filelen;
	return buf;
}

void saveFile(const char *filename,const void *buf, size_t len){
	FILE *fp;
	size_t written,wlen = 0;
	#if defined (__EMSCRIPTEN__)
	(void)filename;
	(void)buf;
	(void)len;
	return;
	#endif

	fp = fopen(filename,"wb");
	if(fp == NULL){return;}

	while(wlen < len){
		written = fwrite(buf+wlen,1,len-wlen,fp);
		if(written == 0){return;}
		wlen += written;
	}
	fclose(fp);
}
