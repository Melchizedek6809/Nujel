/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "misc.h"

#include <stdarg.h>
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef __MINGW32__
#include <windows.h>
#include <shlobj.h>
#endif

void lWidgetMarkI(uint i){(void)i;}
void lPrintError(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vfprintf(stderr,format,ap);
	va_end(ap);
}


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

int isDir(const char *name){
	#if defined (__EMSCRIPTEN__)
	(void)name;
	return 0;
	#endif
	DIR *dp = opendir(name);
	if(dp == NULL){return 0;}
	closedir(dp);
	return 1;
}

int makeDir(const char *name){
	if(isDir(name)){return 1;}
	#ifdef __MINGW32__
	return mkdir(name);
	#elif defined (__EMSCRIPTEN__)
	(void)name;
	return 1;
	#else
	return mkdir(name,0755);
	#endif
}

int makeDirR(const char *name){
	char buf[256];
	strncpy(buf,name,sizeof(buf));
	buf[sizeof(buf)-1] = 0;
	for(int i=0;i<256;i++){
		if(buf[i] != '/'){continue;}
		buf[i] = 0;
		makeDir(buf);
		buf[i] = '/';
	}
	return makeDir(buf);
}

void rmDirR(const char *name){
	#if defined (__EMSCRIPTEN__)
	return;
	#endif
	DIR *dp = opendir(name);
	if(dp == NULL){return;}
	struct dirent *de = NULL;
	while((de = readdir(dp)) != NULL){
		char buf[520];
		if(de->d_name[0] == '.'){continue;}
		snprintf(buf,sizeof(buf),"%s/%s",name,de->d_name);
		if(isDir(buf)){
			rmDirR(buf);
			rmdir(buf);
		}else{
			unlink(buf);
		}
	}
	closedir(dp);
	rmdir(name);
}
