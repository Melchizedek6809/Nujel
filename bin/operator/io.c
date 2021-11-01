/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "io.h"
#include "../misc.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

lSymbol *lsMode, *lsSize, *lsUserID, *lsGroupID, *lsAccessTime, *lsModificationTime;

void setIOSymbols(){
	lsMode             = lSymS(":mode");
	lsSize             = lSymS(":size");
	lsUserID           = lSymS(":user-id");
	lsGroupID          = lSymS(":group-id");
	lsAccessTime       = lSymS(":access-time");
	lsModificationTime = lSymS(":modification-time");
}

static lVal *lnfQuit(lClosure *c, lVal *v){
	(void)c;
	exit(castToInt(lCar(v),0));
	return NULL;
}

static lVal *lnfInput(lClosure *c, lVal *v){
	(void)c;
	const char *prompt = castToString(lCar(v),NULL);
	if(prompt != NULL){
		printf("%s",prompt);
	}
	char buf[4096];
	if(fgets(buf,sizeof(buf),stdin) == NULL){
		return NULL;
	}
	return lValString(buf);
}

static lVal *lnfPrint(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return v;}
	lDisplayVal(lCar(v));
	return NULL;
}

static lVal *lnfError(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return v;}
	lDisplayErrorVal(lCar(v));
	return NULL;
}

static lVal *lnfFileRead(lClosure *c, lVal *v){
	(void)c;
	const char *filename = castToString(lCar(v),NULL);
	if(filename == NULL){return NULL;}
	size_t len = 0;
	const char *data = loadFile(filename,&len);
	lVal *ret = lValString(data);
	free((void *)data);
	return ret;

}

static lVal *lnfFileWrite(lClosure *c, lVal *v){
	(void)c;
	const char *filename = castToString( lCar(v),NULL);
	const char *content  = castToString(lCadr(v),NULL);
	if(filename == NULL){return NULL;}
	if(content  == NULL){return NULL;}
	size_t len = strnlen(content,1<<20);
	saveFile(filename,content,len);
	return NULL;
}

static lVal *lnfFileRemove(lClosure *c, lVal *v){
	(void)c;
	const char *filename = castToString( lCar(v),NULL);
	if(filename == NULL){return NULL;}
	unlink(filename);
	return NULL;
}

static lVal *lnfFileStat(lClosure *c, lVal *v){
	(void)c;
	const char *filename = castToString(lCar(v),NULL);
	if(filename == NULL){return NULL;}
	struct stat statbuf;
	int err = stat(filename,&statbuf);
	if(err){
		lPrintError("Error while trying to gather stats for %s, errno: [%u] %s\n",filename,errno,strerror(errno));
		return NULL;
	}
	lVal *ret = lRootsValPush(lValTree(NULL));
	ret->vTree = lTreeInsert(ret->vTree, lsMode, lValInt(statbuf.st_mode));
	ret->vTree = lTreeInsert(ret->vTree, lsUserID, lValInt(statbuf.st_uid));
	ret->vTree = lTreeInsert(ret->vTree, lsGroupID, lValInt(statbuf.st_gid));
	ret->vTree = lTreeInsert(ret->vTree, lsSize, lValInt(statbuf.st_size));
	ret->vTree = lTreeInsert(ret->vTree, lsAccessTime, lValInt(statbuf.st_atime));
	ret->vTree = lTreeInsert(ret->vTree, lsModificationTime, lValInt(statbuf.st_mtime));

	return ret;
}

static lVal *lnfFileTemp(lClosure *c, lVal *v){
	(void)c; (void)v;
	const char *content  = castToString(lCar(v),NULL);

	char buf[32];
	snprintf(buf,sizeof(buf),"/tmp/nujel-XXXXXX");
	int ret = mkstemp(buf);
	FILE *fd = fdopen(ret,"w");

	if(content){
		const int len = lStringLength(lCar(v)->vString);
		int written = 0;
		while(written < len){
			int r = fwrite(&content[written],1,len - written,fd);
			if(r <= 0){
				if(ferror(fd)){
					lPrintError("Error while writing to temporary file: %s\n",buf);
					break;
				}
			}else{
				written += r;
			}
		}
	}

	fclose(fd);
	return lValString(buf);
}

static lVal *lnfPopen(lClosure *c, lVal *v){
	(void) c;

	const char *command = castToString(lCar(v),NULL);
	if(command == NULL){return NULL;}

	const int readSize = 1<<12;
	int len   = 0;
	int bufSize = readSize;
	char *buf = malloc(readSize);

	FILE *child = popen(command,"r");
	if(child == NULL){
		printf("Error openeing %s\n",command);
	}
	while(1){
		int ret = fread(&buf[len],1,readSize,child);
		if(ret < readSize){
			if(feof(child)){
				len += ret;
				break;
			}else if(ferror(child)){
				pclose(child);
				return NULL;
			}
		}
		if(ret > 0){
			len += ret;
		}
		if((len + readSize) >= bufSize){
			bufSize += readSize;
			buf = realloc(buf,bufSize);
		}
	}
	const int exitStatus = pclose(child);

	buf = realloc(buf,len+1);
	buf[len] = 0;

	lVal *ret = lRootsValPush(lCons(NULL,NULL));
	ret->vList.car = lValInt(exitStatus);
	ret->vList.cdr = lValStringNoCopy(buf,len);

	return ret;
}

void addNativeFuncs(lClosure *c){
	lAddNativeFunc(c,"error",      "[...args]",          "Prints ...args to stderr",                          lnfError);
	lAddNativeFunc(c,"print",      "[...args]",          "Displays ...args",                                  lnfPrint);
	lAddNativeFunc(c,"input",      "[]",                 "Reads in a line of user input and returns it",      lnfInput);
	lAddNativeFunc(c,"exit",       "[a]",                "Quits with code a",                                 lnfQuit);
	lAddNativeFunc(c,"popen",      "[command]",          "Return a list of [exit-code stdout stderr]",        lnfPopen);
	lAddNativeFunc(c,"file/read",  "[filename]",         "Load FILENAME and return the contents as a string", lnfFileRead);
	lAddNativeFunc(c,"file/write", "[filename content]", "Writes CONTENT into FILENAME",                      lnfFileWrite);
	lAddNativeFunc(c,"file/remove","[filename]",         "Remove FILENAME from the filesystem, if possible",  lnfFileRemove);
	lAddNativeFunc(c,"file/temp",  "[content]",          "Write CONTENT to a temp file and return its path",  lnfFileTemp);
	lAddNativeFunc(c,"file/stat",  "[filename]",         "Return some stats about FILENAME",                  lnfFileStat);
}
