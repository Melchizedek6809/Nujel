/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "io.h"
#include "../misc.h"
#include "../../lib/exception.h"

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


lSymbol *lsMode;
lSymbol *lsSize;
lSymbol *lsUserID;
lSymbol *lsGroupID;
lSymbol *lsAccessTime;
lSymbol *lsModificationTime;
lSymbol *lsError;
lSymbol *lsErrorNumber;
lSymbol *lsErrorText;
lSymbol *lsRegularFile;
lSymbol *lsDirectory;
lSymbol *lsCharacterDevice;
lSymbol *lsBlockDevice;
lSymbol *lsNamedPipe;

void setIOSymbols(){
	lsError            = RSYMP(lSymS("error?"));
	lsErrorNumber      = RSYMP(lSymS("error-number"));
	lsErrorText        = RSYMP(lSymS("error-text"));
	lsMode             = RSYMP(lSymS("mode"));
	lsSize             = RSYMP(lSymS("size"));
	lsUserID           = RSYMP(lSymS("user-id"));
	lsGroupID          = RSYMP(lSymS("group-id"));
	lsAccessTime       = RSYMP(lSymS("access-time"));
	lsModificationTime = RSYMP(lSymS("modification-time"));

	lsRegularFile      = RSYMP(lSymS("regular-file?"));
	lsDirectory        = RSYMP(lSymS("directory?"));
	lsCharacterDevice  = RSYMP(lSymS("character-device?"));
	lsBlockDevice      = RSYMP(lSymS("block-device?"));
	lsNamedPipe        = RSYMP(lSymS("named-pipe?"));
}

extern uint symbolLookups;
extern uint tombLookups;

static lVal *lnfQuit(lClosure *c, lVal *v){
	(void)c;
	if(lVerbose){
		pf("\nLookups %u/%u == %f\n", (i64)symbolLookups, (i64)tombLookups, (double)tombLookups / (double)symbolLookups);
	}
	exit(castToInt(lCar(v),0));
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

static lVal *lnfPrint(lClosure *c, lVal *v){
	(void)c;
	lWriteVal(lCar(v));
	return NULL;
}

static lVal *lnfError(lClosure *c, lVal *v){
	(void)c;
	lDisplayErrorVal(lCar(v));
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
	lVal *ret = RVP(lValTree(NULL));
	ret->vTree = lTreeInsert(ret->vTree, lsError, RVP(lValBool(err)));
	if(err){
		ret->vTree = lTreeInsert(ret->vTree, lsErrorNumber,      RVP(lValInt(errno)));
		ret->vTree = lTreeInsert(ret->vTree, lsErrorText,        RVP(lValString(strerror(errno))));
	}else{
		ret->vTree = lTreeInsert(ret->vTree, lsMode,             RVP(lValInt(statbuf.st_mode)));
		ret->vTree = lTreeInsert(ret->vTree, lsUserID,           RVP(lValInt(statbuf.st_uid)));
		ret->vTree = lTreeInsert(ret->vTree, lsGroupID,          RVP(lValInt(statbuf.st_gid)));
		ret->vTree = lTreeInsert(ret->vTree, lsSize,             RVP(lValInt(statbuf.st_size)));
		ret->vTree = lTreeInsert(ret->vTree, lsAccessTime,       RVP(lValInt(statbuf.st_atime)));
		ret->vTree = lTreeInsert(ret->vTree, lsModificationTime, RVP(lValInt(statbuf.st_mtime)));

		ret->vTree = lTreeInsert(ret->vTree, lsRegularFile,      RVP(lValBool(S_ISREG(statbuf.st_mode))));
		ret->vTree = lTreeInsert(ret->vTree, lsDirectory,        RVP(lValBool(S_ISDIR(statbuf.st_mode))));
		ret->vTree = lTreeInsert(ret->vTree, lsCharacterDevice,  RVP(lValBool(S_ISCHR(statbuf.st_mode))));
		ret->vTree = lTreeInsert(ret->vTree, lsBlockDevice,      RVP(lValBool(S_ISBLK(statbuf.st_mode))));
		ret->vTree = lTreeInsert(ret->vTree, lsNamedPipe,        RVP(lValBool(S_ISFIFO(statbuf.st_mode))));
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
				lPrintError("Error while writing to temporary file: %s\n", ret);
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
		fpf(stderr,"Error openeing %s\n",command);
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

	lVal *ret = lRootsValPush(lCons(NULL,NULL));
	ret->vList.car = lValInt(exitCode);
	ret->vList.cdr = lValStringNoCopy(buf,len);

	return ret;
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
			ret = cur = lRootsValPush(lCons(NULL,NULL));
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
	lAddNativeFunc(c,"error",            "[v]",      "Prints v to stderr",                          lnfError);
	lAddNativeFunc(c,"print",            "[v]",      "Displays v",                                  lnfPrint);
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
