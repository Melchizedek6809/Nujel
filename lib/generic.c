/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

static lVal lBufferViewRef(lVal car, size_t i){
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

lVal lGenericRef(lVal col, lVal key){
	switch(col.type){
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
		const lBytecodeArray *arr = col.vBytecodeArr;
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
		lArray *arr = col.vArray;
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
		return lBufferViewRef(col, t.vInt); }
	case ltTree: {
		if(unlikely((key.type != ltSymbol) && (key.type != ltKeyword))){
			return lValExceptionType(col, ltKeyword);
		}
		lVal r = lTreeRef(col.vTree->root, key.vSymbol);
		return r.type != ltException ? r : NIL; }
	default:
		return lValException("type-error", "Can't ref that", col);
	}
}


static lVal lBufferViewSet(lVal car, size_t i, lVal v){
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
	case lbvtU8: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((u8 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtS8: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((i8 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtU16: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((u16 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtS16: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((i16 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtU32: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((u32 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtS32: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((i32 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtS64: {
		const lVal nv = requireInt(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((i64 *)buf)[i] = nv.vInt;
		return car; }
	case lbvtF32: {
		const lVal nv = requireFloat(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((float *)buf)[i] = nv.vFloat;
		return car; }
	case lbvtF64: {
		const lVal nv = requireFloat(v);
		if(unlikely(nv.type == ltException)){
			return nv;
		}
		((double *)buf)[i] = nv.vFloat;
		return car; }
	}
}

lVal lGenericSet(lVal col, lVal key, lVal v){
	switch(col.type){
	case ltBytecodeArr: {
		const lBytecodeArray *arr = col.vBytecodeArr;
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const int i = keyVal.vInt;
		if(unlikely((arr->data + i) >= arr->dataEnd)){
			return lValException("out-of-bounds","(ref) bytecode-array index provided is out of bounds", col);
		}
		if(unlikely((v.type != ltInt))){
			return lValException("type-error", "Can't set! a non int value into a BytecodeArray", v);
		}
		arr->data[i] = v.vInt;
		return col; }
	case ltArray: {
		lArray *arr = col.vArray;
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const int i = keyVal.vInt;
		if(unlikely(arr->length <= i)){
			return lValException("out-of-bounds","(ref) array index provided is out of bounds", col);
		}
		arr->data[i] = v;
		return col; }
	case ltBuffer: {
		char *buf  = col.vBuffer->buf;
		const size_t len = col.vBuffer->length;
		lVal keyVal = requireNaturalInt(key);
		if(unlikely(keyVal.type == ltException)){
			return keyVal;
		}
		const size_t i = keyVal.vInt;
		if(unlikely(len <= i)){
			return lValException("out-of-bounds","(ref) buffer index provided is out of bounds", col);
		}
		if(unlikely((v.type != ltInt))){
			return lValException("type-error", "Can't set! a non int value into a BytecodeArray", v);
		}
		buf[i] = v.vInt;
		return col; }
	case ltBufferView: {
		lVal t = requireNaturalInt(key);
		if(unlikely(t.type == ltException)){
			return t;
		}
		return lBufferViewSet(col, t.vInt, v); }
	case ltTree: {
		if(unlikely((key.type != ltSymbol) && (key.type != ltKeyword))){
			return lValExceptionType(col, ltKeyword);
		}
		if(unlikely(col.vTree->root && col.vTree->root->flags & TREE_IMMUTABLE)){
			return lValException("type-error", "Can only set! mutable trees", col);
		}
		col.vTree->root = lTreeInsert(col.vTree->root, key.vSymbol, v);
		return col; }
	default:
		return lValException("type-error", "Can't set! that", col);
	}
}
