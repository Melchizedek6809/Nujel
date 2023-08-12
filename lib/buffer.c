/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

const void *lBufferData (lBuffer *v){
	return v == NULL ? NULL : v->buf;
}

void *lBufferDataMutable(lBuffer *v){
	if(v->flags & BUFFER_IMMUTABLE){return NULL;}
	return v == NULL ? NULL : v->buf;
}

const char *lStringData(const lString *v){
	return v == NULL ? NULL : v->data;
}

size_t lBufferLength(const lBuffer *v){
	return v == NULL ? 0 : v->length;
}

const void *lBufferViewData(lBufferView *v){
	if(unlikely(v == NULL)){return NULL;}
	if(unlikely(v->buf->flags & BUFFER_IMMUTABLE)){return NULL;}
	return &((u8 *)v->buf->buf)[v->offset * lBufferViewTypeSize(v->type)];
}

void *lBufferViewDataMutable(lBufferView *v){
	return &((u8 *)v->buf->buf)[v->offset * lBufferViewTypeSize(v->type)];
}

size_t lBufferViewLength(const lBufferView *v){
	return v->length * lBufferViewTypeSize(v->type);
}

static lVal lnfBufferAllocate(lVal a) {
	lVal car = requireNaturalInt(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	const i64 size = car.vInt;
	lBuffer *buf = lBufferAlloc(size, false);
	return lValAlloc(ltBuffer, buf);
}

static lVal bufferFromPointer(bool immutable, const void *data, size_t length){
	lBuffer *retBuf = lBufferAllocRaw();
	retBuf->length = length;
	retBuf->flags = immutable ? BUFFER_IMMUTABLE : 0;
	retBuf->buf = malloc(length);
	if (unlikely(retBuf->buf == NULL)) {
		return lValException(lSymOOM, "(buffer/length!) couldn't allocate a buffer", NIL);
	}
	memcpy(retBuf->buf, data, length);

	return lValAlloc(ltBuffer, retBuf);
}

static lVal lnfBufferCopy(lVal aDest, lVal vSrc, lVal aDestOffset, lVal aLength){
	lVal car = requireMutableBuffer(aDest);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBuffer *dest = car.vBuffer;
	lVal offV = requireNaturalInt(aDestOffset);
	if(unlikely(offV.type == ltException)){
		return offV;
	}
	const int destOffset = offV.vInt;
	const void *buf = NULL;
	int length = 0;

	switch(vSrc.type){
	case ltString:
	case ltBuffer:
		buf = vSrc.vBuffer->data;
		length = castToInt(aLength, vSrc.vBuffer->length);
		break;
	}

	if(unlikely((buf == NULL) || (length < 0))){
		return lValException(lSymTypeError, "Can't copy from that", vSrc);
	}
	if(unlikely(((length + destOffset) > dest->length) || (length > vSrc.vBuffer->length))){
		return lValException(lSymOutOfBounds, "Can't fit everything in that buffer", vSrc);
	}
	memcpy(&((u8*)dest->buf)[destOffset], buf, length);
	return car;
}

static lVal lnfBufferToString(lVal a, lVal aLength){
	lVal car = requireBuffer(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBuffer *buf = car.vBuffer;
	const i64 length = castToInt(aLength, buf->length);
	if(unlikely(length < 0)){
		return lValException(lSymTypeError, "Length has to be greater than 0", aLength);
	}
	return lValStringLen(buf->buf, length);
}

static lVal bufferView(lVal a, lVal aImmutable, lBufferViewType T){
	lVal car = requireBuffer(a);
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBuffer *buf = car.vBuffer;
	bool immutable = castToBool(aImmutable);
	if(unlikely(!immutable && (buf->flags & BUFFER_IMMUTABLE))){
		if(aImmutable.type == ltBool){
			return lValException(lSymTypeError, "Can't create a mutable view for an immutable buffer", a);
		} else {
			immutable = true;
		}
	}
	const size_t length = buf->length / lBufferViewTypeSize(T);
	lBufferView *bufView = lBufferViewAlloc(buf, T, 0, length, immutable);
	return lValAlloc(ltBufferView, bufView);
}

static lVal  lnmBufferU8(lVal a, lVal aImmutable){ return bufferView(a, aImmutable,  lbvtU8); }
static lVal  lnmBufferS8(lVal a, lVal aImmutable){ return bufferView(a, aImmutable,  lbvtS8); }
static lVal lnmBufferU16(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtU16); }
static lVal lnmBufferS16(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtS16); }
static lVal lnmBufferU32(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtU32); }
static lVal lnmBufferS32(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtS32); }
static lVal lnmBufferF32(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtF32); }
static lVal lnmBufferS64(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtS64); }
static lVal lnmBufferF64(lVal a, lVal aImmutable){ return bufferView(a, aImmutable, lbvtF64); }

static lVal lnmBufferViewBuffer(lVal self){
	return lValAlloc(ltBuffer, self.vBufferView->buf);
}

static lVal lnmBufferLength(lVal self){
	return lValInt(self.vBuffer->length);
}

static lVal lnmBufferViewLength(lVal self){
	return lValInt(self.vBufferView->length);
}

static lVal lnmBufferLengthSet(lVal self, lVal newLength){
	if(unlikely(self.vBuffer->flags & BUFFER_IMMUTABLE)){
		return lValException(lSymTypeError, ":length! requires a mutable buffer", self);
	}
	lBuffer *buf = self.vBuffer;

	lVal lenVal = requireNaturalInt(newLength);
	if(unlikely(lenVal.type == ltException)){
		return lenVal;
	}
	const int length = lenVal.vInt;
	if(length < buf->length){
		return lValException(lSymOutOfBounds, "Buffers can only grow, not shrink.", self);
	}
	void *nBuf = realloc(buf->buf, length);
	if (unlikely(nBuf == NULL)) {
		return lValException(lSymOOM, "(buffer/length!) couldn't allocate its buffer", self);
	}
	memset(&((u8 *)nBuf)[buf->length], 0, length - buf->length);
	buf->buf = nBuf;
	buf->length = length;
	return self;
}

static lVal lnmBufferImmutable(lVal self){
	return lValBool(self.vBuffer->flags & BUFFER_IMMUTABLE);
}

static lVal lnmBufferViewImmutable(lVal self){
	return lValBool(self.vBufferView->flags & BUFFER_VIEW_IMMUTABLE);
}

static lVal lnmBufferClone(lVal self, lVal immutable){
	return bufferFromPointer(castToBool(immutable), self.vBuffer->buf, self.vBuffer->length);
}

void lOperationsBuffer(lClosure *c){
	lClass *Buffer = &lClassList[ltBuffer];
	lAddNativeMethodV (Buffer, lSymS("length"),     "(self)", lnmBufferLength, NFUNC_PURE);
	lAddNativeMethodV (Buffer, lSymS("immutable?"), "(self)", lnmBufferImmutable, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("length!"),    "(self new-length)", lnmBufferLengthSet, 0);
	lAddNativeMethodVV(Buffer, lSymS("clone"),      "(self immutable?)", lnmBufferClone, NFUNC_PURE);

	lAddNativeMethodVV(Buffer, lSymS("u8"),  "(buf immutable?)", lnmBufferU8, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("s8"),  "(buf immutable?)", lnmBufferS8, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("u16"), "(buf immutable?)", lnmBufferU16, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("s16"), "(buf immutable?)", lnmBufferS16, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("u32"), "(buf immutable?)", lnmBufferU32, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("s32"), "(buf immutable?)", lnmBufferS32, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("f32"), "(buf immutable?)", lnmBufferF32, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("s64"), "(buf immutable?)", lnmBufferS64, NFUNC_PURE);
	lAddNativeMethodVV(Buffer, lSymS("f64"), "(buf immutable?)", lnmBufferF64, NFUNC_PURE);

	lClass *BufferView = &lClassList[ltBufferView];
	lAddNativeMethodV (BufferView, lSymS("length"),     "(self)", lnmBufferViewLength, NFUNC_PURE);
	lAddNativeMethodV (BufferView, lSymS("buffer"),     "(self)", lnmBufferViewBuffer, NFUNC_PURE);
	lAddNativeMethodV (BufferView, lSymS("immutable?"), "(self)", lnmBufferViewImmutable, NFUNC_PURE);

	lAddNativeFuncV   (c, "buffer/allocate", "(length)",         "Allocate a new buffer of LENGTH",   lnfBufferAllocate, 0);
	lAddNativeFuncVVVV(c, "buffer/copy",     "(dest src dest-offset length)","Return a copy of BUF that might be IMMUTABLE", lnfBufferCopy, 0);
	lAddNativeFuncVV  (c, "buffer->string",  "(buf length)",     "Turn BUF into a string of LENGTH which defaults to the size of BUF", lnfBufferToString, 0);
}
