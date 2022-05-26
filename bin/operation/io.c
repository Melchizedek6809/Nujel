/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "io.h"
#include "../misc.h"

#ifdef __WATCOMC__
	#include <direct.h>
#else
	#include <dirent.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


#if (defined(__WATCOMC__)) || (defined(__EMSCRIPTEN__))
	#define NO_POPEN
#else
	#define ENABLE_POPEN
#endif

#if (!defined(__MINGW32__)) && (!defined(__WATCOMC__))
	#include <sys/wait.h>
#endif

lSymbol *lsError;
lSymbol *lsErrorNumber;
lSymbol *lsErrorText;
lSymbol *lsMode;
lSymbol *lsSize;
lSymbol *lsUserID;
lSymbol *lsGroupID;
lSymbol *lsAccessTime;
lSymbol *lsModificationTime;

lSymbol *lsRegularFile;
lSymbol *lsDirectory;
lSymbol *lsCharacterDevice;
lSymbol *lsBlockDevice;
lSymbol *lsNamedPipe;

void (*ioMarkerChain)() = NULL;
void ioSymbolMarker(){
	lSymbolGCMark(lsError);
	lSymbolGCMark(lsErrorText);
	lSymbolGCMark(lsMode);
	lSymbolGCMark(lsSize);
	lSymbolGCMark(lsUserID);
	lSymbolGCMark(lsGroupID);
	lSymbolGCMark(lsAccessTime);
	lSymbolGCMark(lsModificationTime);
	lSymbolGCMark(lsRegularFile);
	lSymbolGCMark(lsDirectory);
	lSymbolGCMark(lsCharacterDevice);
	lSymbolGCMark(lsBlockDevice);
	lSymbolGCMark(lsNamedPipe);

	if(ioMarkerChain){ioMarkerChain();}
}

void setIOSymbols(){
	lsError            = lSymS("error?");
	lsErrorNumber      = lSymS("error-number");
	lsErrorText        = lSymS("error-text");
	lsMode             = lSymS("mode");
	lsSize             = lSymS("size");
	lsUserID           = lSymS("user-id");
	lsGroupID          = lSymS("group-id");
	lsAccessTime       = lSymS("access-time");
	lsModificationTime = lSymS("modification-time");

	lsRegularFile      = lSymS("regular-file?");
	lsDirectory        = lSymS("directory?");
	lsCharacterDevice  = lSymS("character-device?");
	lsBlockDevice      = lSymS("block-device?");
	lsNamedPipe        = lSymS("named-pipe?");

	ioMarkerChain = rootsMarkerChain;
	rootsMarkerChain = ioSymbolMarker;
}

extern uint symbolLookups;
extern uint tombLookups;

static lVal *lnfQuit(lClosure *c, lVal *v){
	(void)c;
	if(lVerbose){
		pf("\nLookups %u/%u == %f\n", (i64)symbolLookups, (i64)tombLookups, (double)tombLookups / (double)symbolLookups);
	}
	lVal *car = lCar(v);
	exit((car && (car->type == ltInt)) ? car->vInt : 0);
	return NULL;
}

static lVal *lnfInput(lClosure *c, lVal *v){
	(void)c;
	const char *prompt = castToString(lCar(v),NULL);
	if(prompt != NULL){
		pf("%s",prompt);
	}
	char buf[4096];
	if(fgets(buf,sizeof(buf),stdin) == NULL){
		return NULL;
	}
	return lValString(buf);
}

#ifdef __EMSCRIPTEN__
	#include <emscripten.h>
	EM_JS(void, wasmConsoleLog, (const char *str), {
		console.log(UTF8ToString(str));
	});

	EM_JS(void, wasmConsoleError, (const char *str), {
		console.error(UTF8ToString(str));
	});
#endif

static void lWriteVal(lVal *v, FILE *fp){
	char dispWriteBuf[1<<16];
	char *end = spf(dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)-1],"%V",v);
	#ifdef __EMSCRIPTEN__
	if(fp == stderr){
		wasmConsoleError(dispWriteBuf);
	}else{
		wasmConsoleLog(dispWriteBuf);
	}
	#endif
	fwrite(dispWriteBuf, end - dispWriteBuf, 1, fp);
}

static lVal *lnfError(lClosure *c, lVal *v){
	(void)c;
	lWriteVal(lCar(v), stderr);
	return NULL;
}

static lVal *lnfPrint(lClosure *c, lVal *v){
	(void)c;
	lWriteVal(lCar(v), stdout);
	return NULL;
}

static lVal *lnfFileRead(lClosure *c, lVal *v){
	size_t len       = 0;
	lString *str     = requireString(c, lCar(v));
	const char *data = loadFile(str->data,&len);
	return lValStringNoCopy(data, len);
}

static lVal *lnfFileWrite(lClosure *c, lVal *v){
	lVal *contentV    = lCar(v);
	lString *content  = requireString(c, contentV);
	lString *filename = requireString(c, lCadr(v));

	saveFile(filename->data, content->data, lStringLength(content));
	return contentV;
}

static lVal *lnfFileRemove(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	lString *filename = requireString(c, car);
	unlink(filename->data);
	return car;
}

static lVal *lnfFileStat(lClosure *c, lVal *v){
	lString *filename = requireString(c, lCar(v));
	struct stat statbuf;
	int err = stat(filename->data, &statbuf);
	lVal *ret = lValTree(NULL);
	ret->vTree = lTreeInsert(ret->vTree, lsError, lValBool(err));
	if(err){
		ret->vTree = lTreeInsert(ret->vTree, lsErrorNumber,      lValInt(errno));
		ret->vTree = lTreeInsert(ret->vTree, lsErrorText,        lValString(strerror(errno)));
	}else{
		ret->vTree = lTreeInsert(ret->vTree, lsMode,             lValInt(statbuf.st_mode));
		ret->vTree = lTreeInsert(ret->vTree, lsUserID,           lValInt(statbuf.st_uid));
		ret->vTree = lTreeInsert(ret->vTree, lsGroupID,          lValInt(statbuf.st_gid));
		ret->vTree = lTreeInsert(ret->vTree, lsSize,             lValInt(statbuf.st_size));
		ret->vTree = lTreeInsert(ret->vTree, lsAccessTime,       lValInt(statbuf.st_atime));
		ret->vTree = lTreeInsert(ret->vTree, lsModificationTime, lValInt(statbuf.st_mtime));

		ret->vTree = lTreeInsert(ret->vTree, lsRegularFile,      lValBool(S_ISREG(statbuf.st_mode)));
		ret->vTree = lTreeInsert(ret->vTree, lsDirectory,        lValBool(S_ISDIR(statbuf.st_mode)));
		ret->vTree = lTreeInsert(ret->vTree, lsCharacterDevice,  lValBool(S_ISCHR(statbuf.st_mode)));
		ret->vTree = lTreeInsert(ret->vTree, lsBlockDevice,      lValBool(S_ISBLK(statbuf.st_mode)));
		ret->vTree = lTreeInsert(ret->vTree, lsNamedPipe,        lValBool(S_ISFIFO(statbuf.st_mode)));
	}
	return ret;
}


static lVal *lnfFileTemp(lClosure *c, lVal *v){
	lString *content = requireString(c, lCar(v));

	const char *ret = tempFilename();
	FILE *fd = fopen(ret,"w");

	const int len = lStringLength(content);
	int written = 0;
	while(written < len){
		int r = fwrite(&content->data[written], 1, len - written, fd);
		if(r <= 0){
			if(ferror(fd)){
				fpf(stderr, "Error while writing to temporary file: %s\n", ret);
				break;
			}
		}else{
			written += r;
		}
	}

	fclose(fd);
	return lValString(ret);
}

#ifdef ENABLE_POPEN
static lVal *lnfPopen(lClosure *c, lVal *v){
	lString *command = requireString(c, lCar(v));

	const int readSize = 1<<12;
	int len   = 0;
	int bufSize = readSize;
	char *buf = malloc(readSize);

	FILE *child = popen(command->data, "r");
	if(child == NULL){
		fpf(stderr,"Error opening %s\n",command);
		return NULL;
	}
	while(1){
		const int ret = fread(&buf[len],1,readSize,child);
		if(ret < readSize){
			if(feof(child)){
				len += ret;
				break;
			}else if(ferror(child)){
				pclose(child);
				return NULL;
			}
		}
		if(ret > 0){len += ret;}

		if((len + readSize) >= bufSize){
			bufSize += readSize;
			buf = realloc(buf,bufSize);
		}
	}
	#ifdef __MINGW32__
	int exitCode = pclose(child);
	#else
	int exitStatus = pclose(child);
	int exitCode = WEXITSTATUS(exitStatus);
	#endif

	buf = realloc(buf,len+1);
	buf[len] = 0;

	return lCons(lValInt(exitCode),lValStringNoCopy(buf,len));
}
#else
static lVal *lnfPopen(lClosure *c, lVal *v){
	lExceptionThrowValClo("not-available","[popen] is not implemented on your current platform, please try and work around that", v, c);
	return NULL;
}
#endif

static lVal *lnfDirectoryRead(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	const bool showHidden = castToBool(lCadr(v));

	DIR *dp = opendir(path->data);
	if(dp == NULL){return NULL;}
	lVal *ret = NULL;
	lVal *cur = NULL;
	for(struct dirent *de = readdir(dp); de ; de = readdir(dp)){
		if(!showHidden){
			if(de->d_name[0] == '.'){continue;}
		}
		if((de->d_name[0] == '.') && (de->d_name[1] == 0)){continue;}
		if((de->d_name[0] == '.') && (de->d_name[1] == '.') && (de->d_name[2] == 0)){continue;}
		if(cur == NULL){
			ret = cur = lCons(NULL,NULL);
		}else{
			cur = cur->vList.cdr = lCons(NULL,NULL);
		}
		cur->vList.car = lValString(de->d_name);
	}

	closedir(dp);
	return ret;
}

static lVal *lnfDirectoryMake(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	return lValBool(makeDir(path->data) == 0);
}

static lVal *lnfDirectoryRemove(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	return lValBool(rmdir(path->data) == 0);
}

static lVal *lnfChangeDirectory(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	return lValBool(chdir(path->data) == 0);
}

static lVal *lnfGetCurrentWorkingDirectory(lClosure *c, lVal *v){
	(void)c;(void)v;
	char path[512];
	if(getcwd(path, sizeof(path)) == NULL){
		return NULL;
	}
	return lValString(path);
}

void lOperationsIO(lClosure *c){
	lAddNativeFunc(c,"error",            "[v]",            "Prints v to stderr",                                lnfError);
	lAddNativeFunc(c,"print",            "[v]",            "Displays v",                                        lnfPrint);
	lAddNativeFunc(c,"input",            "[]",             "Reads in a line of user input and returns it",      lnfInput);
	lAddNativeFunc(c,"exit",             "[a]",            "Quits with code a",                                 lnfQuit);
	lAddNativeFunc(c,"popen",            "[command]",      "Return a list of [exit-code stdout stderr]",        lnfPopen);

	lAddNativeFunc(c,"file/read",        "[path]",         "Load FILENAME and return the contents as a string", lnfFileRead);
	lAddNativeFunc(c,"file/write",       "[content path]", "Writes CONTENT into FILENAME",                      lnfFileWrite);
	lAddNativeFunc(c,"file/remove",      "[path]",         "Remove FILENAME from the filesystem, if possible",  lnfFileRemove);
	lAddNativeFunc(c,"file/temp",        "[content]",      "Write CONTENT to a temp file and return its path",  lnfFileTemp);
	lAddNativeFunc(c,"file/stat",        "[path]",         "Return some stats about FILENAME",                  lnfFileStat);

	lAddNativeFunc(c,"directory/read",   "[path show-hidden]",   "Return all files within $PATH",               lnfDirectoryRead);
	lAddNativeFunc(c,"directory/remove", "[path]",               "Remove empty directory at PATH",              lnfDirectoryRemove);
	lAddNativeFunc(c,"directory/make",   "[path]",               "Create a new empty directory at PATH",        lnfDirectoryMake);

	lAddNativeFunc(c,"path/change",      "[path]",         "Change the current working directory to PATH",      lnfChangeDirectory);
	lAddNativeFunc(c,"path/working-directory","[]",        "Return the current working directory",              lnfGetCurrentWorkingDirectory);
}
