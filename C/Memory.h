#ifndef MEMORY_INCLUDE_GUARD
#define MEMORY_INCLUDE_GUARD

#ifdef MEMORY_EXPORT
#define MEMORY_API
#else
#define MEMORY_API extern
#endif
#ifndef NO_MEMORY
typedef void (*MemoryErrorFunc)(char *, void *);
MEMORY_API void pushMemoryErrorFunction(MemoryErrorFunc fnc, void *prm);
MEMORY_API void popMemoryErrorFunction(void);
MEMORY_API void *cAlloc(unsigned count, unsigned size, char *trace);
MEMORY_API void *reAlloc(void *area, unsigned size, char *trace);
MEMORY_API void *mAlloc(unsigned size, char *trace);
MEMORY_API void mFree(void *area, char *trace);
MEMORY_API void *cAllocT(unsigned count, unsigned size, char *trace);
MEMORY_API void *reAllocT(void *area, unsigned size, char *trace);
MEMORY_API void *mAllocT(unsigned size, char *trace);
MEMORY_API void mFreeT(void *area, char *trace);
#else
#include <stdlib.h>
#define mAlloc(s,t) malloc(s)
#define cAlloc(c,s,t) calloc(c,s)
#define reAlloc(m,s,t) realloc(m,s)
#define mFree(m,t) free(m)
#endif
#ifdef MEMORY_TRACKX
#define mAlloc mAllocT
#define cAlloc cAllocT
#define reAlloc reAllocT
#define mFree mFreeT
#endif
MEMORY_API void trakReport(char *id);
#undef MEMORY_API
#endif
