/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "misc.h"
#include "../lib/printer.h"

#ifdef __WATCOMC__
	#include <direct.h>
	#include <process.h>
#else
	#include <dirent.h>
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef __MINGW32__
#include <windows.h>
#include <shlobj.h>
#endif

void lWidgetMarkI(uint i){(void)i;}

/* Load filename into a fresly allocated buffer which is always zero terminated.
 * The length is returned in len if len is not NULL */
void *loadFile(const char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	u8 *buf = NULL;

#ifdef __WATCOMC__
	char dosFilename[256];
	for(int i=0;i<sizeof(dosFilename);i++){
		dosFilename[i] = *filename++;
		if(dosFilename[i] == 0){break;}
		if(dosFilename[i] == '/'){dosFilename[i] = '\\';}
	}
	dosFilename[sizeof(dosFilename)-1] = 0;
	fp = fopen(dosFilename,"rb");
#else
	fp = fopen(filename,"rb");
#endif
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

	if(len){*len = filelen;}
	return buf;
}

/* Save len bytes of buf into filename, overwriting whatever was there before */
void saveFile(const char *filename,const void *buf, size_t len){
	FILE *fp;
	size_t written,wlen = 0;
	const char *cbuf = buf;
	#if defined (__EMSCRIPTEN__)
	(void)filename; (void)buf; (void)len;
	return;
	#endif

	fp = fopen(filename,"wb");
	if(fp == NULL){return;}

	while(wlen < len){
		written = fwrite(&cbuf[wlen],1,len-wlen,fp);
		if(written == 0){return;}
		wlen += written;
	}
	fclose(fp);
}

/* Return true if name is a directory */
int isDir(const char *name){
	DIR *dp = opendir(name);
	if(dp == NULL){return 0;}
	closedir(dp);
	return 1;
}

/* Create a new directory in a portable manner */
int makeDir(const char *name){
	if(isDir(name)){return 1;}
	#if defined(__MINGW32__) || defined(__WATCOMC__)
	return mkdir(name);
	#elif defined (__EMSCRIPTEN__)
	(void)name;
	return 1;
	#else
	return mkdir(name,0755);
	#endif
}

/* Generate a temporary filename in a portable manner */
const char *tempFilename(){
	static uint counter = 0;
	static char buf[32];
	spf(buf,&buf[sizeof(buf)], "/tmp/nujel-%x-%x", getpid(),++counter);
	return buf;
}
