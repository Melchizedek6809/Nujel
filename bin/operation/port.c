/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../private.h"
#endif

#if (!defined(_WIN32))
#include <termios.h>
#include <stdlib.h>

bool rawMode = false;
struct termios orig_termios;

static void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static lVal *lnfFileRaw(lClosure *c, lVal *v){
	struct termios raw;
	FILE *fh = requireFileHandle(c, lCar(v));

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

	return NULL;
}

#endif


static lVal *lnfFileOpenOutput(lClosure *c, lVal *v){
	lString *pathname = requireString(c, lCar(v));
	const char *path = lStringData(pathname);
	const lSymbol *mode = optionalSymbolic(c, lCadr(v), lSymError);

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
		lExceptionThrowValClo("type-error", "Don't know that particular behaviour: ", lCadr(v), c);
	}

	return fh ? lValFileHandle(fh) : NULL;
}

static lVal *lnfFileOpenInput(lClosure *c, lVal *v){
	lString *pathname = requireString(c, lCar(v));
	FILE *fh = fopen(lStringData(pathname), "rb");
	return fh ? lValFileHandle(fh) : NULL;
}

static lVal *lnfFileClose(lClosure *c, lVal *v){
	FILE *fh = requireFileHandle(c, lCar(v));
	fclose(fh);
	return NULL;
}

static lVal *lnfFileReadAst(lClosure *c, lVal *v){
	FILE *fh = requireFileHandle(c, lCar(v));
	lVal *contentV = lCadr(v);
	const i64 size = requireNaturalInt(c, lCaddr(v));
	const i64 offset = castToInt(lCadddr(v), 0);

	void *buf = NULL;
	i64 bufSize = 0;
	i64 bytesRead = 0;
	typeswitch(contentV){
	default:
		lExceptionThrowValClo("type-error", "Can't read into that", contentV, c);
		return NULL;
	case ltBuffer:
		buf = lBufferDataMutable(contentV->vBuffer);
		bufSize = lBufferLength(contentV->vBuffer);
		break;
	case ltBufferView:{
		buf = lBufferViewDataMutable(contentV->vBufferView);
		bufSize = lBufferViewLength(contentV->vBufferView);
		break;
	}}
	if(buf == NULL){
		lExceptionThrowValClo("type-error", "Can't read into an immutable buffer", contentV, c);
	}
	if((bufSize - offset) < size){
		lExceptionThrowValClo("type-error", "Buffer is too small for that read operation", contentV, c);
	}
	while(bytesRead < size){
		const int r = fread(&((u8 *)buf)[offset + bytesRead], 1, size - bytesRead, fh);
		if(ferror(fh)){
			lExceptionThrowValClo("io-error", "IO Error occured during read", lCar(v), c);
		}
		if(feof(fh)){
			return NULL;
		}
		if(r > 0){ bytesRead += r; }
	}
	return lCar(v);
}

static lVal *lnfFileWriteAst(lClosure *c, lVal *v){
	FILE *fh = requireFileHandle(c, lCar(v));
	lVal *contentV = lCadr(v);
	i64 size = castToInt(lCaddr(v), -1);
	const i64 offset = castToInt(lCadddr(v), 0);

	const void * buf = NULL;
	i64 bufSize = 0;
	i64 bytesWritten = 0;
	typeswitch(contentV){
	default:
		lExceptionThrowValClo("type-error", "Can't read into that", contentV, c);
		return NULL;
	case ltString:
		buf = lStringData(contentV->vBuffer);
		bufSize = lStringLength(contentV->vBuffer);
		break;
	case ltBuffer:
		buf = lBufferData(contentV->vBuffer);
		bufSize = lBufferLength(contentV->vBuffer);
		break;
	case ltBufferView:{
		buf = lBufferViewData(contentV->vBufferView);
		bufSize = lBufferViewLength(contentV->vBufferView);
		break;
	}}
	if(buf == NULL){
		lExceptionThrowValClo("type-error", "Can't read into an immutable buffer", contentV, c);
	}
	if(size < 0){
		size = bufSize - offset;
	}
	if((bufSize - offset) < size){
		lExceptionThrowValClo("type-error", "Buffer is too small for that read operation", contentV, c);
	}
	while(bytesWritten < size){
		const int r = fwrite(&((u8 *)buf)[offset + bytesWritten], 1, size - bytesWritten, fh);
		if(ferror(fh)){
			lExceptionThrowValClo("io-error", "IO Error occured during write", lCar(v), c);
		}
		if(r > 0){ bytesWritten += r; }
	}

	return lCar(v);
}

static lVal *lnfFileFlush(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	FILE *fh = requireFileHandle(c, car);
	fflush(fh);
	return car;
}

static lVal *lnfFileTell(lClosure *c, lVal *v){
	FILE *fh = requireFileHandle(c, lCar(v));
	const i64 pos = ftell(fh);
	return lValInt(pos);
}

static lVal *lnfFileSeek(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	FILE *fh = requireFileHandle(c, car);
	const i64 offset = requireInt(c, lCadr(v));
	const i64 whenceRaw = requireInt(c, lCaddr(v));
	if((whenceRaw < 0) || (whenceRaw > 2)){
		lExceptionThrowValClo("type-error", "Whence has to be in the range 0-2", lCaddr(v), c);
	}
	const int whence = whenceRaw == 0 ? SEEK_SET : whenceRaw == 1 ? SEEK_CUR : SEEK_END;
	fseek(fh, offset, whence);
	return car;
}

static lVal *lnfFileEof(lClosure *c, lVal *v){
	return lValBool(feof(requireFileHandle(c, lCar(v))));
}

static lVal *lnfFileError(lClosure *c, lVal *v){
	return lValBool(ferror(requireFileHandle(c, lCar(v))));
}

void lOperationsPort(lClosure *c){
	lAddNativeFunc(c,"file/open-output*", "(pathname if-exists)",   "Try to open PATHNAME for MODE",        lnfFileOpenOutput);
	lAddNativeFunc(c,"file/open-input*",  "(pathname)",             "Try to open PATHNAME for MODE",        lnfFileOpenInput);
	lAddNativeFunc(c,"file/close*",  "(handle)",                    "Close the open HANDLE",                lnfFileClose);
	lAddNativeFunc(c,"file/read*",   "(handle buffer size offset)", "Reader from HANDLE into BUFFER",       lnfFileReadAst);
	lAddNativeFunc(c,"file/write*",  "(handle buffer size offset)", "Write BUFFER into HANDLE",             lnfFileWriteAst);
	lAddNativeFunc(c,"file/flush*",  "(handle)",                    "Flush stream of HANDLE",               lnfFileFlush);
	lAddNativeFunc(c,"file/tell*",   "(handle)",                    "Return the stream position of HANDLE", lnfFileTell);
	lAddNativeFunc(c,"file/seek*",   "(handle offset whence)",      "Seek stream of HANDLE to OFFSET from WHENCE where 0 is SEEK_SET, 1 is SEEK_CUR and 2 is SEEK_END", lnfFileSeek);
	lAddNativeFunc(c,"file/eof*?",   "(handle)",                    "Return whether the end-of-file indicator is set for HANDLE", lnfFileEof);
	lAddNativeFunc(c,"file/error*?", "(handle)",                    "Return whether the error indicator is set for HANDLE", lnfFileError);
#if (!defined(_WIN32))
	lAddNativeFunc(c,"file/raw*", "(handle)",                    "Set an input stream into raw mode", lnfFileRaw);
#endif
}

void lOperationsInit(lClosure *c){
	lDefineVal(c, "stdin*",  lValFileHandle(stdin));
	lDefineVal(c, "stdout*", lValFileHandle(stdout));
	lDefineVal(c, "stderr*", lValFileHandle(stderr));
}
