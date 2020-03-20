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
/* #define MEMORY_TRAK */
#include <wchar.h>
#include "Memory.h"
/**
 * Import the API declarations.
*/
/* #define TESTDRIVER */
#define API_EXPORT
#include "UCStringBuffer.h"
/**
 * Character buffer unit structure
*/
struct UCSbu
{
  /**
   * A link to the previous buffer unit, 0 in the first unit.
  */
  struct UCSbu *next;
  /**
   * A link to the next buffer unit, 0 in the last unit.
  */
  struct UCSbu *back;
  /**
   * The size of the data area.
  */
  unsigned short len;
  /**
   * A reference counter for the UCSbu chain, unused in all but the
   * first buffer on a chain.
  */
  short      ref;
  /**
   * The data area, whose size is given by the member len.  All the units
   * in a single buffer chain have the same size in this implementation.
  */
  Char       data[1];
};
/**
 * The default character buffer size.
*/
static const int bufferSize = 128;
/**
 * Allocate a new character buffer unit and link it at the end of
 * the character buffer unit chain.
 * @param cb the associated UCStringBuffer object
 * @param len the length of the character buffer unit
 * @return the new character buffer unit
*/
static struct UCSbu *
newCU(UCStringBuffer *cb, unsigned int len)
{
 struct UCSbu *bu;
 bu = (struct UCSbu *)mAlloc(sizeof(struct UCSbu) + (len - 1) * sizeof(Char), "UCSBUnit");
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
 * Construct a UCStringBuffer. The maximum allowed unit size is 32767.
 * @param cb a pointer to a derived (or stack) structure, or 0 if none
 * @param len the length of each buffer unit, 0 implies default
 * @return a reference to a new UCStringBuffer
*/
UCStringBuffer *
newUCStringBuffer(UCStringBuffer *cb, unsigned len)
{
  if (len <= 0 || len > 32767)
    len = bufferSize;
  if (cb == 0)
    cb = (UCStringBuffer *)mAlloc(sizeof(UCStringBuffer), "UCStringBuffer");
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
 * Destruct the UCStringBuffer whose reference is passed.  The reference
 * count in the buffer unit chain is decremented and the chain is
 * released if there are no more references.  The UCStringBuffer struct
 * is freed in all cases.
 * @param cb the UCStringBuffer to be destructed
*/
void
delUCStringBuffer(UCStringBuffer *cb)
{
 struct UCSbu *cbup, *cbs;
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
 mFree(cb, "UCStringBuffer");
}
/**
 * Virtually destruct the UCStringBuffer whose reference is passed.  The reference
 * count in the buffer unit chain is decremented and the chain is
 * released if there are no more references.  The UCStringBuffer struct
 * is not freed, the assumption being that it is on the stack, or within
 * a derived structure.
 * @param cb the UCStringBuffer to be virtually destructed
*/
void
vdeUCStringBuffer(UCStringBuffer *cb)
{
 struct UCSbu *cbup, *cbs;
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
 * Copy (shallow) the passed UCStringBuffer, incrementing the
 * reference counter on the chain of buffer units.
 * @param copy the UCStringBuffer to be copied
 * @return a reference to the new copy
*/
UCStringBuffer *
ucstringBufferCopy(UCStringBuffer *copy)
{
  UCStringBuffer *cb;
  cb = (UCStringBuffer *)mAlloc(sizeof(UCStringBuffer), "stringBufcopy");
  cb->frst = copy->frst;
  cb->last = copy->last;
  cb->curr = copy->curr;
  cb->coff = copy->coff;
  cb->cptr = copy->cptr;
  ++cb->frst->ref;
  return cb;
}
/**
 * Assign the second UCStringBuffer to the first, resulting in two
 * UCStringBuffer's pointing to the same chain of buffer units.  A
 * reference count is maintained in the buffer chain so that
 * it will not be deleted until all references to it are gone.
 * @param cb the target of the assignment
 * @param src the source of the assignment
 * @return the newly assigned target
*/
UCStringBuffer *
ucstringBufferAssign(UCStringBuffer *cb, UCStringBuffer *src)
{
 struct UCSbu *cbup, *cbs;
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
 * @param cb a reference to the UCStringBuffer
 * @param amt the increment
 * @return a pointer to the updated character position
*/
Char *
ucstringBufferInc(UCStringBuffer *cb, int amt)
{
 if (amt < 0)
   return ucstringBufferDec(cb, -amt);
 cb->coff += amt;
 while (cb->cptr - cb->curr->data + amt >= cb->curr->len)
  {
   amt -= (int)(&cb->curr->data[cb->curr->len] - cb->cptr);
   if (cb->curr->next == (struct UCSbu *)0)
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
 * @param cb a reference to the UCStringBuffer
 * @param amt the increment
 * @return a pointer to the updated character position
*/
Char *
ucstringBufferDec(UCStringBuffer *cb, int amt)
{
 if (amt < 0)
   return ucstringBufferInc(cb, -amt);
 cb->coff -= amt;
 while (cb->cptr - amt < cb->curr->data)
  {
   amt -= (int)(cb->cptr - cb->curr->data);
   if (cb->curr->back == (struct UCSbu *)0)
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
 * @param cb a reference to the UCStringBuffer
 * @return a pointer to the updated character position
*/
Char *
ucstringBufferPreInc(UCStringBuffer *cb)
{
 ++cb->coff;
 if (++cb->cptr - cb->curr->data >= cb->curr->len)
  {
   if (cb->curr->next == (struct UCSbu *)0)
     newCU(cb, cb->frst->len);
   else
     cb->curr = cb->curr->next;
   cb->cptr = cb->curr->data;
  }
 return cb->cptr;
}
/**
 * Post-increment the passed character buffer pointer.
 * @param cb a reference to the UCStringBuffer
 * @return a pointer to the character position before the increment
*/
Char *
ucstringBufferPostInc(UCStringBuffer *cb)
{
 Char *cp;
 ++cb->coff;
 cp = cb->cptr++;
 if (cb->cptr - cb->curr->data >= cb->curr->len)
  {
   if (cb->curr->next == (struct UCSbu *)0)
     newCU(cb, cb->frst->len);
   else
     cb->curr = cb->curr->next;
   cb->cptr = cb->curr->data;
  }
 return cp;
}
/**
 * Post-decrement the passed character buffer pointer.
 * @param cb a reference to the UCStringBuffer
 * @return a pointer to the character position before the decrement
*/
Char *
ucstringBufferPostDec(UCStringBuffer *cb)
{
 Char *cp;
 --cb->coff;
 cp = cb->cptr--;
 if (cb->cptr < cb->curr->data)
  {
   if (cb->curr->back == (struct UCSbu *)0)
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
 * @param cb a reference to the UCStringBuffer
 * @return a pointer to the updated character position
*/
Char *
ucstringBufferPreDec(UCStringBuffer *cb)
{
 --cb->coff;
 --cb->cptr;
 if (cb->cptr < cb->curr->data)
  {
   if (cb->curr->back == (struct UCSbu *)0)
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
 * @param cb a reference to the UCStringBuffer
 * @return the integer offset of the pointer, always greater than or equal to 0
*/
unsigned
ucstringBufferGetOffset(UCStringBuffer *cb)
{
 return cb->coff;
}
/**
 * Set the pointer offset in the character buffer. If
 * the new offset exceeds the buffer unit chain, additional
 * uninitialized buffer units are added as needed.
 * @param cb a reference to the UCStringBuffer
 * @param offset the new pointer offset
 * @return the updated character pointer
*/
Char *
ucstringBufferSetOffset(UCStringBuffer *cb, unsigned offset)
{
 struct UCSbu *ccb;
 unsigned len;
 len = 0;
 ccb = cb->frst;
 while (0 != ccb && (ccb->len > 0) && (len + ccb->len <= offset))
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
 * @param cb a reference to the UCStringBuffer
 * @return the character pointer
*/
Char *
ucstringBufferGetPointer(UCStringBuffer *cb)
{
 return cb->cptr;
}
/**
 * Return the remaining size in the current character buffer.
 * This supports direct I/O into the buffer unit.  The storage area
 * from ucstringBufferGetPointer() for the length returned by this
 * function is guaranteed to be contiguous.
 * @param cb a reference to the UCStringBuffer
 * @return the remaining size
*/
unsigned
ucstringBufferGetRemainder(UCStringBuffer *cb)
{
 return (unsigned)(&cb->curr->data[cb->curr->len] - cb->cptr);
}
/**
 * Return a copy of the UCStringBuffer buffer contents in newly allocated
 * contiguous storage, from the beginning of the UCStringBuffer
 * to its current offset.  The returned value is not 0-terminated unless
 * a 0 has been stored at the end of the UCStringBuffer.
 * @param cb a reference to the UCStringBuffer
 * @param lp if non-zero, a pointer to an integer where the size of the
 *   return buffer is stored
 * @return a pointer to the newly allocated copy
*/
Char *
ucstringBufferToString(UCStringBuffer *cb, unsigned *lp)
{
 unsigned len;
 struct UCSbu *sv;
 Char *str, *cp;
 len = cb->coff;
 cp = str = (Char *)mAlloc(sizeof(Char) * len,"ucsBuf2string");
 sv = cb->frst;
 while (cb->curr != sv)
  {
   wcsncpy(cp, sv->data, sv->len);
   cp += sv->len;
   sv = sv->next;
  }
 wcsncpy(cp, sv->data, cb->cptr - sv->data);
 if (lp != 0)
   *lp = len;
 return str;
}
/**
 * Append a contiguous string at the current position in a character buffer.
 * @param cb a reference to the UCStringBuffer
 * @param str a reference to the string to be appended
 * @param len if non-zero, the number of character to be appended; if 0,
 *     the string will be assumed to be 0-terminated, and the length will
 *     be determined by a call to strlen(); note this implies that the
 *     terminating 0-byte will NOT be appended
*/
void
ucstringBufferAppend(UCStringBuffer *cb, const Char *str, unsigned len)
{
 unsigned bfl;
 struct UCSbu *bu;
 if (len <= 0)
   len = (unsigned)wcslen(str);
 bu = cb->curr;
 bfl = bu->len - (unsigned)(cb->cptr - bu->data);
 if (bfl > len)
   bfl = len;
 wcsncpy(cb->cptr, str, bfl);
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
   wcsncpy(cb->cptr, str, bfl);
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
 * than 0, ucstringBufferForEach() immediately returns with that value.
 * Otherwise, the maximum value returned by any callback is returned at the
 * end by ucstringBufferForEach().
 * @param cb a reference to the UCStringBuffer
 * @param callback a pointer to the callback function
 * @param prm an opaque user parameter that will be passed to the callback
 * @return the maximum integer returned from any of the callback calls, unless
 *     a call returns a value less than 0, in which case the iteration
 *     terminates and this value is returned
*/
int
ucstringBufferForEach(UCStringBuffer *cb, UCStringBufferCallback callback, void *prm)
{
 int grtn, rtn;
 struct UCSbu *sv;
 unsigned length, len;
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
static Char *list = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
struct cbinfo
{
  unsigned offset;
  int test;
};
static int
callback(const Char *cp, unsigned len, void *prm, unsigned last)
{
  struct cbinfo *ip;
  ip = (struct cbinfo *)prm;
  if (wcsncmp(cp, &list[ip->offset], len) != 0)
  {
    fwprintf(stderr, L"(%d) ForEach callback compare fail at %d\n", ip->test, ip->offset);
    return -1;
  }
  ip->offset += len;
  return 0;
}
int
main(int argc, char **argv)
{
  UCStringBuffer vcb;
  UCStringBuffer *cb;
  UCStringBuffer *cb2;
  Char *cp;
  Char *lp;
  unsigned len;
  unsigned llen;
  int test;
  struct cbinfo info;
  time_t testtime = time((time_t *)0);
  fwprintf(stderr, L"Unit test %s %s\n", __FILE__, ctime(&testtime));
  /* cvinit */
  cb = newUCStringBuffer((UCStringBuffer *)0, 0);
  cp = list;
  test = 0;
  while (*cp)
  {
    *ucstringBufferPostInc(cb) = *cp;
    ++cp;
  }
  ++test;
  llen = wcslen(list);
  if (ucstringBufferGetOffset(cb) != llen)
  {
    fwprintf(stderr, L"(%d) GetOffset fail\n", test);
  }
  *ucstringBufferPostInc(cb) = '\0';
  cp = ucstringBufferPreDec(cb);
  ++test;
  for (lp = list + llen; lp >= list; --lp)
  {
    if (*lp != *cp)
    {
      fwprintf(stderr, L"(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
    cp = ucstringBufferPreDec(cb);
  }
  cp = ucstringBufferSetOffset(cb, llen);
  if (*cp != 0)
  {
    fwprintf(stderr, L"(%d) SetOffset fail\n", test);
  }
  ++test;
  for (lp = list + llen; lp >= list; )
  {
    cp = ucstringBufferPostDec(cb);
    if (*lp-- != *cp)
    {
      fwprintf(stderr, L"(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
  }
  ++test;
  ucstringBufferSetOffset(cb, 0);
  if (ucstringBufferGetOffset(cb) != 0)
  {
    fwprintf(stderr, L"(%d) GetOffset fail\n", test);
  }
  ++test;
  cp = ucstringBufferGetPointer(cb);
  if (list[0] != *cp)
  {
    fwprintf(stderr, L"(%d) initial mismatch\n", test);
  }
  ++test;
  for (lp = list; *lp; ++lp)
  {
    if (*lp != *cp)
    {
      fwprintf(stderr, L"(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
    cp = ucstringBufferPreInc(cb);
  }
  ++test;
  if (*cp != 0)
  {
    fwprintf(stderr, L"(%d) string not terminated\n", test);
  }
  ++test;
  ucstringBufferInc(cb, 1);
  lp = ucstringBufferToString(cb, &len);
  if (wcslen(lp) != len - 1)
  {
    fwprintf(stderr, L"(%d) string length (%d) failed from ToString (%d)\n",
        test, wcslen(lp), len);
  }
  ++test;
  if (wcscmp(lp, list) != 0)
  {
    fwprintf(stderr, L"(%d) string compare failed from ToString\n%s\n", test, lp);
  }
  mFree(lp, "2str");
  delUCStringBuffer(cb);
  cb = newUCStringBuffer((UCStringBuffer *)0, 13);
  cp = list;
  ++test;
  *ucstringBufferGetPointer(cb) = *cp;
  while (*cp)
  {
    *ucstringBufferPreInc(cb) = *++cp;
  }
  if (ucstringBufferGetOffset(cb) != llen)
  {
    fwprintf(stderr, L"(%d) GetOffset fail\n", test);
  }
  *ucstringBufferPreInc(cb) = '\0';
  ++test;
  ucstringBufferSetOffset(cb, 0);
  if (ucstringBufferGetOffset(cb) != 0)
  {
    fwprintf(stderr, L"(%d) GetOffset fail\n", test);
  }
  ++test;
  cp = ucstringBufferGetPointer(cb);
  if (list[0] != *cp)
  {
    fwprintf(stderr, L"(%d) initial mismatch\n", test);
  }
  ++test;
  for (lp = list; *lp; ++lp)
  {
    if (*lp != *cp)
    {
      fwprintf(stderr, L"(%d) mismatch at offset %d\n", test, lp - list);
      continue;
    }
    cp = ucstringBufferPreInc(cb);
  }
  ++test;
  if (*cp != 0)
  {
    fwprintf(stderr, L"(%d) string not terminated\n", test);
  }
  ++test;
  ucstringBufferInc(cb, 1);
  lp = ucstringBufferToString(cb, &len);
  if (wcslen(lp) != len - 1)
  {
    fwprintf(stderr, L"(%d) string length (%d) failed from ToString (%d)\n",
        test, wcslen(lp), len);
  }
  ++test;
  if (wcscmp(lp, list) != 0)
  {
    fwprintf(stderr, L"(%d) string compare failed from ToString\n%s\n", test, lp);
  }
  mFree(lp, "2str2");
  ++test;
  ucstringBufferSetOffset(cb, 0);
  lp = list;
  for (len = 1; len < 6; ++len)
  {
    cp = ucstringBufferInc(cb, (int)len);
    lp += len;
    if (*lp != *cp)
    {
      fwprintf(stderr, L"(%d) ucstringBufferInc mismatch at offset %d\n", test, lp - list);
      continue;
    }
  }
  ++test;
  ucstringBufferSetOffset(cb, 52);
  lp = &list[52];
  for (len = 1; len < 6; ++len)
  {
    cp = ucstringBufferDec(cb, (int)len);
    lp -= len;
    if (*lp != *cp)
    {
      fwprintf(stderr, L"(%d) ucstringBufferdec mismatch at offset %d\n", test, lp - list);
      continue;
    }
  }
  delUCStringBuffer(cb);
  cb = newUCStringBuffer((UCStringBuffer *)0, 9);
  ++test;
  /* list is even length */
  for (lp = list; *lp; lp += 2)
  {
    ucstringBufferAppend(cb, lp, 2);
  }
  *ucstringBufferPostInc(cb) = L'\0';
  lp = ucstringBufferToString(cb, &len);
  if (wcslen(lp) != len - 1)
  {
    fwprintf(stderr, L"(%d) string length (%d) failed from ToString (%d)\n",
        test, wcslen(lp), len);
  }
  ++test;
  if (wcscmp(lp, list) != 0)
  {
    fwprintf(stderr, L"(%d) string compare failed from ToString\n%s\n", test, lp);
  }
  mFree(lp, "2str3");
  info.test = ++test;
  info.offset = 0;
  ucstringBufferSetOffset(cb, llen);
  ++test;
  if (ucstringBufferForEach(cb, callback, &info))
  {
    fwprintf(stderr, L"(%d) ForEach callback returned non-zero\n", test);
  }
  newUCStringBuffer(&vcb, 0);
  ucstringBufferAssign(&vcb, cb);
  cb2 = ucstringBufferCopy(&vcb);
  delUCStringBuffer(cb2);
  delUCStringBuffer(cb);
  ++test;
  cb = newUCStringBuffer((UCStringBuffer *)0, 13);
  if ((len = ucstringBufferGetRemainder(cb)) != 13)
  {
    fwprintf(stderr, L"(%d) Remainder (%d) unexpected \n", test, len);
  }
  cp = ucstringBufferGetPointer(cb);
  ++test;
  ucstringBufferInc(cb, 13);
  if ((len = ucstringBufferGetRemainder(cb)) != 13)
  {
    fwprintf(stderr, L"(%d) Remainder (%d) unexpected \n", test, len);
  }
  ++test;
  ucstringBufferDec(cb, 13);
  if (ucstringBufferGetPointer(cb) != cp)
  {
    fwprintf(stderr, L"(%d) GetPointer reset mismatch\n", test);
  }
  ++test;
  ucstringBufferDec(cb, 1);
  if (ucstringBufferGetOffset(cb) != 0)
  {
    fwprintf(stderr, L"(%d) GetOffset to -1 fail\n", test);
  }
  delUCStringBuffer(cb);
  vdeUCStringBuffer(&vcb);
  /* cvterm */
#ifdef MEMORY_TRAK
  trakReport(__FILE__);
#endif
  fwprintf(stderr, L"Unit test %s end\n", __FILE__);
  return 0;
}
#endif
