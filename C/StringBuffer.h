#ifndef CHARBUFF_INCLUDE
#define CHARBUFF_INCLUDE
#ifndef API_EXPORT
#define APIDECL extern
#else
#define APIDECL
#endif
/**
 * Character Buffer API declarations.
*/
/**
 * The StringBuffer object provides the anchor for a chain of fixed
 * length buffer units that implement a dynamic string, and stores
 * properties needed to manage the buffer unit chain.
*/
struct Sbu;
typedef struct StringBuffer
{
 /**
  * Locates the first buffer unit on the chain.
 */
 struct Sbu *frst;
 /**
  * Locates the last buffer unit on the chain.
 */
 struct Sbu *last;
 /**
  * Locates the current buffer unit.
 */
 struct Sbu *curr;
 /**
  * The current character pointer, always points within the current
  * buffer unit.
 */
 char       *cptr;
 /**
  * The logical offset of the current character pointer.  If the
  * character data was contiguous, then cptr == &amp;data[coff].
 */
 int         coff;
} StringBuffer;
typedef int (*StringBufferCallback)(const char *, int, void *, int);
APIDECL StringBuffer *newStringBuffer(StringBuffer *, unsigned len);
APIDECL StringBuffer *stringBufferCopy(StringBuffer *);
APIDECL StringBuffer *stringBufferAssign(StringBuffer *cb, StringBuffer *);
APIDECL void delStringBuffer(StringBuffer *);
APIDECL void vdeStringBuffer(StringBuffer *);
APIDECL char *stringBufferInc(StringBuffer *cb, int amt);
APIDECL char *stringBufferDec(StringBuffer *cb, int amt);
APIDECL char *stringBufferPreInc(StringBuffer *cb);
APIDECL char *stringBufferPostInc(StringBuffer *cb);
APIDECL char *stringBufferPostDec(StringBuffer *cb);
APIDECL char *stringBufferPreDec(StringBuffer *cb);
APIDECL int  stringBufferGetOffset(StringBuffer *cb);
APIDECL char *stringBufferSetOffset(StringBuffer *cb, unsigned offset);
APIDECL char *stringBufferGetPointer(StringBuffer *cb);
APIDECL int  stringBufferGetRemainder(StringBuffer *cb);
APIDECL char *stringBufferToString(StringBuffer *cb, int *lp);
APIDECL void stringBufferAppend(StringBuffer *cb, const char *str, unsigned len);
APIDECL int  stringBufferForEach(StringBuffer *cb, StringBufferCallback callback, void *prm);
#undef APIDECL
#endif
