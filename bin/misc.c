/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#if defined(_MSC_VER)
	#include <windows.h>
#else
	#include <dirent.h>
	#include <unistd.h>
#endif

#include <sys/stat.h>

#ifdef __MINGW32__
#include <windows.h>
#include <shlobj.h>
#endif

/* Return true if name is a directory */
int isDir(const char *name){
#ifdef _MSC_VER
	DWORD ftyp = GetFileAttributesA(name);
	return (ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY);
#else
	DIR *dp = opendir(name);
	if(dp == NULL){return 0;}
	closedir(dp);
	return 1;
#endif
}

/* Create a new directory in a portable manner */
int makeDir(const char *name){
	if(isDir(name)){return 1;}
	#if defined(__MINGW32__)
	return mkdir(name);
	#elif defined (__EMSCRIPTEN__)
	(void)name;
	return 1;
	#else
	return mkdir(name,0755);
	#endif
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

	buf = malloc(filelen);
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

	*len = filelen;
	return buf;
}
