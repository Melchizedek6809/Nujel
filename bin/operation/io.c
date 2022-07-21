/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../private.h"
#endif

#ifdef __WATCOMC__
	#include <direct.h>
#elif defined(_MSC_VER)
	#include <windows.h>
	#include <tchar.h>
	#include <stdio.h>
	#include <strsafe.h>
#else
	#include <dirent.h>
	#include <unistd.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#if defined(__WATCOMC__) || defined(__EMSCRIPTEN__) || defined(_MSC_VER)
	#define NO_POPEN
#else
	#define ENABLE_POPEN
#endif

#if (!defined(__WATCOMC__)) && (!defined(_WIN32))
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
lSymbol* lsCreationTime;
lSymbol *lsModificationTime;

lSymbol *lsRegularFile;
lSymbol *lsDirectory;
lSymbol *lsCharacterDevice;
lSymbol *lsBlockDevice;
lSymbol *lsNamedPipe;

void setIOSymbols(){
	lsError            = lSymSM("error?");
	lsErrorNumber      = lSymSM("error-number");
	lsErrorText        = lSymSM("error-text");
	lsMode             = lSymSM("mode");
	lsSize             = lSymSM("size");
	lsUserID           = lSymSM("user-id");
	lsGroupID          = lSymSM("group-id");
	lsAccessTime       = lSymSM("access-time");
	lsCreationTime     = lSymSM("creation-time");
	lsModificationTime = lSymSM("modification-time");

	lsRegularFile      = lSymSM("regular-file?");
	lsDirectory        = lSymSM("directory?");
	lsCharacterDevice  = lSymSM("character-device?");
	lsBlockDevice      = lSymSM("block-device?");
	lsNamedPipe        = lSymSM("named-pipe?");
}

static lVal *lnfQuit(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	exit((car && (car->type == ltInt)) ? car->vInt : 0);
	return NULL;
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

static lVal *lnfFileRemove(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	lString *filename = requireString(c, car);
	unlink(lStringData(filename));
	return car;
}



#ifdef _MSC_VER
LONGLONG FileTime_to_POSIX(FILETIME ft) {
	LARGE_INTEGER date, adjust;
	date.HighPart = ft.dwHighDateTime;
	date.LowPart = ft.dwLowDateTime;
	adjust.QuadPart = 11644473600000 * 10000;
	date.QuadPart -= adjust.QuadPart;
	return date.QuadPart / 10000000;
}
#endif

static lVal *lnfFileStat(lClosure *c, lVal *v){
	lString* filename = requireString(c, lCar(v));
#ifdef _MSC_VER
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	size_t length_of_arg = 0;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	if (lStringLength(filename) >= MAX_PATH) {
		lExceptionThrowValClo("invalid-call", "Directory path is too long.", lCar(v), c);
		return NULL;
	}

	hFind = FindFirstFile(lStringData(filename), &ffd);


	lVal* ret = lValTree(NULL);
	ret->vTree = lTreeInsert(ret->vTree, lsError, lValBool(INVALID_HANDLE_VALUE == hFind));
	if (INVALID_HANDLE_VALUE != hFind) {
		LARGE_INTEGER filesize;
		filesize.LowPart = ffd.nFileSizeLow;
		filesize.HighPart = ffd.nFileSizeHigh;
		ret->vTree = lTreeInsert(ret->vTree, lsSize, lValInt(filesize.QuadPart));
		ret->vTree = lTreeInsert(ret->vTree, lsAccessTime, lValInt(FileTime_to_POSIX(ffd.ftLastAccessTime)));
		ret->vTree = lTreeInsert(ret->vTree, lsCreationTime, lValInt(FileTime_to_POSIX(ffd.ftCreationTime)));
		ret->vTree = lTreeInsert(ret->vTree, lsModificationTime, lValInt(FileTime_to_POSIX(ffd.ftLastWriteTime)));
		ret->vTree = lTreeInsert(ret->vTree, lsUserID, lValInt(1000));
		ret->vTree = lTreeInsert(ret->vTree, lsGroupID, lValInt(1000));

		ret->vTree = lTreeInsert(ret->vTree, lsRegularFile, lValBool(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)));
		ret->vTree = lTreeInsert(ret->vTree, lsDirectory, lValBool(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		ret->vTree = lTreeInsert(ret->vTree, lsCharacterDevice, lValBool(false));
		ret->vTree = lTreeInsert(ret->vTree, lsBlockDevice, lValBool(false));
		ret->vTree = lTreeInsert(ret->vTree, lsNamedPipe, lValBool(false));
	}
	return ret;
#else
	struct stat statbuf;
	int err = stat(lStringData(filename), &statbuf);
	lVal *ret = lValTree(NULL);
	ret->vTree = lTreeInsert(ret->vTree, lsError, lValBool(err));
	if(err){
		ret->vTree = lTreeInsert(ret->vTree, lsErrorNumber,      lValInt(errno));
		ret->vTree = lTreeInsert(ret->vTree, lsErrorText,        lValString(strerror(errno)));
	}else{
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
#endif
}


static lVal *lnfFileTemp(lClosure *c, lVal *v){
	lString *content = requireString(c, lCar(v));

	const char *ret = tempFilename();
	FILE *fd = fopen(ret,"w");

	const int len = lStringLength(content);
	int written = 0;
	while(written < len){
		int r = fwrite(&lStringData(content)[written], 1, ((size_t)len) - written, fd);
		if(r <= 0){
			if(ferror(fd)){
				epf("Error while writing to temporary file: %s\n", ret);
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

	FILE *child = popen(lStringData(command), "r");
	if(child == NULL){
		free(buf);
		epf("Error opening %s\n", lStringData(command));
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
	(void)c;
	const char *path = castToString(lCar(v), "./");
	const bool showHidden = castToBool(lCadr(v));

#ifdef _MSC_VER
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg = 0;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	StringCchLength(path, MAX_PATH, &length_of_arg);
	if (length_of_arg > (MAX_PATH - 3)){
		lExceptionThrowValClo("invalid-call", "Directory path is too long.", lCar(v), c);
		return NULL;
	}

	StringCchCopy(szDir, MAX_PATH, path);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		lExceptionThrowValClo("invalid-call", "FindFirstFile failed", lCar(v), c);
		return NULL;
	}

	lVal* ret = NULL;
	do {
		if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !showHidden){
			continue;
		}
		if ((ffd.cFileName[0] == '.') && (ffd.cFileName[1] == 0)) { continue; }
		if ((ffd.cFileName[0] == '.') && (ffd.cFileName[1] == '.') && (ffd.cFileName[2] == 0)) { continue; }
		ret = lCons(lValString(ffd.cFileName), ret);
   } while (FindNextFile(hFind, &ffd) != 0);
   return ret;
#else
	DIR *dp = opendir(path);
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
#endif
}

static lVal *lnfDirectoryMake(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	return lValBool(makeDir(lStringData(path)) == 0);
}

static lVal *lnfDirectoryRemove(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	return lValBool(rmdir(lStringData(path)) == 0);
}

static lVal *lnfChangeDirectory(lClosure *c, lVal *v){
	lString *path = requireString(c, lCar(v));
	return lValBool(chdir(lStringData(path)) == 0);
}

static lVal *lnfGetCurrentWorkingDirectory(lClosure *c, lVal *v){
	(void)c;(void)v;
	char path[512];
	if(!getcwd(path, sizeof(path))){
		return NULL;
	}
	return lValString(path);
}

void lOperationsIO(lClosure *c){
	lAddNativeFunc(c,"exit",                       "[a]",            "Quits with code a",                                 lnfQuit);
	lAddNativeFunc(c,"popen",                      "[command]",      "Return a list of [exit-code stdout stderr]",        lnfPopen);

	lAddNativeFunc(c,"file/remove",                "[path]",         "Remove FILENAME from the filesystem, if possible",  lnfFileRemove);
	lAddNativeFunc(c,"file/stat",                  "[path]",         "Return some stats about FILENAME",                  lnfFileStat);
	lAddNativeFunc(c,"file/temp",                  "[content]",      "Write CONTENT to a temp file and return its path",  lnfFileTemp);

	lAddNativeFunc(c,"ls directory/read",          "[path show-hidden]", "Return all files within $PATH",               lnfDirectoryRead);
	lAddNativeFunc(c,"rmdir directory/remove",     "[path]",             "Remove empty directory at PATH",              lnfDirectoryRemove);
	lAddNativeFunc(c,"mkdir directory/make",       "[path]",             "Create a new empty directory at PATH",        lnfDirectoryMake);

	lAddNativeFunc(c,"cd path/change",             "[path]",         "Change the current working directory to PATH",      lnfChangeDirectory);
	lAddNativeFunc(c,"cwd path/working-directory", "[]",             "Return the current working directory",              lnfGetCurrentWorkingDirectory);
}
