/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include "../third-party/fasthash/fasthash.h"

static const u64 hashSeed = 0x5b0a159d9eac0381ULL;

u32 lHashString(const char *str, i32 len){
	return fasthash64(str, len, hashSeed);
}

static inline u32 lHashVal(lVal v){
	switch(v.type){
	case ltType:
	case ltFileHandle:
	case ltBytecodeArr:
	case ltBufferView:
	case ltEnvironment:
	case ltNativeFunc:
	case ltMacro:
	case ltLambda:
	case ltMap:
	case ltTree:
	case ltArray:
	case ltPair:
		#if INTPTR_MAX == INT32_MAX
		return fasthash32v((u32)v.vPointer);
		#else
		return fasthash64v((u64)v.vPointer);
		#endif

	case ltString:
		return lHashString(v.vString->data, v.vString->length);
	case ltBool:
		return fasthash64v((u64)v.vBool);
	case ltKeyword:
	case ltSymbol:
		return v.vSymbol->hash;
	case ltInt:
		return fasthash64v(v.vInt);
	case ltFloat:
		return fasthash64v(v.vFloat);
	default: // Anything else can't be used as keys in maps
		return 0xdeadbeef;
	}
}

static bool lMapSetSimple(lMap *map, lVal key, lVal val){
	const u32 size = map->size;
	const u32 mask = size-1;
	u32 off = lHashVal(key) & mask;
	if(unlikely(map->entries == NULL)){
		return false;
	}
	for(u32 i=0; i < size; i++){
		if(map->entries[off].key.type == ltNil){
			map->entries[off].key = key;
			map->entries[off].val = val;
			return true;
		}
		if(lValEqual(key, map->entries[off].key)){
			map->entries[off].key = key;
			map->entries[off].val = val;
			return false;
		}
		off = (off + 1) & mask;
	}
	// This can only occur if the hashmap is full, which must be checked outside ot lMapSetSimple
	exit(176);
}

static lVal lMapRefSimple(lMap *map, lVal key){
	const u32 size = map->size;
	const u32 mask = size-1;
	u32 off = lHashVal(key) & mask;
	for(u32 i=0; i < size; i++){
		if(map->entries[off].key.type == ltNil){ return NIL; }
		if(lValEqual(key, map->entries[off].key)){
			return map->entries[off].val;
		}
		off = (off + 1) & mask;
	}
	return NIL;
}

static bool lMapHasSimple(lMap *map, lVal key){
	const u32 size = map->size;
	const u32 mask = size-1;
	u32 off = lHashVal(key) & mask;
	for(u32 i=0; i < size; i++){
		if(map->entries[off].key.type == ltNil){ return false; }
		if(lValEqual(key, map->entries[off].key)){
			return true;
		}
		off = (off + 1) & mask;
	}
	return false;
}

static void lMapResize(lMap *map, u32 size){
	if(unlikely(size <= map->length)){ return; }
	const lMapEntry *oldEntries = map->entries;
	const u32 oldSize = map->size;
	map->entries = calloc(size, sizeof(lMapEntry));
	map->size = size;
	if(oldEntries != NULL){
		for(uint i=0; i < oldSize; i++){
			if(oldEntries[i].key.type == ltNil){ continue;}
			lMapSetSimple(map, oldEntries[i].key, oldEntries[i].val);
		}
		free((void *)oldEntries);
	}
}

lVal lnfMapNew(lVal v) {
	lMap *m = lMapAllocRaw();
	m->entries = NULL;
	lVal e = v;
	while(e.type == ltPair){
		lVal key = lCar(e);
		if(unlikely((key.type == ltNil) || (key.type == ltFloat))){
			return lValException(lSymTypeError, "Can't use Nil or Float values as keys", key);
		}
		lVal val = lCadr(e);
		e = lCddr(e);
		lMapSet(m, key, val);
	}
	return lValAlloc(ltMap, m);;
}

lVal lMapSet(lMap *map, lVal key, lVal val) {
	if(unlikely((key.type == ltNil) || (key.type == ltFloat))){
		return lValException(lSymTypeError, "Can't use Nil or Float values as keys", key);
	}
	const u32 resizeSize = (map->size - (uint)(map->size >> 2));
	if(unlikely(map->length >= resizeSize)){
		lMapResize(map, MAX(4, map->size) * 2);
	}
	if(lMapSetSimple(map, key, val)){
		map->length++;
	}
	return val;
}

lVal lMapRef(lMap *map, lVal key) {
	if(unlikely((key.type == ltNil) || (key.type == ltFloat))){
		return lValException(lSymTypeError, "Can't use Nil or Float values as keys", key);
	}
	return lMapRefSimple(map, key);
}

lVal lMapRefString(lMap *map, const char *str) {
	const u32 size = map->size;
	const u32 mask = size-1;
	i32 len = strlen(str);
	u32 off = lHashString(str, len) & mask;
	for(u32 i=0; i < size; i++){
		if(map->entries[off].key.type == ltNil){ return NIL; }
		if(map->entries[off].key.type != ltString){ continue; }
		const i32 eLen = map->entries[off].key.vString->length;
		if ((len == eLen) && (memcmp(str, map->entries[off].key.vString->data, len) == 0)){
			return map->entries[off].val;
		}
		off = (off + 1) & mask;
	}
	return NIL;
}

static lVal lnmMapLength(lVal self) {
	return lValInt(self.vMap->length);
}

static lVal lnmMapSize(lVal self) {
	return lValInt(self.vMap->size);
}

static lVal lnmMapHas(lVal self, lVal key) {
	return lValBool(lMapHasSimple(self.vMap, key));
}

static lVal lnmMapKey(lVal self, lVal off) {
	reqNaturalInt(off);
	if(unlikely(off.vInt >= self.vMap->size)){
		return NIL;
	}
	return self.vMap->entries[off.vInt].key;
}

static lVal lnmMapVal(lVal self, lVal off) {
	reqNaturalInt(off);
	if(unlikely(off.vInt >= self.vMap->size)){
		return NIL;
	}
	return self.vMap->entries[off.vInt].val;
}

static lVal lnmMapValues(lVal self) {
	lVal ret = NIL;
	const u32 size = self.vMap->size;
	for(uint i=0; i < size; i++){
		if(self.vMap->entries[i].key.type == ltNil){ continue; }
		ret = lCons(self.vMap->entries[i].val, ret);
	}
	return ret;
}

static lVal lnmMapKeys(lVal self) {
	lVal ret = NIL;
	const u32 size = self.vMap->size;
	for(uint i=0; i < size; i++){
		if(self.vMap->entries[i].key.type == ltNil){ continue; }
		ret = lCons(self.vMap->entries[i].key, ret);
	}
	return ret;
}

static lVal lnmMapClone(lVal self) {
	lMap *m = lMapAllocRaw();
	m->length = self.vMap->length;
	m->flags = self.vMap->flags;
	m->size = self.vMap->size;
	m->entries = calloc(m->size, sizeof(lMapEntry));
	memcpy(m->entries, self.vMap->entries, sizeof(lMapEntry) * m->size);
	return lValAlloc(ltMap, m);
}

void lOperationsMap() {
	lClass *Map = &lClassList[ltMap];
	lAddNativeMethodV  (Map, lSymS("length"), "(self)", lnmMapLength, NFUNC_PURE);
	lAddNativeMethodV  (Map, lSymS("clone"), "(self)", lnmMapClone, NFUNC_PURE);
	lAddNativeMethodVV (Map, lSymS("has?"), "(self key)", lnmMapHas, NFUNC_PURE);
	lAddNativeMethodV  (Map, lSymS("values"), "(self)", lnmMapValues, NFUNC_PURE);
	lAddNativeMethodV  (Map, lSymS("keys"), "(self)", lnmMapKeys, NFUNC_PURE);

	lAddNativeMethodV  (Map, lSymS("size*"), "(self)", lnmMapSize, NFUNC_PURE);
	lAddNativeMethodVV (Map, lSymS("key*"), "(self off)", lnmMapKey, NFUNC_PURE);
	lAddNativeMethodVV (Map, lSymS("value*"), "(self off)", lnmMapVal, NFUNC_PURE);
}
