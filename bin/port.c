/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

#if (!defined(_WIN32)) && (!defined(__wasi__))
#include <termios.h>
#include <stdlib.h>

bool rawMode = false;
struct termios orig_termios;

static void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static lVal lnfFileRaw(lVal handle){
	struct termios raw;
	lVal car = requireFileHandle(handle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;

	tcgetattr(fileno(fh), &raw);
	if(!rawMode){
		orig_termios = raw;
		atexit(disableRawMode);
		rawMode = true;
	}
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	tcsetattr(fileno(fh), TCSAFLUSH, &raw);

	return NIL;
}

#endif


static lVal lnfFileOpenOutput(lVal aPathname, lVal aIfExists){
	lVal car = requireString(aPathname);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lString *pathname = car.vString;
	const char *path = lBufferData(pathname);
	lVal cadr = optionalSymbolic(aIfExists, lSymError);
	if(unlikely(cadr.type == ltException)){
		return cadr;
	}
	const lSymbol *mode = cadr.vSymbol;

	FILE *fh = NULL;
	if(mode == lSymError){
#if defined(_MSC_VER)
		if (_access(path, 02) == 0) { lValBool(false); }
#else
		if(access(path, F_OK) == 0){lValBool(false);}
#endif
		fh = fopen(path, "wb");
	}else if(mode == lSymReplace){
		fh = fopen(path, "wb");
	}else if(mode == lSymAppend){
		fh = fopen(path, "ab");
	}else{
		return lValException(lSymTypeError, "Don't know that particular behaviour: ", aIfExists);
	}

	return fh ? lValFileHandle(fh) : NIL;
}

static lVal lnfFileOpenInput(lVal aPathname){
	lVal car = requireString(aPathname);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lString *pathname = car.vString;
	FILE *fh = fopen(lBufferData(pathname), "rb");
	return fh ? lValFileHandle(fh) : NIL;
}

static lVal lnfFileClose(lVal aHandle){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;
	fclose(fh);
	return NIL;
}

static lVal lnfFileReadAst(lVal aHandle, lVal aBuffer, lVal aSize, lVal aOffset){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;
	lVal contentV = aBuffer;
	lVal sizeV = requireNaturalInt(aSize);
	if(unlikely(sizeV.type == ltException)){
		return sizeV;
	}
	const i64 size = sizeV.vInt;
	const i64 offset = castToInt(aOffset, 0);

	void *buf = NULL;
	i64 bufSize = 0;
	i64 bytesRead = 0;
	switch(contentV.type){
	default:
		return lValException(lSymTypeError, "Can't read into that", contentV);
	case ltBuffer:
		buf = lBufferDataMutable(contentV.vBuffer);
		bufSize = lBufferLength(contentV.vBuffer);
		break;
	case ltBufferView:{
		buf = lBufferViewDataMutable(contentV.vBufferView);
		bufSize = lBufferViewLength(contentV.vBufferView);
		break;
	}}
	if(buf == NULL){
		return lValException(lSymTypeError, "Can't read into an immutable buffer", contentV);
	}
	if((bufSize - offset) < size){
		return lValException(lSymTypeError, "Buffer is too small for that read operation", contentV);
	}
	while(bytesRead < size){
		const int r = fread(&((u8 *)buf)[offset + bytesRead], 1, size - bytesRead, fh);
		if(ferror(fh)){
			return lValException(lSymIOError, "IO Error occured during read", aHandle);
		}
		if(feof(fh)){
			return NIL;
		}
		if(r > 0){ bytesRead += r; }
	}
	return aHandle;
}

static lVal lnfFileWriteAst(lVal aHandle, lVal aBuffer, lVal aSize, lVal aOffset){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;
	lVal contentV = aBuffer;
	i64 size = castToInt(aSize, -1);
	const i64 offset = castToInt(aOffset, 0);

	const void * buf = NULL;
	i64 bufSize = 0;
	i64 bytesWritten = 0;
	switch(contentV.type){
	default:
		return lValException(lSymTypeError, "Can't read into that", contentV);
	case ltString:
	case ltBuffer:
		buf = lBufferData(contentV.vBuffer);
		bufSize = lBufferLength(contentV.vBuffer);
		break;
	case ltBufferView:{
		buf = lBufferViewData(contentV.vBufferView);
		bufSize = lBufferViewLength(contentV.vBufferView);
		break;
	}}
	if(buf == NULL){
		return lValException(lSymTypeError, "Can't read into an immutable buffer", contentV);
	}
	if(size < 0){
		size = bufSize - offset;
	}
	if((bufSize - offset) < size){
		return lValException(lSymTypeError, "Buffer is too small for that read operation", contentV);
	}
	while(bytesWritten < size){
		const int r = fwrite(&((u8 *)buf)[offset + bytesWritten], 1, size - bytesWritten, fh);
		if(ferror(fh)){
			return lValException(lSymIOError, "IO Error occured during write", aHandle);
		}
		if(r > 0){ bytesWritten += r; }
	}

	return aHandle;
}

static lVal lnfFileFlush(lVal aHandle){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;
	fflush(fh);
	return car;
}

static lVal lnfFileTell(lVal aHandle){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;
	const i64 pos = ftell(fh);
	return lValInt(pos);
}

static lVal lnfFileSeek(lVal aHandle, lVal aOffset, lVal aWhence){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	FILE *fh = car.vFileHandle;
	lVal offV = requireInt(aOffset);
	if(unlikely(offV.type == ltException)){
		return offV;
	}
	const i64 offset = offV.vInt;
	lVal whenceV = requireInt(aWhence);
	if(unlikely(whenceV.type == ltException)){
		return whenceV;
	}
	const i64 whenceRaw = whenceV.vInt;
	if((whenceRaw < 0) || (whenceRaw > 2)){
		return lValException(lSymTypeError, "Whence has to be in the range 0-2", aWhence);
	}
	const int whence = whenceRaw == 0 ? SEEK_SET : whenceRaw == 1 ? SEEK_CUR : SEEK_END;
	fseek(fh, offset, whence);
	return aHandle;
}

static lVal lnfFileEof(lVal aHandle){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lValBool(feof(car.vFileHandle));
}

static lVal lnfFileError(lVal aHandle){
	lVal car = requireFileHandle(aHandle);
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lValBool(ferror(car.vFileHandle));
}

void lOperationsPort(lClosure *c){
	lAddNativeFuncVV  (c,"file/open-output*", "(pathname if-exists)",   "Try to open PATHNAME for MODE",        lnfFileOpenOutput, 0);
	lAddNativeFuncV   (c,"file/open-input*",  "(pathname)",             "Try to open PATHNAME for MODE",        lnfFileOpenInput, 0);
	lAddNativeFuncV   (c,"file/close*",  "(handle)",                    "Close the open HANDLE",                lnfFileClose, 0);
	lAddNativeFuncVVVV(c,"file/read*",   "(handle buffer size offset)", "Reader from HANDLE into BUFFER",       lnfFileReadAst, 0);
	lAddNativeFuncVVVV(c,"file/write*",  "(handle buffer size offset)", "Write BUFFER into HANDLE",             lnfFileWriteAst, 0);
	lAddNativeFuncV   (c,"file/flush*",  "(handle)",                    "Flush stream of HANDLE",               lnfFileFlush, 0);
	lAddNativeFuncV   (c,"file/tell*",   "(handle)",                    "Return the stream position of HANDLE", lnfFileTell, 0);
	lAddNativeFuncVVV (c,"file/seek*",   "(handle offset whence)",      "Seek stream of HANDLE to OFFSET from WHENCE where 0 is SEEK_SET, 1 is SEEK_CUR and 2 is SEEK_END", lnfFileSeek, 0);
	lAddNativeFuncV   (c,"file/eof*?",   "(handle)",                    "Return whether the end-of-file indicator is set for HANDLE", lnfFileEof, 0);
	lAddNativeFuncV   (c,"file/error*?", "(handle)",                    "Return whether the error indicator is set for HANDLE", lnfFileError, 0);
#if (!defined(_WIN32)) && (!defined(__wasi__))
	lAddNativeFuncV   (c,"file/raw*",    "(handle)",                    "Set an input stream into raw mode", lnfFileRaw, 0);
#endif
}

void lOperationsInit(lClosure *c){
	lDefineVal(c, "stdin*",  lValFileHandle(stdin));
	lDefineVal(c, "stdout*", lValFileHandle(stdout));
	lDefineVal(c, "stderr*", lValFileHandle(stderr));
}
