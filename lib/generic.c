/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

static lVal lBufferViewRef(lClosure *c, lVal car, size_t i){
	(void)c;
	const void *buf = car.vBufferView->buf->buf;
	const size_t length = car.vBufferView->length;
	const lBufferViewType viewType = car.vBufferView->type;

	if(unlikely(buf == NULL)){
		return lValException("type-error", "Can't ref that", car);
	}
	if(unlikely(i >= length)){
		return lValException("out-of-bounds","ref - index provided is out of bounds", car);
	}

	switch(viewType){
	default:
		exit(5);
		return NIL;
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
		return lValFloat(((float *)buf)[i]);
	case lbvtF64:
		return lValFloat(((double *)buf)[i]);
	}
}

lVal lGenericRef(lClosure *c, lVal col, lVal key){
	typeswitch(col){
	case ltPair: {
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const int index = keyVal.vInt;
		for(int i=0;i<index;i++){
			col = lCdr(col);
		}
		return lCar(col); }
	case ltBytecodeArr: {
		lVal bc = requireBytecodeArray(col);
		if(unlikely(bc.type == ltException)){
			return bc;
		}
		const lBytecodeArray *arr = bc.vBytecodeArr;
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const int i = keyVal.vInt;
		if(unlikely((arr->data + i) >= arr->dataEnd)){
			return lValException("out-of-bounds","(ref) bytecode-array index provided is out of bounds", col);
		}
		return lValInt(arr->data[i]); }
	case ltArray: {
		lVal t = requireArray(col);
		if(unlikely(t.type == ltException)){
			return t;
		}
		lArray *arr = t.vArray;
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const int i = keyVal.vInt;
		if(unlikely(arr->length <= i)){
			return lValException("out-of-bounds","(ref) array index provided is out of bounds", col);
		}
		return arr->data[i]; }
	case ltString:
	case ltBuffer: {
		const char *buf  = col.vBuffer->data;
		const size_t len = col.vBuffer->length;
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const size_t i = keyVal.vInt;
		if(unlikely(len <= i)){
			return lValException("out-of-bounds","(ref) buffer index provided is out of bounds", col);
		}
		return lValInt(buf[i]); }
	case ltBufferView: {
		lVal t = requireNaturalInt(key);
		if(unlikely(t.type == ltException)){
			return t;
		}
		return lBufferViewRef(c, col, t.vInt); }
	case ltTree: {
		lVal t = requireTree(col);
		if(unlikely(t.type == ltException)){
			return t;
		}
		lVal tk = requireSymbolic(key);
		if(unlikely(tk.type == ltException)){
			return tk;
		}
		return lTreeGet(t.vTree->root, tk.vSymbol, NULL); }
	default:
		return lValException("type-error", "Can't ref that", col);
	}
}
