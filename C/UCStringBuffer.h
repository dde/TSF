#ifndef UCSTRINGBUFFER_INCLUDE
#define UCSTRINGBUFFER_INCLUDE
#ifndef API_EXPORT
#define APIDECL extern
#else
#define APIDECL
#endif
#define UCS_CHAR
#ifdef UCS_CHAR
typedef wchar_t Char;
#else
typedef char Char;
#endif
/**
 * Character Buffer API declarations.
*/
/**
 * The UCStringBuffer object provides the anchor for a chain of fixed
 * length buffer units that implement a dynamic string, and stores
 * properties needed to manage the buffer unit chain.
*/
struct UCSbu;
typedef struct UCStringBuffer
{
 /**
  * Locates the first buffer unit on the chain.
 */
 struct UCSbu *frst;
 /**
  * Locates the last buffer unit on the chain.
 */
 struct UCSbu *last;
 /**
  * Locates the current buffer unit.
 */
 struct UCSbu *curr;
 /**
  * The current character pointer, always points within the current
  * buffer unit.
 */
 Char       *cptr;
 /**
  * The logical offset of the current character pointer.  If the
  * character data was contiguous, then cptr would equal &amp;data[coff].
 */
 unsigned    coff;
} UCStringBuffer;
typedef int (*UCStringBufferCallback)(const Char *, unsigned, void *, unsigned);
APIDECL UCStringBuffer *newUCStringBuffer(UCStringBuffer *, unsigned len);
APIDECL UCStringBuffer *ucstringBufferCopy(UCStringBuffer *);
APIDECL UCStringBuffer *ucstringBufferAssign(UCStringBuffer *cb, UCStringBuffer *);
APIDECL void delUCStringBuffer(UCStringBuffer *);
APIDECL void vdeUCStringBuffer(UCStringBuffer *);
APIDECL Char *ucstringBufferInc(UCStringBuffer *cb, int amt);
APIDECL Char *ucstringBufferDec(UCStringBuffer *cb, int amt);
APIDECL Char *ucstringBufferPreInc(UCStringBuffer *cb);
APIDECL Char *ucstringBufferPostInc(UCStringBuffer *cb);
APIDECL Char *ucstringBufferPostDec(UCStringBuffer *cb);
APIDECL Char *ucstringBufferPreDec(UCStringBuffer *cb);
APIDECL unsigned ucstringBufferGetOffset(UCStringBuffer *cb);
APIDECL Char *ucstringBufferSetOffset(UCStringBuffer *cb, unsigned offset);
APIDECL Char *ucstringBufferGetPointer(UCStringBuffer *cb);
APIDECL unsigned ucstringBufferGetRemainder(UCStringBuffer *cb);
APIDECL Char *ucstringBufferToString(UCStringBuffer *cb, unsigned *lp);
APIDECL void ucstringBufferAppend(UCStringBuffer *cb, const Char *str, unsigned len);
APIDECL int  ucstringBufferForEach(UCStringBuffer *cb, UCStringBufferCallback callback, void *prm);
#undef APIDECL
#endif
