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
		return lValException(lSymTypeError, "Can't ref that", car);
	}
	if(unlikely(i >= length)){
		return lValException(lSymOutOfBounds, "ref - index provided is out of bounds", car);
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
	case ltMap:
		return lMapRef(col.vMap, key);
	case ltPair:
		reqNaturalInt(key);
		for(int i=0;i<key.vInt;i++){
			col = lCdr(col);
		}
		return lCar(col);
	case ltBytecodeArr: {
		const lBytecodeArray *arr = col.vBytecodeArr;
		reqNaturalInt(key);
		const int i = key.vInt;
		if(unlikely((arr->data + i) >= arr->dataEnd)){
			return lValException(lSymOutOfBounds, "(ref) bytecode-array index provided is out of bounds", col);
		}
		return lValInt(arr->data[i]); }
	case ltArray: {
		lArray *arr = col.vArray;
		reqNaturalInt(key);
		const int i = key.vInt;
		if(unlikely(arr->length <= i)){
			return lValException(lSymOutOfBounds, "(ref) array index provided is out of bounds", col);
		}
		return arr->data[i]; }
	case ltString:
	case ltBuffer: {
		const uint8_t *buf = (uint8_t *)col.vBuffer->data;
		const size_t len = col.vBuffer->length;
		reqNaturalInt(key);
		const size_t i = key.vInt;
		if(unlikely(len <= i)){
			return lValException(lSymOutOfBounds, "(ref) buffer index provided is out of bounds", col);
		}
		return lValInt(buf[i]); }
	case ltBufferView:
		reqNaturalInt(key);
		return lBufferViewRef(col, key.vInt);
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
		if(unlikely((key.type != ltSymbol) && (key.type != ltKeyword))){
			return lValExceptionType(col, ltKeyword);
		}
		return lGetClosureSym(col.vClosure, key.vSymbol);
	case ltTree: {
		if(unlikely((key.type != ltSymbol) && (key.type != ltKeyword))){
			return lValExceptionType(col, ltKeyword);
		}
		lVal r = lTreeRef(col.vTree->root, key.vSymbol);
		return r.type != ltException ? r : NIL; }
	default:
		return lValException(lSymTypeError, "Can't ref that", col);
	}
}


static lVal lBufferViewSet(lVal car, size_t i, lVal v){
	const void *buf = car.vBufferView->buf->buf;
	const size_t length = car.vBufferView->length;
	const lBufferViewType viewType = car.vBufferView->type;

	if(unlikely(buf == NULL)){
		return lValException(lSymTypeError, "Can't ref that", car);
	}
	if(unlikely(i >= length)){
		return lValException(lSymOutOfBounds, "ref - index provided is out of bounds", car);
	}

	switch(viewType){
	default:
		exit(5);
		return NIL;
	case lbvtU8:
		reqInt(v);
		((u8 *)buf)[i] = v.vInt;
		return car;
	case lbvtS8:
		reqInt(v);
		((i8 *)buf)[i] = v.vInt;
		return car;
	case lbvtU16:
		reqInt(v);
		((u16 *)buf)[i] = v.vInt;
		return car;
	case lbvtS16:
		reqInt(v);
		((i16 *)buf)[i] = v.vInt;
		return car;
	case lbvtU32:
		reqInt(v);
		((u32 *)buf)[i] = v.vInt;
		return car;
	case lbvtS32:
		reqInt(v);
		((i32 *)buf)[i] = v.vInt;
		return car;
	case lbvtS64:
		reqInt(v);
		((i64 *)buf)[i] = v.vInt;
		return car;
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
	case ltMap:
		return lMapSet(col.vMap, key, v);
	case ltBytecodeArr: {
		const lBytecodeArray *arr = col.vBytecodeArr;
		reqNaturalInt(key);
		const int i = key.vInt;
		if(unlikely((arr->data + i) >= arr->dataEnd)){
			return lValException(lSymOutOfBounds, "(ref) bytecode-array index provided is out of bounds", col);
		}
		if(unlikely((v.type != ltInt))){
			return lValException(lSymTypeError, "Can't set! a non int value into a BytecodeArray", v);
		}
		arr->data[i] = v.vInt;
		return col; }
	case ltArray: {
		lArray *arr = col.vArray;
		reqNaturalInt(key);
		const int i = key.vInt;
		if(unlikely(arr->length <= i)){
			return lValException(lSymOutOfBounds, "(ref) array index provided is out of bounds", col);
		}
		arr->data[i] = v;
		return col; }
	case ltBuffer: {
		char *buf  = col.vBuffer->buf;
		const size_t len = col.vBuffer->length;
		reqNaturalInt(key);
		const size_t i = key.vInt;
		if(unlikely(len <= i)){
			return lValException(lSymOutOfBounds, "(ref) buffer index provided is out of bounds", col);
		}
		if(unlikely((v.type != ltInt))){
			return lValException(lSymTypeError, "Can't set! a non int value into a BytecodeArray", v);
		}
		buf[i] = v.vInt;
		return col; }
	case ltBufferView:
		reqNaturalInt(key);
		return lBufferViewSet(col, key.vInt, v);
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
		if(unlikely((key.type != ltSymbol) && (key.type != ltKeyword))){
			return lValExceptionType(col, ltKeyword);
		}
		lDefineClosureSym(col.vClosure, key.vSymbol, v);
		return col;
	case ltTree:
		if(unlikely((key.type != ltSymbol) && (key.type != ltKeyword))){
			return lValExceptionType(col, ltKeyword);
		}
		if(unlikely(col.vTree->root && col.vTree->root->flags & TREE_IMMUTABLE)){
			return lValException(lSymTypeError, "Can only set! mutable trees", col);
		}
		col.vTree->root = lTreeInsert(col.vTree->root, key.vSymbol, v);
		return col;
	default:
		return lValException(lSymTypeError, "Can't set! that", col);
	}
}
