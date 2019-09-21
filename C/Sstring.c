/**
 * Set of functions to manage character strings.
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define STRING_EXPORT
#include "Sstring.h"
#include "Memory.h"
/**
 * Construct a String.
 * @param len the desired length of the String
 * @return a reference to a new String
*/
String *
newString(unsigned len)
{
  String *str;
  str = (String *)mAlloc(len + 1, "String");
  *str = 0;
  return str;
}
/**
 * Destruct the String whose reference is passed.
 * @param str the String to be destructed
*/
void
delString(String *str)
{
 mFree(str, "String");
}
/**
 * Return a newly allocated copy of the passed string.
 * @param str the String to be copied
*/
String *
stringCopy(const String *str)
{
 String *cp;
 int len;
 len = (int)strlen(str) + 1;
 cp = mAlloc(len, "StringCopy");
 memcpy(cp, str, len);
 return cp;
}
/**
 * Return a copy of the substring from start to, but not including, end.
 * @param start the first character of the String to be copied
 * @param end a pointer to the character following the last character of
 *     the String to be copied
*/
String *
stringCut(const char *start, const char *end)
{
 char *cp;
 int n;
 n = (int)(end - start);
 if (n < 0)
   n = 0;
 cp = (char *)mAlloc(n + 1, "StringCut");
 memcpy(cp, start, n);
 cp[n] = '\0';
 return (String *)cp;
}
/**
 * Returns the concatenation of the two passed strings.
 * @param str1 the head of the resulting concatenation
 * @param str2 the tail of the resulting concatenation
*/
String *
stringConcat(const String *str1, const String *str2)
{
 String *cp;
 int ln1;
 int ln2;
 ln1 = (int)strlen(str1);
 ln2 = (int)strlen(str2) + 1;
 cp = (String *)mAlloc(ln1 + ln2, "StringConcat");
 memcpy(cp, str1, ln1);
 memcpy(cp + ln1, str2, ln2);
 return cp;
}
/**
 * Returns the concatenation of the two passed character arrays.
 * If ln1 or ln2 is zero, the corresponding array is ignored.
 * @param chr1 the head of the resulting concatenation
 * @param ln1 the length of first array
 * @param chr2 the tail of the resulting concatenation
 * @param ln2 the length of second array
*/
String *
stringConcat2(const char *chr1, unsigned ln1, const char *chr2, unsigned ln2)
{
 String *cp;
 unsigned len;
 len = ln1 + ln2;
 cp = (String *)mAlloc(len + 1, "StringConcat2");
 if (ln1 > 0)
   memcpy(cp, chr1, ln1);
 if (ln2 > 0)
   memcpy(cp + ln1, chr2, ln2);
 cp[len] = 0;
 return cp;
}
/**
 * Returns the concatenation of all the passed strings.  This is an
 * internal function that operates on the variable argument list passed
 * to the calling function.  The argument list is terminated by
 * a 0-pointer.
 * @param str1 the first of the strings to be concatenated
 * @param ax a pointer to the argument list of additional strings
*/
static String *
stringConcatX(const String *str1, va_list ax)
{
 String *str;
 char *cp;
 va_list ap;
 unsigned n, ln;
 const String *nxtarg;
 va_copy(ap, ax);
 n = ln = 0;
 for (nxtarg = str1; nxtarg != 0; nxtarg = va_arg(ap, String *))
 {
   ln += strlen(nxtarg);
   ++n;
 }
 cp = str = mAlloc(ln + 1, "strConcatX");
 va_copy(ap, ax);
 for (nxtarg = str1; n > 0; nxtarg = va_arg(ap, String *), --n)
 {
   ln = (int)strlen(nxtarg);
   memcpy(cp, nxtarg, ln);
   cp += ln;
 }
 *cp = 0;
 return str;
}
/**
 * Returns the concatenation of all the passed strings; the varying
 * argument lsit is terminated by a 0-pointer.
 * @param str1 the first of the strings to be concatenated
*/
String *
stringConcatV(const String *str1, ...)
{
 String *cp;
 va_list ap;
 va_start(ap, str1);
 cp = stringConcatX(str1, ap);
 va_end(ap);
 return cp;
}
/**
 * Appends the second string to the first and returns the modified
 * first string. Storage allocated to the first argument
 * is freed.
 * @param str1p a pointer to the reference to the target string
 * @param str2 a reference to the string to be appended
*/
void
stringAppend(String **str1p, const String *str2)
{
 String *cp;
 cp = stringConcat(*str1p, str2);
 mFree(*str1p, "stringAppend");
 *str1p = cp;
}
/**
 * Appends the second and subsequent strings to the first and returns
 * the modified first string. Storage allocated to the first argument
 * is freed.
 * @param str1p a pointer to the reference to the target string
*/
void
stringAppendV(String **str1p, ...)
{
 String *cp;
 va_list ap;
 va_start(ap, str1p);
 cp = stringConcatX(*str1p, ap);
 va_end(ap);
 mFree(*str1p, "stringAppendV");
 *str1p = cp;
}
/**
 * Make a copy of the source string and assigns it to the string pointed
 * to by target.  If *target is not 0, its previous value is freed.
 * @param target a pointer to the reference to the target string
 * @param source a reference to the string to be assigned
*/
void
stringAssign(String **target, const String *source)
{
 String *str;
 if (*target == source)
   return;
 str = stringCopy(source);
 if (*target != 0)
   mFree(*target, "stringAssign");
 *target = str;
}
/**
 * Copy and assign the passed source strings to the string pointed 
 * by *target.  If *target is not 0, its previous value is freed.
 * @param target a pointer to the reference to the target string
 * @param source a reference to the first of the list of strings to
 *     be concatenated and assigned
*/
void
stringAssignV(String **target, const String *source, ...)
{
 String *cp;
 va_list ap;
 va_start(ap, source);
 cp = stringConcatX(source, ap);
 va_end(ap);
 if (*target != 0)
   mFree(*target, "strAssignV");
 *target = cp;
}
/**
 * Returns 1 if the string contains only digit characters,
 * or 0 otherwise.
 * @param str a reference to the string to be scanned
*/
int
stringIsDigits(const String *str)
{
 while (*str != 0)
 {
   if (!isdigit(*str))
      return 0;
   ++str;
 }
 return 1;
}
/**
 * Return 1 if the string contains only whitespace,
 * otherwise return 0.
 * @param str a reference to the string to be scanned
*/
int
stringIsBlank(const String *str)
{
 while (*str != 0)
 {
   if (!isspace(*str))
      return 0;
   ++str;
 }
 return 1;
}
/**
 * Returns a new allocated string with all alphabetic characters
 * converted to upper case.
 * @param str a reference to the string to be converted
*/
char *
stringToupper(const char *str)
{
  char *nw;
  const char *fp;
  char *tp;
  unsigned len;
  len = (int)strlen(str);
  nw = (String *)mAlloc(len + 1, "StringToupper");
  for (tp = nw, fp = str; *fp != 0; ++tp, ++fp)
    *tp = toupper(*fp);
  *tp = *fp;
  return nw;
}
/**
 * Returns a new allocated string with all alphabetic characters
 * converted to lower case.
 * @param str a reference to the string to be converted
*/
char *
stringTolower(const char *str)
{
  char *nw;
  const char *fp;
  char *tp;
  unsigned len;
  len = (int)strlen(str);
  nw = (String *)mAlloc(len + 1, "StringTolower");
  for (tp = nw, fp = str; *fp != 0; ++tp, ++fp)
    *tp = tolower(*fp);
  *tp = *fp;
  return nw;
}
/**
 * Returns a new allocated string
 * with leading and trailing whitespace characters removed.
 * @param str a reference to the string to be trimmed
*/
String *
stringTrim(const String *str)
{
  const char *cf;
  const char *ce;
  String *nw;
  unsigned len = (unsigned)strlen(str);
  cf = &str[len - 1];
  ce = str;
  while (ce < cf && isspace(*cf))
  {
    --cf;
  }
  ++cf;
  while (ce < cf && isspace(*ce))
  {
    ++ce;
  }
  nw = stringCut(ce, cf);
  return nw ;
}
#if defined(TESTDRIVER)
#include <stdio.h>
#define ArraySize(a) ((sizeof a) / (sizeof a[0]))
static char *list = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char *array[] = {"0123456789", "ABCDEFGHIJKL","MNOPQRSTUVW","XYZ",
    "abcdefghijkl", "mnopqrs", "tuvwxyz"};
int
main(int argc, char **argv)
{
  String *str;
  String *str2;
  String *str3;
  int l1, l2;
  int llen;
  int ix;
  int test;
  /* cvinit */
  test = 0;
  llen = strlen(list);
  ++test;
  str = stringCopy(list);
  if (strcmp(str, list) != 0)
  {
    fprintf(stderr, "(%d) stringCopy fail\n", test);
  }
  l1 = strlen(array[0]);
  l2 = strlen(array[1]);
  ++test;
  str2 = stringCut(list + l1, list + l1 + l2);
  if (strcmp(str2, array[1]) != 0)
  {
    fprintf(stderr, "(%d) stringCut fail (%s)\n", test, str2);
  }
  delString(str);
  delString(str2);
  str2 = stringCut(list + 2, list);
  if (strlen(str2) != 0)
  {
    fprintf(stderr, "(%d) stringCut reverse fail\n", test);
  }
  delString(str2);
  str2 = stringCopy(array[0]);
  ++test;
  for (ix = 1; ix < ArraySize(array); ++ix)
  {
    stringAppend(&str2, array[ix]);
  }
  if (strcmp(str2, list) != 0)
  {
    fprintf(stderr, "(%d) stringAppend fail\n", test);
  }
  delString(str2);
  ++test;
  str = stringCopy(array[0]);
  str2 = stringCopy(str);
  stringAppend(&str, array[1]);
  stringAppend(&str, array[2]);
  stringAppend(&str, array[3]);
  stringAppendV(&str2, array[1], array[2], array[3], (String *)0);
  if (strcmp(str, str2) != 0)
  {
    fprintf(stderr, "(%d) stringAppendV fail\n", test);
  }
  ++test;
  stringAssign(&str, array[0]);
  if (strcmp(str, array[0]) != 0)
  {
    fprintf(stderr, "(%d) stringAssign fail\n", test);
  }
  ++test;
  stringAssignV(&str, array[0], array[1], array[2], array[3], (String *)0);
  if (strcmp(str, str2) != 0)
  {
    fprintf(stderr, "(%d) stringAssignV fail\n", test);
  }
  delString(str);
  ++test;
  str = stringConcat(array[0], array[0]);
  stringAssign(&str2, array[0]);
  stringAppend(&str2, str2);
  if (strcmp(str, str2) != 0)
  {
    fprintf(stderr, "(%d) stringConcat fail\n", test);
  }
  delString(str2);
  ++test;
  str2 = stringConcat2(array[0], strlen(array[0]), array[0], strlen(array[0]));
  if (strcmp(str, str2) != 0)
  {
    fprintf(stderr, "(%d) stringConcat2 fail\n", test);
  }
  ++test;
  str3 = stringConcat(str, str);
  delString(str);
  stringAppend(&str2, str2);
  if (strcmp(str2, str3) != 0)
  {
    fprintf(stderr, "(%d) stringConcat fail\n", test);
  }
  ++test;
  str = stringConcatV(array[0], array[0], array[0], array[0], (String *)0);
  if (strcmp(str, str3) != 0)
  {
    fprintf(stderr, "(%d) stringConcatV fail\n", test);
  }
  ++test;
  if (stringIsDigits(array[0]) == 0)
  {
    fprintf(stderr, "(%d) stringIsDigits fail\n", test);
  }
  ++test;
  if (stringIsDigits(array[1]) != 0)
  {
    fprintf(stderr, "(%d) stringIsDigits fail\n", test);
  }
  delString(str2);
  delString(str3);
  ++test;
  str2 = stringTrim(str);
  if (strcmp(str, str2) != 0)
  {
    fprintf(stderr, "(%d) stringTrim fail (%s)\n", test, str2);
  }
  delString(str2);
  ++test;
  if (stringIsBlank("      ") == 0)
  {
    fprintf(stderr, "(%d) stringIsBlank fail\n", test);
  }
  ++test;
  str2 = stringTrim("      ");
  if (strlen(str2) != 0)
  {
    fprintf(stderr, "(%d) stringTrim fail (%s)\n", test, str2);
  }
  delString(str2);
  ++test;
  str2 = stringTrim("  ab  ");
  if (strcmp(str2, "ab") != 0)
  {
    fprintf(stderr, "(%d) stringTrim fail (%s)\n", test, str2);
  }
  ++test;
  if (stringIsBlank(str2) != 0)
  {
    fprintf(stderr, "(%d) stringIsBlank fail\n", test);
  }
  delString(str2);
  delString(str);
  ++test;
  str = stringToupper(array[4]);
  if (strcmp(str, array[1]) != 0)
  {
    fprintf(stderr, "(%d) stringToupper fail\n", test);
  }
  delString(str);
  ++test;
  str = stringTolower(array[1]);
  if (strcmp(str, array[4]) != 0)
  {
    fprintf(stderr, "(%d) stringTolower fail\n", test);
  }
  delString(str);
  ++test;
  str = newString(5);
  if (strlen(str) != 0)
  {
    fprintf(stderr, "(%d) newString fail\n", test);
  }
  memcpy(str, "12345", 6);
  if (strcmp(str, "12345") != 0)
  {
    fprintf(stderr, "(%d) newString(5) fail\n", test);
  }
  delString(str);
  /* cvterm */
  trakReport(__FILE__);
  return 0;
}
#endif
