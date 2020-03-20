#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <ctype.h>
#include <wctype.h>
#include <time.h>

#include "Memory.h"
#include "NpxElement.h"
#include "NpxUCElement.h"
#include "NpxRoot.h"
#include "StringBuffer.h"
#include "UCStringBuffer.h"

#define UCS_CHAR
#ifdef UCS_CHAR
typedef wchar_t Char;
#else
typedef char Char;
#endif

#define self(s) (*s->mtb)
#define arraySize(a) ((sizeof a)/(sizeof a[0]))

/*
#define TAMBUILDTIME
*/
#ifdef TAMBUILDTIME
clock_t us_TAMBuild;
#define tamStart(x) x = clock();
#define tamStop(x) us_TAMBuild += clock() - x;
#define tamClockDcl(x) clock_t x;
#else
#define tamStart(x)
#define tamStop(x)
#define tamClockDcl(x)
#endif
typedef char String;

/**
 * Convert the passed digit character to its integer value.
 * @param ch a digit character
 * @return the passed character's integer value, 0 for non-digits
*/
static int numericValue(const char ch)
{
    switch (ch)
    {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        default:
            return 0;
    }
}
static int numericValueUC(const Char ch)
{
    switch (ch)
    {
        case L'1':
            return 1;
        case L'2':
            return 2;
        case L'3':
            return 3;
        case L'4':
            return 4;
        case L'5':
            return 5;
        case L'6':
            return 6;
        case L'7':
            return 7;
        case L'8':
            return 8;
        case L'9':
            return 9;
        default:
            return 0;
    }
}
/**
 * Convert the passed digit character to its integer value.
 * @param ch a digit character
 * @return the passed character's integer value, 0 for non-digits
 */
static char charValue(const int val)
{
    switch (val)
    {
        case 1:
            return '1';
        case 2:
            return '2';
        case 3:
            return '3';
        case 4:
            return '4';
        case 5:
            return '5';
        case 6:
            return '6';
        case 7:
            return '7';
        case 8:
            return '8';
        case 9:
            return '9';
        default:
            return '0';
    }
}
/**
 * Scan and convert a sequence of digit characters in the passed string to an integer.
 * @param str the string to be scanned
 * @param idx the starting index for the scan
 * @param rtn a 2-integer array for the return of the scanned number and the following index
 * @return the index of the first non-digit character encountered, equal to rtn[1]
*/
static int fieldLength(const String *str, int idx, int rtn[])
{
    int acc = 0;
    while (isdigit(str[idx]))
    {
        acc = acc * 10 + numericValue(str[idx++]);
    }
    rtn[0] = acc;
    return rtn[1] = idx;
}
static int fieldLengthUC(const Char *str, int idx, int rtn[])
{
    int acc = 0;
    while (iswdigit(str[idx]))
    {
        acc = acc * 10 + numericValueUC(str[idx++]);
    }
    rtn[0] = acc;
    return rtn[1] = idx;
}
/**
 * Compute the number of UCS characters needs to store a UTF-8 string.
 * @param str the input UTF-8 string
 * @param ln the number of bytes in the UTF-8 string
 * @return the number of UCS characters
 */
static int ucslen(const unsigned char *str, int ln)
{
    int ix, usz;
    unsigned char ch;
    usz = ix = 0;
    while (ix < ln)
    {
        ch = str[ix];
        if (ch < 0x80)
        {
            ix += 1;
        }
        else if (ch >= 0xc0)
        {
            if (ch < 0xe0)
            {
                ix += 2;
            }
            else if (ch < 0xf0)
            {
                ix += 3;
            }
            else if (ch < 0xf8)
            {
                ix += 4;
            }
            else if (ch < 0xfc)
            {
                ix += 5;
            }
            else if (ch < 0xfe)
            {
                ix += 6;
            }
            else
            {
                /* encoding error */
                return -1;
            }
        }
        else
        {
            /* encoding error */
            return -1;
        }
        ++usz;
    }
    return usz;
}
static int utf82UCS(Char *wbuf, const unsigned char *str, int ln)
{
    int ix, jx;
    unsigned char ch;
    jx = ix = 0;
    while (ix < ln)
    {
        ch = str[ix];
        if (ch < 0x80)
        {
            wbuf[jx] = (wchar_t) ch;
            ix += 1;
        }
        else if (ch >= 0xc0)
        {
            if (ch < 0xe0)
            {
                wbuf[jx] = (wchar_t) ((ch & 0x1fu) << 6 | (str[ix + 1] & 0x3fu));
                ix += 2;
            }
            else if (ch < 0xf0)
            {
                wbuf[jx] = (wchar_t) (((ch & 0x0fu) << 12) | ((str[ix + 1] & 0x3fu) << 6) |
                                                          (str[ix + 2] & 0x3f));
                ix += 3;
            }
            else if (ch < 0xf8)
            {
                wbuf[jx] = (wchar_t) (((ch & 0x07u) << 18) | ((str[ix + 1] & 0x3fu) << 12) |
                                                          ((str[ix + 2] & 0x3fu) << 6) | (str[ix + 3] & 0x3fu));
                ix += 4;
            }
            else if (ch < 0xfc)
            {
                wbuf[jx] = (wchar_t) (((ch & 0x03u) << 24) | ((str[ix + 1] & 0x3fu) << 18) |
                                                          ((str[ix + 2] & 0x3fu) << 12) | ((str[ix + 3] & 0x3fu) << 6) |
                                                          (str[ix + 4] & 0x3fu));
                ix += 5;
            }
            else if (ch < 0xfe)
            {
                wbuf[jx] = (wchar_t) (((ch & 0x01u) << 30) | ((str[ix + 1] & 0x3fu) << 24) |
                                                          ((str[ix + 2] & 0x3fu) << 18) | ((str[ix + 3] & 0x3fu) << 12) |
                                                          ((str[ix + 4] & 0x3fu) <<  6) |  (str[ix + 4] & 0x3fu));
                ix += 6;
            }
            else
            {
                /* encoding error */
                return -1;
            }
        }
        else
        {
            /* encoding error */
            return -1;
        }
        jx += 1;
    }
    return jx;
}
static UCStringBuffer *utf8toUCS(const unsigned char *str, int ln)
{
    int ix;
    unsigned char ch;
    UCStringBuffer *wbuf;
    wbuf = newUCStringBuffer((UCStringBuffer *) 0, 0);
    ix = 0;
    while (ix < ln)
    {
        ch = str[ix];
        if (ch < 0x80)
        {
            *ucstringBufferPostInc(wbuf) = (wchar_t) ch;
            ix += 1;
        }
        else if (ch >= 0xc0)
        {
            if (ch < 0xe0)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) ((ch & 0x1fu) << 6 | (str[ix + 1] & 0x3fu));
                ix += 2;
            }
            else if (ch < 0xf0)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x0fu) << 12) | ((str[ix + 1] & 0x3fu) << 6) |
                                                          (str[ix + 2] & 0x3f));
                ix += 3;
            }
            else if (ch < 0xf8)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x07u) << 18) | ((str[ix + 1] & 0x3fu) << 12) |
                                                          ((str[ix + 2] & 0x3fu) << 6) | (str[ix + 3] & 0x3fu));
                ix += 4;
            }
            else if (ch < 0xfc)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x03u) << 24) | ((str[ix + 1] & 0x3fu) << 18) |
                                                          ((str[ix + 2] & 0x3fu) << 12) | ((str[ix + 3] & 0x3fu) << 6) |
                                                          (str[ix + 4] & 0x3fu));
                ix += 5;
            }
            else if (ch < 0xfe)
            {
                *ucstringBufferPostInc(wbuf) = (wchar_t) (((ch & 0x01u) << 30) | ((str[ix + 1] & 0x3fu) << 24) |
                                                          ((str[ix + 2] & 0x3fu) << 18) | ((str[ix + 3] & 0x3fu) << 12) |
                                                          ((str[ix + 4] & 0x3fu) <<  6) |  (str[ix + 4] & 0x3fu));
                ix += 6;
            }
            else
            {
                /* encoding error */
                delUCStringBuffer(wbuf);
                return 0;
            }
        }
        else
        {
            /* encoding error */
            delUCStringBuffer(wbuf);
            return 0;
        }
    }
    return wbuf;
}
static int extendedName(const char *str, int idx)
{
    char ch;
    while ((ch = str[idx]) >= arraySize(ctype) || 0 == (ctype[ch] & ST))
    {
        if (*ESCAPE == ch)
            ++idx;
        ++idx;
    }
    return idx;
}
static int extendedNameUC(const Char *str, int idx)
{
    Char ch;
    while ((ch = str[idx]) >= arraySize(ctype) || 0 == (ctype[ch] & ST))
    {
        if (*ESCAPE == (char)ch)
            ++idx;
        ++idx;
    }
    return idx;
}
/**
 * Extract each of n content elements from the next position in the TSF string.
 * @param npxElement the NpxElement object being built from the content
 * @param str the TSF string
 * @param ct the number of content elements to extract
 * @param ndx the beginning index of the content in the TSF string
 * @return the index of the character after the extracted content
*/
static int
processElems(NpxElement *npxElement, String *str, const int ct, int ndx, char typ)
{
    int idx, ix;
    int rtn[2];
    unsigned char ch;
    NpxElement *elm;
    tamClockDcl(tam)
    idx = ndx;
    for (ix = 0; ix < ct; ++ix)
    {
//        if (6386 == ct)
//            fprintf(stderr, "processElems %d loop %d index %d\n", ct, ix, idx);
//        else if (34 == ct)
//            fprintf(stderr, "processElems %d loop %d index %d\n", ct, ix, idx);
        ndx = fieldLength(str, idx, rtn);
        str[idx] = '\0';  // remove temporarily to see full string
        //idx = rtn[0] + ndx + 1;
        ch = str[ndx];
        if (ch >= arraySize(ctype) || 0 == (ctype[ch] & ST)) // has a name
        {
//            tamStart(tam)
//            self(npxElement).add(npxElement, ch, ndx + 1, idx);
//            tamStop(tam)
//            goto l1; // effectively, continue outer
            if (*ENAME == ch)
            {
                ndx = extendedName(str, ndx + 1);
                ch = str[ndx];
            }
            else
            {
                while ((ch = str[ndx]) >= arraySize(ctype) || 0 == (ctype[ch] & ST))
                {
                    ++ndx;
                }
            }
        }
        if (ch < arraySize(ctype) && 0 != (ctype[ch] & PL))
        {
            str[ndx] = '\0';
            idx = rtn[0] + ndx + 1;
            self(npxElement).add(npxElement, str, rtn[1], ndx, idx, (char)ch);
            continue;
        }
        if (0 < rtn[0])
            elm = newNpxElement(str, rtn[0], npxElement);
        else
            elm = (NpxElement *)0;
        idx = ndx;
        idx = processElems(elm, str, rtn[0], idx + 1, (char)ch);
        self(npxElement).addElem(npxElement, &str[rtn[1]], elm, (char)ch);
        str[ndx] = '\0';
    }
//    if (555642 == idx)
//      fprintf(stderr, "processElems return index %d\n", idx);
    return idx;
}
static int
processElemsUC(NpxUCElement *npxElement, Char *str, const int ct, int ndx, char typ)
{
    int idx, ix;
    int rtn[2];
    Char ch;
    NpxUCElement *elm;
    tamClockDcl(tam)
    idx = ndx;
    for (ix = 0; ix < ct; ++ix)
    {
//        if (6386 == ct)
//            fprintf(stderr, "processElems %d loop %d index %d\n", ct, ix, idx);
//        else if (34 == ct)
//            fprintf(stderr, "processElems %d loop %d index %d\n", ct, ix, idx);
        ndx = fieldLengthUC(str, idx, rtn);
        str[idx] = L'\0';  // remove temporarily to see full string
        //idx = rtn[0] + ndx + 1;
        ch = str[ndx];
        if (ch >= arraySize(ctype) || 0 == (ctype[ch] & ST)) // has a name
        {
            if (*ENAME == (char)ch)
            {
                ndx = extendedNameUC(str, ndx + 1);
                ch = str[ndx];
            }
            else
            {
                while ((ch = str[ndx]) >= arraySize(ctype) || 0 == (ctype[ch] & ST))
                {
                    ++ndx;
                }
            }
        }
        if (ch < arraySize(ctype) && 0 != (ctype[ch] & PL))
        {
            str[ndx] = L'\0';
            idx = rtn[0] + ndx + 1;
            self(npxElement).add(npxElement, str, rtn[1], ndx, idx, (char)ch);
            continue;
        }
        if (0 < rtn[0])
            elm = newNpxUCElement(str, rtn[0], npxElement);
        else
            elm = (NpxUCElement *)0;
        idx = ndx;
        idx = processElemsUC(elm, str, rtn[0], idx + 1, (char)ch);
        self(npxElement).addElem(npxElement, &str[rtn[1]], elm, (char)ch);
        str[ndx] = L'\0';
    }
    return idx;
}
/**
 * Deserialize the passed TSF string into a hierarchy of NpxElement objects representing the lexical units
 * of the TSF string.
 * @param str the TSF string to be deserialized
 * @param lst the number of character in the input string (also the last character subscript)
 * @return the NpxElement document object
*/
NpxElement *
deserialize(String *str, int lst)
{
    NpxElement *docElement;
    int ndx, idx, rtn[2];
#ifdef TAMBUILDTIME
    us_TAMBuild = 0;
#endif
    tamClockDcl(tam)
    if (0 == lst)
      lst = (int)strlen(str);
    ndx = fieldLength(str, 0, rtn);
    if (*ATTRS != str[ndx]) /* no count of document level lexical units present - 1 is implied */
    {
        rtn[0] = 1;
        idx = 0;
    }
    else
    {
        idx = ndx + 1;
    }
    tamStart(tam)
    docElement = newNpxElement(str, rtn[0], 0);
    tamStop(tam)
    idx = processElems(docElement, str, rtn[0], idx, str[ndx]);
    if (lst != idx)
        fprintf(stderr, "TSF string not completely processed %d vs %d\n", lst, idx);
    return docElement;
}
/**
 * Deserialize the passed TSF string into a hierarchy of NpxElement objects representing the lexical units
 * of the TSF string.
 * @param str the TSF string to be deserialized
 * @return the NpxElement document object
*/
NpxUCElement *
deserializeUC(char *buf, int siz)
{
    NpxUCElement *docElement;
    UCStringBuffer *ucsb;
    Char *wbuf;
    int ndx, idx, rtn[2], len;
#ifdef TAMBUILDTIME
    us_TAMBuild = 0;
#endif
    tamClockDcl(tam)
    if (0 == siz)
        siz = (int)strlen(buf);
    wbuf = mAlloc(sizeof(Char) * (siz + 1), "deserialWbuf");
    len = utf82UCS(wbuf, (const unsigned char *)buf, (int)siz);
    if (0 > len)
    {
        return 0;
    }
    wbuf[len] = L'\0';
    //rprintf("%s\n", buf);
    ndx = fieldLengthUC(wbuf, 0, rtn);
    if (*ATTRS != wbuf[ndx]) /* no count of document level lexical units present - 1 is implied */
    {
        rtn[0] = 1;
        idx = 0;
    }
    else
    {
        idx = ndx + 1;
    }
    tamStart(tam)
    docElement = newNpxUCElement(wbuf, rtn[0], 0);
    tamStop(tam)
    idx = processElemsUC(docElement, wbuf, rtn[0], idx, (char)wbuf[ndx]);
    if (len != idx)
        fprintf(stderr, "TSF string not completely processed %d vs %d\n", len, idx);
    return docElement;
}
static void contentCb(NpxElement *s, int ix, char *name, void *obj, char typ, void *prm)
{
    StringBuffer *sb = (StringBuffer *)prm;
    NpxElement *elm;
    int ect;
    char buf[12];
    if (0 != (ctype[typ] & SL))
    {
        elm = (NpxElement *)obj;
        ect = self(elm).getElementCount(elm);
        sprintf(buf, "%d", ect);
        stringBufferAppend(sb, buf, 0);
        stringBufferAppend(sb, name, 0);
        stringBufferAppend(sb, &typ, 1);
        if (0 < ect)
        {
            self(elm).forEachContent(elm, prm, contentCb);
        }
    }
    else if (0 != (ctype[typ] & PL))
    {
        sprintf(buf, "%d", (int)strlen((char *)obj));
        stringBufferAppend(sb, buf, 0);
        stringBufferAppend(sb, name, 0);
        stringBufferAppend(sb, &typ, 0);
    }
    else
    {
        /* invalid NpxElement */
        fprintf(stderr, "invalid NpxElement name %s\n", name);
    }
}
int charLength(char *buf, int val, int len)
{
    static int tens[10] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
    int ix, ln, quo, mx = 10;
    if (0 == val)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }
    ix = 0;
    while (tens[ix] > val)
    {
        ++ix;
    }
    ln = mx - ix;
    for ( ; ix < mx; ++ix)
    {
        quo = val / tens[ix];
        buf[ix++] = charValue(quo);
        val -= 10 * quo;
    }
    buf[ix] = '\0';
    return ln;
}
clock_t getTAMTime()
{
#ifdef TAMBUILDTIME
    return us_TAMBuild;
#else
    return 0;
#endif
}
char *
serialize(NpxElement *doc)
{
    StringBuffer *sb;
    char buf[12];
    char *ser;
    int dct;
    sb = newStringBuffer((StringBuffer *)0, 0);
    if (1 < (dct = self(doc).getElementCount(doc)))
    {
        sprintf(buf, "%d", dct - 1);
        stringBufferAppend(sb, buf, 0);
        stringBufferAppend(sb, ELEMENT, 1);
    }
    self(doc).forEachContent(doc, sb, contentCb);
    *stringBufferPostInc(sb) = '\0';
    ser = stringBufferToString(sb, 0);
    delStringBuffer(sb);
    return ser;
}
