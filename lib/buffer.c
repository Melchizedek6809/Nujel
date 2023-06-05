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

static lVal lnfBufferAllocate(lClosure *c, lVal v) {
	(void)c;
	lVal car = requireNaturalInt(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	const i64 size = car.vInt;
	lBuffer *buf = lBufferAlloc(size, false);
	return lValAlloc(ltBuffer, buf);
}

static lVal lnfBufferLengthGet(lClosure *c, lVal v){
	(void)c;
	lVal car = lCar(v);
	switch(car.type){
	default:
		return lValException("type-error", "Expected a buffer or something compatible", car);
	case ltString:
	case ltBuffer:
		return lValInt(car.vBuffer->length);
	case ltBufferView:
		return lValInt(car.vBufferView->length);
	}
}

static lVal lnfBufferLengthSet(lClosure *c, lVal v){
	(void)c;
	lVal bufv = requireMutableBuffer(lCar(v));
	if(unlikely(bufv.type == ltException)){
		return bufv;
	}
	lBuffer *buf = bufv.vBuffer;

	lVal lenVal = requireNaturalInt(lCadr(v));
	if(unlikely(lenVal.type == ltException)){
		return lenVal;
	}
	const int length = lenVal.vInt;
	if(length < buf->length){
		return lValException("error", "Buffers can only grow, not shrink.", v);
	}
	void *nBuf = realloc(buf->buf, length);
	if (unlikely(nBuf == NULL)) {
		return lValException("out-of-memory", "(buffer/length!) couldn't allocate its buffer", v);
	}
	memset(&((u8 *)nBuf)[buf->length], 0, length - buf->length);
	buf->buf = nBuf;
	buf->length = length;
	return bufv;
}

static lVal lnfBufferImmutableGet(lClosure *c, lVal v){
	(void)c;
	lVal car = lCar(v);
	switch(car.type){
	case ltBufferView:
		return lValBool(car.vBufferView->flags & BUFFER_VIEW_IMMUTABLE);
	case ltString:
		return lValBool(true);
	case ltBuffer:
		return lValBool(car.vBuffer->flags & BUFFER_IMMUTABLE);
	default:
		return lValException("type-error", "Can't get immutability info from that: ", car);
	}
}

static lVal bufferFromPointer(lClosure *c, bool immutable, const void *data, size_t length){
	(void)c;
	lBuffer *retBuf = lBufferAllocRaw();
	retBuf->length = length;
	retBuf->flags = immutable ? BUFFER_IMMUTABLE : 0;
	retBuf->buf = malloc(length);
	if (unlikely(retBuf->buf == NULL)) {
		return lValException("out-of-memory", "(buffer/length!) couldn't allocate a buffer", NIL);
	}
	memcpy(retBuf->buf, data, length);

	return lValAlloc(ltBuffer, retBuf);
}

static lVal lnfBufferDup(lClosure *c, lVal v){
	lVal vBuf = requireBuffer(lCar(v));
	if(unlikely(vBuf.type == ltException)){
		return vBuf;
	}
	lBuffer *buf = vBuf.vBuffer;
	return bufferFromPointer(c, castToBool(lCadr(v)), buf->buf, buf->length);
}

static lVal lnfBufferCopy(lClosure *c, lVal v){
	(void)c;
	lVal car = requireMutableBuffer(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBuffer *dest = car.vBuffer;
	lVal vSrc = lCadr(v);
	lVal offV = requireNaturalInt(lCaddr(v));
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
		length = castToInt(lCadddr(v), vSrc.vBuffer->length);
		break;
	}

	if(unlikely((buf == NULL) || (length < 0))){
		return lValException("type-error", "Can't copy from that", vSrc);
	}
	if(unlikely(((length + destOffset) > dest->length) || (length > vSrc.vBuffer->length))){
		return lValException("out-of-bounds", "Can't fit everything in that buffer", v);
	}
	memcpy(&((u8*)dest->buf)[destOffset], buf, length);
	return car;
}

static lVal lnfStringToBuffer(lClosure *c, lVal v){
	lVal car = requireString(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lString *str = car.vString;
	return bufferFromPointer(c, castToBool(lCadr(v)), str->data, str->length);
}

static lVal lnfBufferToString(lClosure *c, lVal v){
	(void)c;
	lVal car = requireBuffer(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBuffer *buf = car.vBuffer;
	const i64 length = castToInt(lCadr(v), buf->length);
	if(unlikely(length < 0)){
		return lValException("type-error", "Length has to be greater than 0", lCadr(v));
	}
	return lValStringLen(buf->buf, length);
}

static lVal bufferView(lClosure *c, lVal v, lBufferViewType T){
	(void)c;
	lVal car = requireBuffer(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBuffer *buf = car.vBuffer;
	const bool immutable = lCdr(v).type != ltNil ? castToBool(lCadr(v)) : buf->flags & BUFFER_IMMUTABLE;
	if(unlikely(!immutable && (buf->flags & BUFFER_IMMUTABLE))){
		return lValException("type-error", "Can't create a mutable view for an immutable buffer", v);
	}
	const size_t length = buf->length / lBufferViewTypeSize(T);
	lBufferView *bufView = lBufferViewAlloc(buf, T, 0, length, immutable);
	return lValAlloc(ltBufferView, bufView);
}

static lVal  lnfBufferViewU8(lClosure *c, lVal v){ return bufferView(c, v,  lbvtU8); }
static lVal  lnfBufferViewS8(lClosure *c, lVal v){ return bufferView(c, v,  lbvtS8); }
static lVal lnfBufferViewU16(lClosure *c, lVal v){ return bufferView(c, v, lbvtU16); }
static lVal lnfBufferViewS16(lClosure *c, lVal v){ return bufferView(c, v, lbvtS16); }
static lVal lnfBufferViewU32(lClosure *c, lVal v){ return bufferView(c, v, lbvtU32); }
static lVal lnfBufferViewS32(lClosure *c, lVal v){ return bufferView(c, v, lbvtS32); }
static lVal lnfBufferViewF32(lClosure *c, lVal v){ return bufferView(c, v, lbvtF32); }
static lVal lnfBufferViewS64(lClosure *c, lVal v){ return bufferView(c, v, lbvtS64); }
static lVal lnfBufferViewF64(lClosure *c, lVal v){ return bufferView(c, v, lbvtF64); }

static lVal lnfBufferViewSet(lClosure *c, lVal v){
	(void)c;
	lVal car = lCar(v);
	void *buf = NULL;
	size_t length = 0;
	lBufferViewType viewType;
	lVal iv = requireNaturalInt(lCadr(v));
	if(unlikely(iv.type == ltException)){
		return iv;
	}
	const size_t i = iv.vInt;

	switch(car.type){
	case ltBufferView:
		buf      = car.vBufferView->buf->buf;
		length   = car.vBufferView->length;
		viewType = car.vBufferView->type;
		break;
	case ltBuffer:
		if(car.vBuffer->flags & BUFFER_IMMUTABLE){break;}
		buf      = car.vBuffer->buf;
		length   = car.vBuffer->length;
		viewType = lbvtU8;
		break;
	}

	if(unlikely(buf == NULL)){
		return lValException("type-error", "Can't set! in that", car);
	}
	if(unlikely(i >= length)){
		return lValException("out-of-bounds","(buffer/set!] index provided is out of bounds", v);
	}

	switch(viewType){
	default:
		exit(6);
	case lbvtU8: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((u8 *)buf)[i] = val.vInt;
		break; }
	case lbvtS8: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((i8 *)buf)[i] = val.vInt;
		break; }
	case lbvtS16: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((i16 *)buf)[i] = val.vInt;
		break; }
	case lbvtU16: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((u16 *)buf)[i] = val.vInt;
		break; }
	case lbvtS32: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((i32 *)buf)[i] = val.vInt;
		break; }
	case lbvtU32: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((u32 *)buf)[i] = val.vInt;
		break; }
	case lbvtS64: {
		lVal val = requireInt(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((i64 *)buf)[i] = val.vInt;
		break; }
	case lbvtF32: {
		lVal val = requireFloat(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((float *)buf)[i] = val.vFloat;
		break; }
	case lbvtF64: {
		lVal val = requireFloat(lCaddr(v));
		if(unlikely(val.type == ltException)){
			return val;
		}
		((double *)buf)[i] = val.vFloat;
		break; }
	}
	return car;
}

static lVal lnfBufferViewBuffer(lClosure *c, lVal v){
	(void)c;
	lVal car = requireBufferView(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lBufferView *view = car.vBufferView;
	return lValAlloc(ltBuffer, view->buf);
}

void lOperationsBuffer(lClosure *c){
	lAddNativeFunc(c, "buffer/allocate",        "(length)",         "Allocate a new buffer of LENGTH",   lnfBufferAllocate);
	lAddNativeFunc(c, "buffer/length!",         "(buf new-length)", "Set the size of BUF to NEW-LENGTH", lnfBufferLengthSet);
	lAddNativeFunc(c, "buffer/dup",             "(buf immutable?)", "Return a copy of BUF that might be IMMUTABLE", lnfBufferDup);
	lAddNativeFunc(c, "buffer/copy",            "(dest src dest-offset length)","Return a copy of BUF that might be IMMUTABLE", lnfBufferCopy);

	lAddNativeFunc(c, "string->buffer",         "(str immutable?)", "Copy STR into a buffer and return it", lnfStringToBuffer);
	lAddNativeFunc(c, "buffer->string",         "(buf length)",     "Turn BUF into a string of LENGTH which defaults to the size of BUF", lnfBufferToString);

	lAddNativeFunc(c, "buffer/u8*",             "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewU8);
	lAddNativeFunc(c, "buffer/s8*",             "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewS8);
	lAddNativeFunc(c, "buffer/u16*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewU16);
	lAddNativeFunc(c, "buffer/s16*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewS16);
	lAddNativeFunc(c, "buffer/u32*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewU32);
	lAddNativeFunc(c, "buffer/s32*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewS32);
	lAddNativeFunc(c, "buffer/f32*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewF32);
	lAddNativeFunc(c, "buffer/s64*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewS64);
	lAddNativeFunc(c, "buffer/f64*",            "(buf immutable?)", "Create a new view for BUF spanning the entire area", lnfBufferViewF64);
	lAddNativeFunc(c, "buffer-view->buffer",    "(view)",           "Return the buffer of VIEW", lnfBufferViewBuffer);

	lAddNativeFunc(c, "buffer/set!",            "(view off val)",   "Set the value of VIEW at OFF to VAL", lnfBufferViewSet);
	lAddNativeFunc(c, "buffer/length",          "(buf)",            "Return the size of BUF in bytes",   lnfBufferLengthGet);
	lAddNativeFunc(c, "buffer/immutable?",      "(buf)",            "Return #t if BUF is immutable",     lnfBufferImmutableGet);
}
