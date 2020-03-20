#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
/* #define MEMORY_TRAK */
#define MEMORY_EXPORT
#include "Memory.h"
/**
 * @brief A set of functions that provide wrappers around the basic
 * memory allocation routines in order to capture out-of-memory
 * errors and to provide for optional recovery or graceful
 * shutdown.  Debugging fuctions also provide for memory allocation
 * tracking and a memory loss report.
*/
/**
 * An error message to capture the trace identifier passed to
 * the allocation or free function.  The message is static so
 * that no additional allocation is necessary to create an
 * error message when an allocation error has already occurred.
 * The x's are overlayed with the first 16 characters of the
 * trace identifier.
*/
static char nomemmsg[] = "out of memory:xxxxxxxxxxxxxxxx";
/**
 * An error message for freeing a 0-pointer.
*/
static char zeromsg[]  = "freeing 0 ptr:xxxxxxxxxxxxxxxx";
/*
 * Memory error function prototype.
*/
static void memoryError(char *msg, void *prm);
/**
 * The MemoryErrorBlock tracks the current memory error function.
 * MemoryErrorBlock's are chained in a stack that is managed by
 * the pushMemoryErrorFunction and popMemoryErrorFunction routines.
*/
typedef struct MemoryErrorBlock
{
  struct MemoryErrorBlock *next;
  MemoryErrorFunc errorFunc;
  void *parm;
} MemoryErrorBlock;
#define _tagSize 16
static const int tagSize = _tagSize;
/**
 * The base of the MemoryErrorBlock stack, which is always present.
*/
static MemoryErrorBlock baseMemoryErrorBlock = {0, memoryError, 0};
/**
 * A chain of free MemoryErrorBlock's to reduce the impact of stack
 * addition and deletion on allocation.
*/
static MemoryErrorBlock *mebFree  = 0;
/**
 * A reference to the top MemoryErrorBlock on the stack.
*/
static MemoryErrorBlock *mebStack = &baseMemoryErrorBlock;
/**
 * Push a memory error function onto the stack, making it the
 * current memory error function.  The current memory error function
 * will be called if an allocation request returns a 0-pointer.
 * @param fnc the new memory error function
 * @param userdata a reference to opaque user data
*/
void
pushMemoryErrorFunction(MemoryErrorFunc fnc, void *userdata)
{
 MemoryErrorBlock *blk;
 if (mebFree == 0)
 {
   blk = (MemoryErrorBlock *)malloc(sizeof(MemoryErrorBlock));
   if (blk == 0)
   {
     mebStack->errorFunc("pushMemoryErrorFunction", (void *)0);
     return;
   }
 }
 else
 {
   blk = mebFree;
   mebFree = blk->next;
 }
 blk->errorFunc = fnc;
 blk->next = mebStack;
 blk->parm = userdata;
 mebStack = blk;
}
/**
 * Pop the top of the memory error function stack, discarding the
 * top function and making the next function the current memory
 * error function.  The base of the stack is a static MemoryErrorBlock
 * that points to the generic memory error function, and is never popped.
*/
void
popMemoryErrorFunction(void)
{
 MemoryErrorBlock *blk;
 blk = mebStack;
 if (blk != &baseMemoryErrorBlock)
 {
   mebStack = blk->next;
   blk->next = mebFree;
   mebFree = blk;
 }
 else
   mebStack->errorFunc("popMemoryErrorFunction", (void *)0);
}
/**
 * The generic memory error function that passes the error
 * message constructed by the allocation routine to the system
 * error function perror() and exits.
 * @param msg the error message which contains the caller's trace
 * @param prm a reference to opaque user data, unused in the base error function
*/
static void
memoryError(char *msg, void *prm)
{
  fprintf(stderr, "%s", msg);
  fprintf(stderr, "\n");
  exit(64);
}
#ifdef MEMORY_TRAK
/**
 * An error message for freeing unallocated memory.
 */
static char unalmsg[]  = "unalloc free :xxxxxxxxxxxxxxxx";
static struct StorTrak
{
  unsigned size;
  struct StorTrak *link;
  char label[_tagSize];
} *trak = 0;
static unsigned char trakFence[4] = {'Z','Z','Z','Z'};
static void report(void *area, char *label, unsigned size);
/**
 * A wrapper around the basic system memory allocation interface that
 * adds memory tracking and out-of-bounds write analysis.
*/
void *
mAlloc(unsigned sz, char *trace)
{
 char *stor;
 struct StorTrak *sttr;
 int size;
 size = sz + sizeof(struct StorTrak) + 4;
 if ((sttr = (struct StorTrak *)malloc(size)) == 0)
 {
   memcpy(&nomemmsg[14], trace, tagSize);
   mebStack->errorFunc(nomemmsg, mebStack->parm);
   return 0;
 }
 sttr->link = trak;
 trak = sttr;
 sttr->size = sz;
 memset(&sttr->label[(sizeof sttr->label) - 4], 'A', 4);
 strncpy(sttr->label, trace, sizeof sttr->label);
 stor = (char *)(sttr + 1);
 memcpy(stor + sz, trakFence, 4);
 return stor;
}
/**
 * A wrapper around the system calloc routine to allocate an initialized
 * array of elements; the wrapper checks for a
 * 0-pointer return and calls the current memory error function on
 * allocation failure.
 * @param count the number of array elements to allocate
 * @param size the size of each array element
 * @param trace the caller trace message
 * @return a pointer to the allocated array; the memory allocated
 *     is initialized to zero; if allocation fails and the current
 *     memory error function returns, the 0-pointer is returned
 *     to the caller
*/
void *
cAlloc(unsigned count, unsigned size, char *trace)
{
 char *stor; 
 unsigned bytes;
 bytes = count * size;
 if ((stor = (void *)mAlloc(bytes, trace)) != 0)
   memset(stor, 0, bytes);
 return stor; 
}
int
mIsAlloc(void *area)
{
 struct StorTrak *sttr, *sq;
 sttr = (struct StorTrak *)area - 1;
 sq = trak;
 while (sq != 0)
 {
   if (sq == sttr)
     return 1;
   sq = sq->link;
 }
 return 0;
}
void 
mFree(void *area, char *trace)
{
 struct StorTrak *sttr, **sq;
 if (area != 0)
 {
   sttr = (struct StorTrak *)area - 1;
   sq = &trak;
   while (*sq != 0)
   {
     if (*sq == sttr)
     {
       *sq = sttr->link;
       if (memcmp((char *)area + sttr->size, trakFence, 4) != 0)
	 report(area, sttr->label, sttr->size);
       free(sttr);
       return;
     }
     sq = &(*sq)->link;
   }
   /* for the moment, assume allocated outside memory tracking 
   free(area); */
   memcpy(&unalmsg[14], trace, tagSize); 
   mebStack->errorFunc(unalmsg, mebStack->parm);
 }
 else
 {
   memcpy(&zeromsg[14], trace, tagSize); 
   mebStack->errorFunc(zeromsg, mebStack->parm);
 }
}
/**
 * A wrapper around the system realloc routine to expand (or contract)
 * a previously allocated storage area.  The contents of the previously
 * allocated area are copied to the beginning of the new area as
 * size permits, and the previously allocated area is freed.
 * In order to accomodate first-time allocation, a 0-pointer for the
 * previous
 * The wrapper checks for a 0-pointer return and calls the current
 * memory error function on allocation failure.
 * @param area the area to be expanded (or contracted); if 0, the
 *        requests defaults to a simple allocation request
 * @param size the size of requested area
 * @param trace the caller trace message
 * @return a pointer to the allocated array; the memory allocated
 *     is initialized to the contents of the previously allocated area
 *     as size permits, the remaining area is uninitialized; if
 *     allocation fails and the current memory error function returns,
 *     the 0-pointer is returned to the caller
*/
void *
reAlloc(void *area, unsigned size, char *trace)
{
 char *stor;                  
 struct StorTrak *sttr, **sq;
 if (area == 0)
   return mAlloc(size, trace);
 sttr = (struct StorTrak *)area - 1;
 sq = &trak;
 while (*sq != 0)
 {
   if (*sq == sttr)
     break;
   sq = &(*sq)->link;
 }
 if (*sq == 0)
 {
   memcpy(&unalmsg[14], trace, tagSize); 
   mebStack->errorFunc(unalmsg, mebStack->parm);
   return 0;
 }
 *sq = sttr->link;
 if ((stor = (void *)mAlloc(size, trace)) != 0)
   memcpy(stor, area, sttr->size);
 if (memcmp((char *)area + sttr->size, trakFence, 4) != 0)
   report(area, sttr->label, sttr->size);
 free(sttr);  /* memory already removed from chain, no need for mFree */
 return stor; 
}
static void
report(void *area, char *label, unsigned size)
{
  int ct;
  int max;
  char hex[33], chr[17];
  unsigned char *cp;
  fprintf(stderr, "allocated area %08x trace %*.*s size %d\n", area, tagSize, tagSize, label, size);
  max = (size > 16) ? 16 : size;
  cp = (unsigned char *)area;
  for (ct = 0; ct < max; ++ct, ++cp)
  {
    sprintf(&hex[ct + ct], "%02x", *cp);
    chr[ct] = (isalnum(*cp)) ? *(char *)cp : '.';
  }
  for (; ct < 16; ++ct)
  {
    sprintf(&hex[ct + ct], "  ");
    chr[ct] = ' ';
  }
  chr[16] = 0;
  fprintf(stderr, "  %s *%s*\n", hex, chr);
  if (memcmp((char *)area + size, trakFence, 4) != 0)
  {
    cp = (unsigned char *)area + size;
    for (ct = 0; ct < 4; ++ct, ++cp)
    {
      sprintf(&hex[ct + ct], "%02x", *cp);
    }
    fprintf(stderr, "  fence corruption %s\n", hex);
  }
}
void 
trakReport(char *id)
{
 struct StorTrak *sttr, **sq;
 int ct;
 ct = 0;
 sq = &trak;
 fprintf(stderr, "Memory Track Report for %s\n", id);
 while (*sq != 0)
 {
   sttr = *sq;
   ++ct;
   report((void *)(sttr + 1), sttr->label, sttr->size);
   sq = &sttr->link;
 }
 fprintf(stderr, "Memory Track Report complete\n", id);
}
#else
/**
 * A wrapper around the basic system memory allocation interface that
 * adds memory tracking and out-of-bounds write analysis.
*/
void *
mAlloc(unsigned sz, char *trace)
{
 char *stor;
 if ((stor = (char *)malloc(sz)) == 0)
 {
   memcpy(&nomemmsg[14], trace, tagSize);
   mebStack->errorFunc(nomemmsg, mebStack->parm);
   return 0;
 }
 return stor;
}
/**
 * A wrapper around the system calloc routine to allocate an initialized
 * array of elements; the wrapper checks for a
 * 0-pointer return and calls the current memory error function on
 * allocation failure.
 * @param count the number of array elements to allocate
 * @param size the size of each array element
 * @param trace the caller trace message
 * @return a pointer to the allocated array; the memory allocated
 *     is initialized to zero; if allocation fails and the current
 *     memory error function returns, the 0-pointer is returned
 *     to the caller
*/
void *
cAlloc(unsigned count, unsigned size, char *trace)
{
 char *stor; 
 if ((stor = (void *)calloc(count, size)) == 0)
 {
   memcpy(&nomemmsg[14], trace, tagSize);
   mebStack->errorFunc(nomemmsg, mebStack->parm);
   return 0;
 }
 return stor; 
}
void *
reAlloc(void *area, unsigned size, char *trace)
{
 char *stor;
 if ((stor = (char *)realloc(area, size)) == 0)
 {
   memcpy(&nomemmsg[14], trace, tagSize); 
   mebStack->errorFunc(nomemmsg, mebStack->parm);
   return 0;
 }
 return stor;
}
void 
mFree(void *area, char *trace)
{
 if (area != 0)
 {
   free(area);
 }
 else
 {
   memcpy(&zeromsg[14], trace, tagSize); 
   mebStack->errorFunc(zeromsg, mebStack->parm);
 }
}
#endif
