/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#ifdef _MSC_VER
#include <malloc.h>
#endif

/* This boolean flag is used to determine if the garbage collector should run soon.
 * It is set when active heap bytes cross the next GC threshold. This keeps
 * collection at VM safe points, where we already know how to mark the runtime
 * stacks without scanning the native C stack.
 */
bool lGCShouldRunSoon = false;

/* This macro defines a chunked allocator for a given type T.
 * For each type T it defines:
 * - TActive counting currently allocated objects
 * - TFFree pointing to the first free object in the free list
 * - TAllocRaw() function to allocate new instances
 *
 * The allocator works in two ways:
 * 1. If there are freed objects (TFFree != NULL), reuse one from the free list
 * 2. Otherwise allocate a new chunk, add its slots to the free list, and retry
 *
 * It also integrates with the GC by:
 * - Keeping one tail mark byte per object slot in each aligned chunk
 * - Clearing mark bits for new allocations
 * - Zeroing new objects with memset
 */
lHeapBlock *lHeapBlocks = NULL;
uint lHeapBlockMax = 0;
uint lHeapBlockCap = 0;
size_t lHeapActiveBytes = 0;
size_t lHeapNextGCBytes = 4 * 1024 * 1024;

static bool lHeapIsPowerOfTwo(uint v){
	return v && ((v & (v - 1)) == 0);
}

void *lHeapAllocBlock(uint type, size_t eleSize, uint chunkBytes, uint *count){
	if(unlikely(!lHeapIsPowerOfTwo(chunkBytes))){
		fprintf(stderr, "VM error: heap chunk size must be a power of two\n");
		exit(125);
	}
	if(unlikely(lHeapBlockMax >= lHeapBlockCap)){
		const uint newCap = lHeapBlockCap ? lHeapBlockCap * 2 : 16;
		lHeapBlock *newBlocks = realloc(lHeapBlocks, newCap * sizeof(lHeapBlock));
		if(unlikely(newBlocks == NULL)){
			fprintf(stderr, "OOM: couldn't allocate heap block table\n");
			exit(123);
		}
		lHeapBlocks = newBlocks;
		lHeapBlockCap = newCap;
	}

	uint slotCount = chunkBytes / (eleSize + 1);
	if(slotCount < 1){slotCount = 1;}
	const uint objectBytes = slotCount * eleSize;
	void *ptr = NULL;
#ifdef _MSC_VER
	ptr = _aligned_malloc(chunkBytes, chunkBytes);
	if(ptr != NULL){
		memset(ptr, 0, chunkBytes);
	}
#else
	if(posix_memalign(&ptr, chunkBytes, chunkBytes) == 0){
		memset(ptr, 0, chunkBytes);
	} else {
		ptr = NULL;
	}
#endif
	if(unlikely(ptr == NULL)){
		fprintf(stderr, "OOM: couldn't allocate heap block\n");
		exit(123);
	}

	lHeapBlocks[lHeapBlockMax++] = (lHeapBlock){ptr, type, objectBytes};
	*count = slotCount;
	return ptr;
}

i64 lHeapObjectID(const void *ptr, uint type, size_t eleSize){
	i64 ret = 0;
	const uintptr_t p = (uintptr_t)ptr;
	for(uint i=0;i<lHeapBlockMax;i++){
		lHeapBlock *block = &lHeapBlocks[i];
		if(block->type != type){continue;}
		const uintptr_t start = (uintptr_t)block->ptr;
		const uintptr_t end = start + block->size;
		if((p >= start) && (p < end)){
			return ret + ((p - start) / eleSize);
		}
		ret += block->size / eleSize;
	}
	return 0;
}

#define defineAllocator(T, typeTag, chunkBytes) \
uint T##Active = 0; \
T * T##FFree = NULL; \
static void T##AllocBlock(){\
	uint count;\
	T *block = lHeapAllocBlock(typeTag, sizeof(T), chunkBytes, &count);\
	u8 *marks = ((u8 *)block) + (count * sizeof(T));\
	for(uint i=0;i<count;i++){\
		block[i].nextFree = T##FFree;\
		T##FFree = &block[i];\
		marks[i] = 2;\
	}\
}\
T * T##AllocRaw (){\
	if((T##FFree) == NULL){			\
		T##AllocBlock();\
	}\
	T *ret = T ## FFree;\
	(T##FFree) = ret->nextFree;\
	T##Active++;\
	lHeapActiveBytes += sizeof(T);\
	if(unlikely(lHeapActiveBytes > lHeapNextGCBytes)){lGCShouldRunSoon = true;} \
	*lHeapMarkByte(ret, sizeof(T), chunkBytes) = 0;\
	memset(ret,0,sizeof(T));\
	return ret;\
}
allocatorTypes()
#undef defineAllocator

/* We could also use the defineAllocator macro for this, but right now it doesn't make sense to free Native Function bindings
 * since they can't be created from within Nujel, only via the C API.
 */
lNFunc   lNFuncList[NFN_MAX];
uint     lNFuncMax    = 0;


int lBufferViewTypeSize(lBufferViewType T){
	switch(T){
	default:
		exit(4);
	case lbvtU8:
	case lbvtS8:
		return 1;
	case lbvtS16:
	case lbvtU16:
		return 2;
	case lbvtF32:
	case lbvtS32:
	case lbvtU32:
		return 4;
	case lbvtF64:
	case lbvtS64:
		return 8;
	}
}


lBuffer *lBufferAlloc(size_t length, bool immutable){
	lBuffer *ret = lBufferAllocRaw();
	ret->length = length;
	if(immutable){
		ret->flags = BUFFER_IMMUTABLE;
	}else{
		ret->buf = calloc(length, 1);
	}
	return ret;
}

lBufferView *lBufferViewAlloc(lBuffer *buf, lBufferViewType type, size_t offset, size_t length, bool immutable){
	lBufferView *ret = lBufferViewAllocRaw();
	ret->buf    = buf;
	ret->offset = offset;
	ret->length = length;
	ret->flags  = immutable;
	ret->type   = type;
	return ret;
}

lBytecodeArray *lBytecodeArrayAlloc(size_t len){
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	ret->data = calloc(len, sizeof(lBytecodeOp));
	if(unlikely(ret->data == NULL)){
		fprintf(stderr, "OOM: Couldn't allocate a new BC array\n");
		exit(134);
	}
	ret->dataEnd = &ret->data[len];
	return ret;
}

lArray *lArrayAlloc(size_t len){
	lArray *ret = lArrayAllocRaw();
	ret->data = calloc(len + 1, sizeof(lVal));
	if(unlikely(ret->data == NULL)){
		fprintf(stderr, "OOM: Couldn't allocate a new array");
		exit(135);
	}
	ret->length = len;
	return ret;
}

lNFunc *lNFuncAlloc(){
	if(unlikely(lNFuncMax >= NFN_MAX-1)){
		exit(124);
	}
	memset(&lNFuncList[lNFuncMax++], 0, sizeof(ltNativeFunc));
	return &lNFuncList[lNFuncMax++];
}
