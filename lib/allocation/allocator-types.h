#define NFN_MAX (1<<10)

#define ARR_MAX (1<<14)
defineAllocator(lArray, ARR_MAX)

#define CLO_MAX (1<<16)
defineAllocator(lClosure, CLO_MAX)

#define TRE_MAX (1<<19)
defineAllocator(lTree, TRE_MAX)

#define VAL_MAX (1<<21)
defineAllocator(lVal, VAL_MAX)

#define BCA_MAX (1<<14)
defineAllocator(lBytecodeArray, BCA_MAX)

#define BUF_MAX (1<<15)
defineAllocator(lBuffer, BUF_MAX)

#define BFV_MAX (1<<14)
defineAllocator(lBufferView, BFV_MAX)
