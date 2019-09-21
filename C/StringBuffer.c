/**
 * A set of functions to support dynamically variable character strings.
 * Variable string data is managing in a set of linked, fixed length
 * buffer units.  There are functions that correspond to the normal pointer
 * operations of pre- and post- increment and decrement, as weel as
 * pointer arithmetic.  Space is automatically allocated as the current
 * pointer with the buffer increases in offset.
*/
#ifdef WIN32
#endif
#include <stdlib.h>
#include <string.h>
#include "Memory.h"
/**
 * Import the API declarations.
*/
#define API_EXPORT
#include "StringBuffer.h"
/**
 * Character buffer unit structure
*/
struct Sbu
{
  /**
   * A link to the previous buffer unit, 0 in the first unit.
  */
  struct Sbu *next;
  /**
   * A link to the next buffer unit, 0 in the last unit.
  */
  struct Sbu *back;
  /**
   * The size of the data area.
  */
  unsigned short len;
  /**
   * A reference counter for the Sbu chain, unused in all but the
   * first buffer on a chain.
  */
  short      ref;
  /**
   * The data area, whose size is given by the member len.  All the units
   * in a single buffer chain have the same size in this implementation.
  */
  char       data[1];
};
/**
 * The default character buffer size.
*/
static const int bufferSize = 128;
/**
 * Allocate a new character buffer unit and link it at the end of
 * the character buffer unit chain.
 * @param cb the associated StringBuffer object
 * @param len the length of the character buffer unit
 * @return the new character buffer unit
*/
static struct Sbu *
newCU(StringBuffer *cb, int len)
{
 struct Sbu *bu;
 bu = (struct Sbu *)mAlloc(sizeof(struct Sbu) + len - 1, "StringBUnit");
 bu->ref = 0;
 bu->back = cb->curr;
 bu->next = 0;
 if (cb->curr != 0)
   cb->curr->next = bu;
 else
   cb->frst = bu;
 cb->curr = cb->last = bu;
 bu->len = len;
 return bu;
}
/**
 * Construct a StringBuffer. The maximum allowed unit size is 32767.
 * @param cb a pointer to a derived (or stack) structure, or 0 if none
 * @param len the length of each buffer unit, 0 implies default
 * @return a reference to a new StringBuffer
*/
StringBuffer *
newStringBuffer(StringBuffer *cb, unsigned len)
{
  if (len <= 0 || len > 32767)
    len = bufferSize;
  if (cb == 0)
    cb = (StringBuffer *)mAlloc(sizeof(StringBuffer), "StringBuffer");
  cb->frst = 0;
  cb->last = 0;
  cb->curr = 0;
  cb->coff = 0;
  newCU(cb, len);
  cb->cptr = cb->curr->data;
  ++cb->frst->ref;
  return cb;
}
/**
 * Destruct the StringBuffer whose reference is passed.  The reference
 * count in the buffer unit chain is decremented and the chain is
 * released if there are no more references.  The StringBuffer struct
 * is freed in all cases.
 * @param cb the StringBuffer to be destructed
*/
void
delStringBuffer(StringBuffer *cb)
{
 struct Sbu *cbup, *cbs;
 cbup = cb->frst;
 if (--cbup->ref <= 0)
  {
   while (cbup != 0)
    {
     cbs = cbup->next;
     mFree((char *)cbup, "StringBUnit");
     cbup = cbs;
    }
  }
 mFree(cb, "StringBuffer");
}
/**
 * Virtually destruct the StringBuffer whose reference is passed.  The reference
 * count in the buffer unit chain is decremented and the chain is
 * released if there are no more references.  The StringBuffer struct
 * is not freed, the assumption being that it is on the stack, or within
 * a derived structure.
 * @param cb the StringBuffer to be virtually destructed
*/
void
vdeStringBuffer(StringBuffer *cb)
{
 struct Sbu *cbup, *cbs;
 cbup = cb->frst;
 if (--cbup->ref <= 0)
  {
   while (cbup != 0)
    {
     cbs = cbup->next;
     mFree((char *)cbup, "StringBUnitV");
     cbup = cbs;
    }
  }
}
/**
 * Copy (shallow) the passed StringBuffer, incrementing the
 * reference counter on the chain of buffer units.
 * @param copy the StringBuffer to be copied
 * @return a reference to the new copy
*/
StringBuffer *
stringBufferCopy(StringBuffer *copy)
{
  StringBuffer *cb;
  cb = (StringBuffer *)mAlloc(sizeof(StringBuffer), "stringBufcopy");
  cb->frst = copy->frst;
  cb->last = copy->last;
  cb->curr = copy->curr;
  cb->coff = copy->coff;
  cb->cptr = copy->cptr;
  ++cb->frst->ref;
  return cb;
}
/**
 * Assign the second StringBuffer to the first, resulting in two
 * StringBuffer's pointing to the same chain of buffer units.  A
 * reference count is maintained in the buffer chain so that
 * it will not be deleted until all references to it are gone.
 * @param cb the target of the assignment
 * @param src the source of the assignment
 * @return the newly assigned target
*/
StringBuffer *
stringBufferAssign(StringBuffer *cb, StringBuffer *src)
{
 struct Sbu *cbup, *cbs;
 cbup = cb->frst;
 if (--cbup->ref <= 0)
  {
   while (cbup != 0)
    {
     cbs = cbup->next;
     mFree((char *)cbup, "StringBUnitA");
     cbup = cbs;
    }
  }
 cb->frst = src->frst;
 cb->last = src->last;
 cb->curr = src->curr;
 cb->coff = src->coff;
 cb->cptr = src->cptr;
 ++cb->frst->ref;
 return cb;
}
/**
 * Increment the passed character buffer pointer by an integer.  If
 * the new pointer exceeds the buffer unit chain, additional
 * uninitialized buffer units are added as needed.
 * @param cb a reference to the StringBuffer
 * @param amt the increment
 * @return a pointer to the updated character position
*/
char *
stringBufferInc(StringBuffer *cb, int amt)
{
 if (amt < 0)
   return stringBufferDec(cb, -amt);
 cb->coff += amt;
 while (cb->cptr - cb->curr->data + amt >= cb->curr->len)
  {
   amt -= &cb->curr->data[cb->curr->len] - cb->cptr;
   if (cb->curr->next == (struct Sbu *)0)
     newCU(cb, cb->frst->len);
   else
     cb->curr = cb->curr->next;
   cb->cptr = cb->curr->data;
  }
 cb->cptr += amt;
 return cb->cptr;
}
/**
 * Decrement the passed character buffer pointer by an integer.  The
 * pointer cannot be set beyond the first character.
 * @param cb a reference to the StringBuffer
 * @param amt the increment
 * @return a pointer to the updated character position
*/
char *
stringBufferDec(StringBuffer *cb, int amt)
{
 if (amt < 0)
   return stringBufferInc(cb, -amt);
 cb->coff -= amt;
 while (cb->cptr - amt < cb->curr->data)
  {
   amt -= cb->cptr - cb->curr->data;
   if (cb->curr->back == (struct Sbu *)0)
    {
     cb->cptr = cb->frst->data;
     cb->coff = 0;
     return 0;
    }
   else
     cb->curr = cb->curr->back;
   cb->cptr = &cb->curr->data[cb->curr->len];
  }
 cb->cptr -= amt;
 return cb->cptr;
}
/**
 * Pre-increment the passed character buffer pointer
 * @param cb a reference to the StringBuffer
 * @return a pointer to the updated character position
*/
char *
stringBufferPreInc(StringBuffer *cb)
{
 ++cb->coff;
 if (++cb->cptr - cb->curr->data >= cb->curr->len)
  {
   if (cb->curr->next == (struct Sbu *)0)
     newCU(cb, cb->frst->len);
   else
     cb->curr = cb->curr->next;
   cb->cptr = cb->curr->data;
  }
 return cb->cptr;
}
/**
 * Post-increment the passed character buffer pointer.
 * @param cb a reference to the StringBuffer
 * @return a pointer to the character position before the increment
*/
char *
stringBufferPostInc(StringBuffer *cb)
{
 char *cp;
 ++cb->coff;
 cp = cb->cptr++;
 if (cb->cptr - cb->curr->data >= cb->curr->len)
  {
   if (cb->curr->next == (struct Sbu *)0)
     newCU(cb, cb->frst->len);
   else
     cb->curr = cb->curr->next;
   cb->cptr = cb->curr->data;
  }
 return cp;
}
/**
 * Post-decrement the passed character buffer pointer.
 * @param cb a reference to the StringBuffer
 * @return a pointer to the character position before the decrement
*/
char *
stringBufferPostDec(StringBuffer *cb)
{
 char *cp;
 --cb->coff;
 cp = cb->cptr--;
 if (cb->cptr < cb->curr->data)
  {
   if (cb->curr->back == (struct Sbu *)0)
     cb->cptr = 0;
   else
    {
     cb->curr = cb->curr->back;
     cb->cptr = cb->curr->data + cb->curr->len - 1;
    }
  }
 return cp;
}
/**
 * Pre-decrement the passed character buffer pointer.
 * @param cb a reference to the StringBuffer
 * @return a pointer to the updated character position
*/
char *
stringBufferPreDec(StringBuffer *cb)
{
 --cb->coff;
 --cb->cptr;
 if (cb->cptr < cb->curr->data)
  {
   if (cb->curr->back == (struct Sbu *)0)
     cb->cptr = 0;
   else
    {
     cb->curr = cb->curr->back;
     cb->cptr = cb->curr->data + cb->curr->len - 1;
    }
  }
 return cb->cptr;
}
/**
 * Return the pointer offset of the character buffer.
 * @param cb a reference to the StringBuffer
 * @return the integer offset of the pointer, always greater than or equal to 0
*/
int
stringBufferGetOffset(StringBuffer *cb)
{
 return cb->coff;
}
/**
 * Set the pointer offset in the character buffer. If
 * the new offset exceeds the buffer unit chain, additional
 * uninitialized buffer units are added as needed.
 * @param cb a reference to the StringBuffer
 * @param offset the new pointer offset
 * @return the updated character pointer
*/
char *
stringBufferSetOffset(StringBuffer *cb, unsigned offset)
{
 struct Sbu *ccb;
 unsigned len;
 len = 0;
 ccb = cb->frst;
 while (ccb && (ccb->len > 0) && (len + ccb->len <= offset))
  {
   len += ccb->len;
   if ((ccb = ccb->next) == 0)
     ccb = newCU(cb, cb->frst->len);
  }
 cb->curr = ccb;
 cb->cptr = ccb->data + (offset - len);
 cb->coff = offset;
 return cb->cptr;
}
/**
 * Return the pointer to the current location in the character buffer.
 * @param cb a reference to the StringBuffer
 * @return the character pointer
*/
char *
stringBufferGetPointer(StringBuffer *cb)
{
 return cb->cptr;
}
/**
 * Return the remaining size in the current character buffer.
 * This supports direct I/O into the buffer unit.  The storage area
 * from stringBufferGetPointer() for the length returned by this
 * function is guaranteed to be contiguous.
 * @param cb a reference to the StringBuffer
 * @return the character pointer
*/
int
stringBufferGetRemainder(StringBuffer *cb)
{
 return (int)(&cb->curr->data[cb->curr->len] - cb->cptr);
}
/**
 * Return a copy of the StringBuffer buffer contents in newly allocated
 * contiguous storage, from the beginning of the StringBuffer
 * to its current offset.  The returned value is not 0-terminated unless
 * a 0 has been stored at the end of the StringBuffer.
 * @param cb a reference to the StringBuffer
 * @param lp if non-zero, a pointer to an integer where the size of the
 *   return buffer is stored
 * @return a pointer to the newly allocated copy
*/
char *
stringBufferToString(StringBuffer *cb, int *lp)
{
 int len;
 struct Sbu *sv;
 char *str, *cp;
 len = cb->coff;
 cp = str = (char *)mAlloc(len,"stringBuf2string");
 sv = cb->frst;
 while (cb->curr != sv)
  {
   memcpy(cp, sv->data, sv->len);
   cp += sv->len;
   sv = sv->next;
  }
 memcpy(cp, sv->data, cb->cptr - sv->data);
 if (lp != 0)
   *lp = len;
 return str;
}
/**
 * Append a contiguous string at the current position in a character buffer.
 * @param cb a reference to the StringBuffer
 * @param str a reference to the string to be appended
 * @param len if non-zero, the number of character to be appended; if 0,
 *     the string will be assumed to be 0-terminated, and the length will
 *     be determined by a call to strlen(); note this implies that the
 *     terminating 0-byte will NOT be appended
*/
void
stringBufferAppend(StringBuffer *cb, const char *str, unsigned len)
{
 unsigned bfl;
 struct Sbu *bu;
 if (len <= 0)
   len = (unsigned)strlen(str);
 bu = cb->curr;
 bfl = bu->len - (unsigned)(cb->cptr - bu->data);
 if (bfl > len)
   bfl = len;
 memcpy(cb->cptr, str, bfl);
 len -= bfl;
 cb->coff += bfl;
 while (len > 0)
  {
   str += bfl;
   if ((bu = bu->next) == 0)
     bu = newCU(cb, cb->frst->len);
   else
	 cb->curr = bu;
   cb->cptr = bu->data;
   bfl = (len < bu->len) ? len : bu->len;
   memcpy(cb->cptr, str, bfl);
   cb->coff += bfl;
   len -= bfl;
  }
 cb->cptr += bfl;
 if ((cb->cptr - bu->data) == bu->len)
  {
   if (bu->next == 0)
     bu = newCU(cb, cb->frst->len);
   else
   {
     bu = bu->next;
     cb->curr = bu;
   }
   cb->cptr = bu->data;
  }
}
/**
 * Sequence through the chain of character buffers, passing each buffer's
 * beginning and length to a callback function in turn.
 * The callback receives the following four parameters:
 *   const char *start - the start of the current buffer unit
 *   int len - the number of character in the current buffer unit
 *   void *prm - the opaque user parameter passed through
 *   int last - set to 1 on the last buffer unit, 0 otherwise
 * The callback should normally return 0.  If the callback return is less
 * than 0, stringBufferForEach() immediately returns with that value.
 * Otherwise, the maximum value returned by any callback is returned at the
 * end by stringBufferForEach().
 * @param cb a reference to the StringBuffer
 * @param callback a pointer to the callback function
 * @param prm an opaque user parameter that will be passed to the callback
 * @return the maximum integer returned from any of the callback calls, unless
 *     a call returns a value less than 0, in which case the iteration
 *     terminates and this value is returned
*/
int
stringBufferForEach(StringBuffer *cb, StringBufferCallback callback, void *prm)
{
 int grtn, rtn;
 struct Sbu *sv;
 int length, len;
 grtn = 0;
 sv = cb->frst;
 length = cb->coff;
 while (0 != sv)
  {
   len = (length > sv->len) ? sv->len : length;
   rtn = callback(sv->data, len, prm, sv->next == 0);
   length -= len;
   if (rtn != 0)
    {
     if (rtn < 0)
       return rtn;
     if (rtn > grtn)
       grtn = rtn;
    }
   sv = sv->next;
  }
 return grtn;
}
#ifdef TESTDRIVER
#include <stdio.h>
#include <time.h>
static char *list = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
struct cbinfo
{
  int offset;
  int test;
};
static int
callback(const char *cp, int len, void *prm, int last)
{
  struct cbinfo *ip;
  ip = (struct cbinfo *)prm;
  if (strncmp(cp, &list[ip->offset], len) != 0)
  {
    fprintf(stderr, "(%d) ForEach callback compare fail at %d\n", ip->test, ip->offset);
    return -1;
  }
  ip->offset += len;
  return 0;
}
int
main(int argc, char **argv)
{
  StringBuffer vcb;
  StringBuffer *cb;
  StringBuffer *cb2;
  char *cp;
  char *lp;
  int len;
  int llen;
  int test;
  struct cbinfo info;
  time_t testtime = time((time_t *)0);
  fprintf(stderr, "Unit test %s %s\n", __FILE__, ctime(&testtime));
  /* cvinit */
  cb = newStringBuffer((StringBuffer *)0, 0);
  cp = list;
  test = 0;
  while (*cp)
  {
    *stringBufferPostInc(cb) = *cp;
    ++cp;
  }
  ++test;
  llen = strlen(list);
  if (stringBufferGetOffset(cb) != llen)
  {
    fprintf(stderr, "(%d) GetOffset fail\n", test);
  }
  *stringBufferPostInc(cb) = '\0';
  cp = stringBufferPreDec(cb);
  ++test;
  for (lp = list + llen; lp >= list; --lp)
  {
    if (*lp != *cp)
    {
      fprintf(stderr, "(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
    cp = stringBufferPreDec(cb);
  }
  cp = stringBufferSetOffset(cb, llen);
  if (*cp != 0)
  {
    fprintf(stderr, "(%d) SetOffset fail\n", test);
  }
  ++test;
  for (lp = list + llen; lp >= list; )
  {
    cp = stringBufferPostDec(cb);
    if (*lp-- != *cp)
    {
      fprintf(stderr, "(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
  }
  ++test;
  stringBufferSetOffset(cb, 0);
  if (stringBufferGetOffset(cb) != 0)
  {
    fprintf(stderr, "(%d) GetOffset fail\n", test);
  }
  ++test;
  cp = stringBufferGetPointer(cb);
  if (list[0] != *cp)
  {
    fprintf(stderr, "(%d) initial mismatch\n", test);
  }
  ++test;
  for (lp = list; *lp; ++lp)
  {
    if (*lp != *cp)
    {
      fprintf(stderr, "(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
    cp = stringBufferPreInc(cb);
  }
  ++test;
  if (*cp != 0)
  {
    fprintf(stderr, "(%d) string not terminated\n", test);
  }
  ++test;
  stringBufferInc(cb, 1);
  lp = stringBufferToString(cb, &len);
  if (strlen(lp) != len - 1)
  {
    fprintf(stderr, "(%d) string length (%d) failed from ToString (%d)\n",
        test, strlen(lp), len);
  }
  ++test;
  if (strcmp(lp, list) != 0)
  {
    fprintf(stderr, "(%d) string compare failed from ToString\n%s\n", test, lp);
  }
  mFree(lp, "2str");
  delStringBuffer(cb);
  cb = newStringBuffer((StringBuffer *)0, 13);
  cp = list;
  ++test;
  *stringBufferGetPointer(cb) = *cp;
  while (*cp)
  {
    *stringBufferPreInc(cb) = *++cp;
  }
  if (stringBufferGetOffset(cb) != llen)
  {
    fprintf(stderr, "(%d) GetOffset fail\n", test);
  }
  *stringBufferPreInc(cb) = '\0';
  ++test;
  stringBufferSetOffset(cb, 0);
  if (stringBufferGetOffset(cb) != 0)
  {
    fprintf(stderr, "(%d) GetOffset fail\n", test);
  }
  ++test;
  cp = stringBufferGetPointer(cb);
  if (list[0] != *cp)
  {
    fprintf(stderr, "(%d) initial mismatch\n", test);
  }
  ++test;
  for (lp = list; *lp; ++lp)
  {
    if (*lp != *cp)
    {
      fprintf(stderr, "(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
    cp = stringBufferPreInc(cb);
  }
  ++test;
  if (*cp != 0)
  {
    fprintf(stderr, "(%d) string not terminated\n", test);
  }
  ++test;
  stringBufferInc(cb, 1);
  lp = stringBufferToString(cb, &len);
  if (strlen(lp) != len - 1)
  {
    fprintf(stderr, "(%d) string length (%d) failed from ToString (%d)\n",
        test, strlen(lp), len);
  }
  ++test;
  if (strcmp(lp, list) != 0)
  {
    fprintf(stderr, "(%d) string compare failed from ToString\n%s\n", test, lp);
  }
  mFree(lp, "2str2");
  ++test;
  stringBufferSetOffset(cb, 0);
  lp = list;
  for (len = 1; len < 6; ++len)
  {
    cp = stringBufferInc(cb, len);
    lp += len;
    if (*lp != *cp)
    {
      fprintf(stderr, "(%d) stringBufferInc mismatch at offset %d\n", test, lp - list);
      continue;
    }
  }
  ++test;
  stringBufferSetOffset(cb, 52);
  lp = &list[52];
  for (len = 1; len < 6; ++len)
  {
    cp = stringBufferDec(cb, len);
    lp -= len;
    if (*lp != *cp)
    {
      fprintf(stderr, "(%d) stringBufferdec mismatch at offset %d\n", test, lp - list);
      continue;
    }
  }
  delStringBuffer(cb);
  cb = newStringBuffer((StringBuffer *)0, 9);
  ++test;
  /* list is even length */
  for (lp = list; *lp; lp += 2)
  {
    stringBufferAppend(cb, lp, 2);
  }
  *stringBufferPostInc(cb) = '\0';
  lp = stringBufferToString(cb, &len);
  if (strlen(lp) != len - 1)
  {
    fprintf(stderr, "(%d) string length (%d) failed from ToString (%d)\n",
        test, strlen(lp), len);
  }
  ++test;
  if (strcmp(lp, list) != 0)
  {
    fprintf(stderr, "(%d) string compare failed from ToString\n%s\n", test, lp);
  }
  mFree(lp, "2str3");
  info.test = ++test;
  info.offset = 0;
  stringBufferSetOffset(cb, llen);
  ++test;
  if (stringBufferForEach(cb, callback, &info))
  {
    fprintf(stderr, "(%d) ForEach callback returned non-zero\n", test);
  }
  newStringBuffer(&vcb, 0);
  stringBufferAssign(&vcb, cb);
  cb2 = stringBufferCopy(&vcb);
  delStringBuffer(cb2);
  delStringBuffer(cb);
  ++test;
  cb = newStringBuffer((StringBuffer *)0, 13);
  if ((len = stringBufferGetRemainder(cb)) != 13)
  {
    fprintf(stderr, "(%d) Remainder (%d) unexpected \n", test, len);
  }
  cp = stringBufferGetPointer(cb);
  ++test;
  stringBufferInc(cb, 13);
  if ((len = stringBufferGetRemainder(cb)) != 13)
  {
    fprintf(stderr, "(%d) Remainder (%d) unexpected \n", test, len);
  }
  ++test;
  stringBufferDec(cb, 13);
  if (stringBufferGetPointer(cb) != cp)
  {
    fprintf(stderr, "(%d) GetPointer reset mismatch\n", test);
  }
  ++test;
  stringBufferDec(cb, 1);
  if (stringBufferGetOffset(cb) != 0)
  {
    fprintf(stderr, "(%d) GetOffset to -1 fail\n", test);
  }
  delStringBuffer(cb);
  vdeStringBuffer(&vcb);
  /* cvterm */
  trakReport(__FILE__);
  fprintf(stderr, "Unit test %s end\n", __FILE__);
  return 0;
}
#endif
