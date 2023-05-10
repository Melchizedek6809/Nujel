/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

static lVal *lBufferViewRef(lClosure *c, lVal *car, size_t i){
	const void *buf = car->vBufferView->buf->buf;
	const size_t length = car->vBufferView->length;
	const lBufferViewType viewType = car->vBufferView->type;

	if(unlikely(buf == NULL)){
		lExceptionThrowValClo("type-error", "Can't ref that", car, c);
	}
	if(unlikely(i >= length)){
		lExceptionThrowValClo("out-of-bounds","(buffer/view/ref] index provided is out of bounds", car, c);
	}

	switch(viewType){
	default:
		exit(5);
		return NULL;
	case lbvtU8:
		return lValInt(((u8 *)buf)[i]);
	case lbvtS8:
		return lValInt(((i8 *)buf)[i]);
	case lbvtU16:
		return lValInt(((u16 *)buf)[i]);
	case lbvtS16:
		return lValInt(((i16 *)buf)[i]);
	case lbvtU32:
		return lValInt(((u32 *)buf)[i]);
	case lbvtS32:
		return lValInt(((i32 *)buf)[i]);
	case lbvtS64:
		return lValInt(((i64 *)buf)[i]);
	case lbvtF32:
		return lValFloat(c, ((float *)buf)[i]);
	case lbvtF64:
		return lValFloat(c, ((double *)buf)[i]);
	}
}

static lVal *lnfGenericRef(lClosure *c, lVal *v){
	lVal *col = lCar(v);
	typeswitch(col){
	case ltPair: {
		const int index = requireNaturalInt(c, lCadr(v));
		for(int i=0;i<index;i++){
			col = lCdr(col);
		}
		return lCar(col); }
	case ltBytecodeArr: {
		const lBytecodeArray *arr = requireBytecodeArray(c, col);
		const int i = requireNaturalInt(c, lCadr(v));
		if(unlikely((arr->data + i) >= arr->dataEnd)){
			lExceptionThrowValClo("out-of-bounds","(ref) bytecode-array index provided is out of bounds", v, c);
		}
		return lValBytecodeOp(arr->data[i]); }
	case ltArray: {
		lArray *arr = requireArray(c, col);
		const int i = requireNaturalInt(c, lCadr(v));
		if(unlikely(arr->length <= i)){
			lExceptionThrowValClo("out-of-bounds","(ref) array index provided is out of bounds", v, c);
		}
		return arr->data[i]; }
	case ltString:
	case ltBuffer: {
		const char *buf = col->vBuffer->data;
		const size_t len = col->vBuffer->length;
		const size_t i = requireNaturalInt(c, lCadr(v));
		if(unlikely(len <= i)){
			lExceptionThrowValClo("out-of-bounds","(ref) buffer index provided is out of bounds", v, c);
		}
		return lValInt(buf[i]); }
	case ltBufferView:
		return lBufferViewRef(c, col, requireNaturalInt(c, lCadr(v)));
	case ltTree:
		return lTreeGet(requireTree(c, lCar(v)), requireSymbolic(c, lCadr(v)), NULL);
	default:
		lExceptionThrowValClo("type-error", "Can't ref that", col, c);
		return NULL;
	}
}

void lOperationsGeneric(lClosure *c){
	lAddNativeFuncPure(c,"ref","(collection key)", "Look up key in collection", lnfGenericRef);
}
