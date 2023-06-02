/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#if defined(_MSC_VER)
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


#if defined(__EMSCRIPTEN__) || defined(_MSC_VER)
	#define NO_POPEN
#else
	#define ENABLE_POPEN
#endif

#if (!defined(_WIN32))
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

lSymbol *lSymError;
lSymbol *lSymReplace;
lSymbol *lSymAppend;

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

	lSymError          = lSymSM("error");
	lSymReplace        = lSymSM("replace");
	lSymAppend         = lSymSM("append");
}

static lVal lnfQuit(lClosure *c, lVal v){
	(void)c;
	exit(castToInt(lCar(v), 0));
	return NIL;
}

static lVal lnfFileRemove(lClosure *c, lVal v){
	lVal car = lCar(v);
	lString *filename = requireString(c, car);
	unlink(lBufferData(filename));
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

static lVal lnfFileStat(lClosure *c, lVal v){
	lString* filename = requireString(c, lCar(v));
#ifdef _MSC_VER
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	size_t length_of_arg = 0;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	if (unlikely(lStringLength(filename) >= MAX_PATH)) {
		lExceptionThrowValClo("invalid-call", "Directory path is too long.", lCar(v), c);
		return NULL;
	}

	hFind = FindFirstFile(lStringData(filename), &ffd);


	lVal ret = lValTree(NULL);
	lTreeRoot *t = ret.vTree;
	t->root = lTreeInsert(t->root, lsError, lValBool(INVALID_HANDLE_VALUE == hFind));
	if (likely(INVALID_HANDLE_VALUE != hFind)) {
		LARGE_INTEGER filesize;
		filesize.LowPart = ffd.nFileSizeLow;
		filesize.HighPart = ffd.nFileSizeHigh;
		t->root = lTreeInsert(t->root, lsSize, lValInt(filesize.QuadPart));
		t->root = lTreeInsert(t->root, lsAccessTime, lValInt(FileTime_to_POSIX(ffd.ftLastAccessTime)));
		t->root = lTreeInsert(t->root, lsCreationTime, lValInt(FileTime_to_POSIX(ffd.ftCreationTime)));
		t->root = lTreeInsert(t->root, lsModificationTime, lValInt(FileTime_to_POSIX(ffd.ftLastWriteTime)));
		t->root = lTreeInsert(t->root, lsUserID, lValInt(1000));
		t->root = lTreeInsert(t->root, lsGroupID, lValInt(1000));

		t->root = lTreeInsert(t->root, lsRegularFile, lValBool(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)));
		t->root = lTreeInsert(t->root, lsDirectory, lValBool(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		t->root = lTreeInsert(t->root, lsCharacterDevice, lValBool(false));
		t->root = lTreeInsert(t->root, lsBlockDevice, lValBool(false));
		t->root = lTreeInsert(t->root, lsNamedPipe, lValBool(false));
	}
	return ret;
#else
	struct stat statbuf;
	int err = stat(lBufferData(filename), &statbuf);
	lVal ret = lValTree(NULL);
	lTreeRoot *t = ret.vTree;
	t->root = lTreeInsert(t->root, lsError, lValBool(err));
	if(err){
		t->root = lTreeInsert(t->root, lsErrorNumber,      lValInt(errno));
		t->root = lTreeInsert(t->root, lsErrorText,        lValString(strerror(errno)));
	}else{
		t->root = lTreeInsert(t->root, lsUserID,           lValInt(statbuf.st_uid));
		t->root = lTreeInsert(t->root, lsGroupID,          lValInt(statbuf.st_gid));
		t->root = lTreeInsert(t->root, lsSize,             lValInt(statbuf.st_size));
		t->root = lTreeInsert(t->root, lsAccessTime,       lValInt(statbuf.st_atime));
		t->root = lTreeInsert(t->root, lsModificationTime, lValInt(statbuf.st_mtime));

		t->root = lTreeInsert(t->root, lsRegularFile,      lValBool(S_ISREG(statbuf.st_mode)));
		t->root = lTreeInsert(t->root, lsDirectory,        lValBool(S_ISDIR(statbuf.st_mode)));
		t->root = lTreeInsert(t->root, lsCharacterDevice,  lValBool(S_ISCHR(statbuf.st_mode)));
		t->root = lTreeInsert(t->root, lsBlockDevice,      lValBool(S_ISBLK(statbuf.st_mode)));
		t->root = lTreeInsert(t->root, lsNamedPipe,        lValBool(S_ISFIFO(statbuf.st_mode)));
	}
	return ret;
#endif
}

#ifdef ENABLE_POPEN
static lVal lnfPopen(lClosure *c, lVal v){
	lString *command = requireString(c, lCar(v));

	const int readSize = 1<<12;
	int len   = 0;
	int bufSize = readSize;
	char *buf = malloc(readSize);

	FILE *child = popen(lBufferData(command), "r");
	if(child == NULL){
		free(buf);
		return NIL;
	}
	while(1){
		const int ret = fread(&buf[len],1,readSize,child);
		if(ret < readSize){
			if(feof(child)){
				len += ret;
				break;
			}else if(ferror(child)){
				pclose(child);
				return NIL;
			}
		}
		if(ret > 0){len += ret;}

		if((len + readSize) >= bufSize){
			bufSize += readSize;
			buf = realloc(buf,bufSize);
		}
	}
	#ifdef __MINGW32__
	const int exitCode = pclose(child);
	#else
	const int exitStatus = pclose(child);
	const int exitCode = WEXITSTATUS(exitStatus);
	#endif

	buf = realloc(buf,len+1);
	buf[len] = 0;

	return lCons(lValInt(exitCode),lValStringNoCopy(buf,len));
}
#else
static lVal lnfPopen(lClosure *c, lVal v){
	lExceptionThrowValClo("not-available","(popen) is not implemented on your current platform, please try and work around that", v, c);
	return NULL;
}
#endif

static lVal lnfDirectoryRead(lClosure *c, lVal v){
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

	lVal ret = NULL;
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
	if(dp == NULL){return NIL;}
	lVal ret = NIL;
	lVal cur = NIL;
	for(struct dirent *de = readdir(dp); de ; de = readdir(dp)){
		if(!showHidden){
			if(de->d_name[0] == '.'){continue;}
		}
		if((de->d_name[0] == '.') && (de->d_name[1] == 0)){continue;}
		if((de->d_name[0] == '.') && (de->d_name[1] == '.') && (de->d_name[2] == 0)){continue;}
		if(cur.type == ltNil){
			ret = cur = lCons(NIL, NIL);
		}else{
			cur = cur.vList->cdr = lCons(NIL, NIL);
		}
		cur.vList->car = lValString(de->d_name);
	}

	closedir(dp);
	return ret;
#endif
}

static lVal lnfDirectoryMake(lClosure *c, lVal v){
	lString *path = requireString(c, lCar(v));
	return lValBool(makeDir(lBufferData(path)) == 0);
}

static lVal lnfDirectoryRemove(lClosure *c, lVal v){
	lString *path = requireString(c, lCar(v));
	return lValBool(rmdir(lBufferData(path)) == 0);
}

static lVal lnfChangeDirectory(lClosure *c, lVal v){
	lString *path = requireString(c, lCar(v));
	return lValBool(chdir(lBufferData(path)) == 0);
}

static lVal lnfGetCurrentWorkingDirectory(lClosure *c, lVal v){
	(void)c;(void)v;
	char path[512];
	if(!getcwd(path, sizeof(path))){
		return NIL;
	}
	return lValString(path);
}

void lOperationsIO(lClosure *c){
	lAddNativeFunc(c,"exit",                       "(a)",                "Quits with code a",                                 lnfQuit);
	lAddNativeFunc(c,"popen",                      "(command)",          "Return a list of [exit-code stdout stderr)",        lnfPopen);

	lAddNativeFunc(c,"file/stat",                  "(path)",             "Return some stats about FILENAME",                  lnfFileStat);
	lAddNativeFunc(c,"rm file/remove",             "(path)",             "Remove FILENAME from the filesystem, if possible",  lnfFileRemove);
	lAddNativeFunc(c,"ls directory/read",          "(path show-hidden)", "Return all files within $PATH",                     lnfDirectoryRead);
	lAddNativeFunc(c,"rmdir directory/remove",     "(path)",             "Remove empty directory at PATH",                    lnfDirectoryRemove);
	lAddNativeFunc(c,"mkdir directory/make",       "(path)",             "Create a new empty directory at PATH",              lnfDirectoryMake);
	lAddNativeFunc(c,"cd path/change",             "(path)",             "Change the current working directory to PATH",      lnfChangeDirectory);
	lAddNativeFunc(c,"cwd path/working-directory", "()",                 "Return the current working directory",              lnfGetCurrentWorkingDirectory);
}