#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "NpxElement.h"
#include "StringBuffer.h"

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

static const char ELEMENT[2] = "<";
static const char CDATA[2]   = "]";
static const char PI[2]      = "?";
static const char COMMENT[2] = "-";
static const char TEXT[2]    = "[";
static const char ATTRS[2]   = ">";
static const char ATTR[2]    = "=";
static const char DOCTYPE[2] = "!";
static const char *const etypes[] = {TEXT, CDATA, COMMENT, PI, DOCTYPE};
/*NpxDataType dt = new NpxDataType();*/

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
 * @return the index of the first non-digit character encountered
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
/**
 * Extract each of n attribute name/value pairs from the next position in the TFX string.
 * @param npxElement the NpxElement object being built into which the attributes are stored
 * @param str the TFX string
 * @param ct the number of attributes to extract
 * @param idx the beginning index of the attribute list in the TFX string
 * @return the index of the character after the extracted attributes
 */
static int
processAttrList(NpxElement *npxElement, String *str, const int ct, int idx)
{
    int ndx, ix;
    tamClockDcl(tam)
    int rtn[2];
    for (ix = 0; ix < ct; ++ix)
    {
        ndx = fieldLength(str, idx, rtn);
        str[idx] = '\0';
        while (0 != str[ndx] && *ATTR != str[ndx])
        {
            ++ndx;
        }
        idx = rtn[0] + ndx + 1;
        str[ndx] = '\0';
        tamStart(tam)
        self(npxElement).addAttr(npxElement, &str[rtn[1]], &str[ndx + 1]);
        tamStop(tam)
    }
    str[idx] = '\0';
    return idx;
}
/**
 * Extract each of n content elements from the next position in the TFX string.
 * @param npxElement the NpxElement object being built from the content
 * @param str the TFX string
 * @param ct the number of content elements to extract
 * @param ndx the beginning index of the content in the TFX string
 * @return the index of the character after the extracted content
*/
static int
processElems(NpxElement *npxElement, String *str, const int ct, int ndx)
{
    int idx, ix, jx;
    int rtn[2], atn[2];
    char ch;
    NpxElement *elm;
    tamClockDcl(tam)
    idx = ndx;
    for (ix = 0; ix < ct; ++ix)
    {
        ndx = fieldLength(str, idx, rtn);
        str[idx] = '\0';
        idx = rtn[0] + ndx + 1;
        ch = str[ndx];
        for (jx = 0; jx < arraySize(etypes); ++jx)
        {
            if (ch == *etypes[jx])
            {
                tamStart(tam)
                self(npxElement).add(npxElement, etypes[jx], ndx + 1, idx);
                tamStop(tam)
                goto l1; // effectively, continue outer
            }
        }
        while ((ch = str[ndx]) != *ATTRS && ch != *ELEMENT)
        {
            ++ndx;
        }
        if (*ATTRS == ch)
        {
            idx = fieldLength(str, ndx + 1, atn);
            tamStart(tam)
            elm = newNpxElement(str, rtn[0], atn[0]);
            tamStop(tam)
            idx = processAttrList(elm, str, atn[0], idx + 1);
        }
        else
        {
            tamStart(tam)
            elm = newNpxElement(str, rtn[0], 0);
            tamStop(tam)
            idx = ndx;
        }
        idx = processElems(elm, str, rtn[0], idx + 1);
        str[ndx] = '\0';
        tamStart(tam)
        self(npxElement).addElem(npxElement, &str[rtn[1]], elm);
        tamStop(tam)
    l1:;
    }
    return idx;
}
/**
 * Deserialize the passed TFX string into a hierarchy of NpxElement objects representing the XML elements
 * of the TFX string.
 * @param str the TFX string to be deserialized
 * @return the NpxElement document object
*/
NpxElement *
deserialize(String *str)
{
    NpxElement *docElement;
    int lst, ndx, idx, rtn[2];
#ifdef TAMBUILDTIME
    us_TAMBuild = 0;
#endif
    tamClockDcl(tam)
    lst = (int)strlen(str);
    ndx = fieldLength(str, 0, rtn);
    if (*ATTRS != str[ndx]) /* no count of document level lexical units present - 1 is implied */
    {
        rtn[0] = 1;
        ndx = 0;
    }
    else
    {
        ++rtn[0];
        ++ndx;
    }
    tamStart(tam)
    docElement = newNpxElement(str, rtn[0], 0);
    tamStop(tam)
    idx = processElems(docElement, str, rtn[0], ndx);
    if (lst != idx)
        fprintf(stderr, "TFX string not completely processed %d vs %d\n", lst, idx);
    return docElement;
}
static void attributeCb(NpxElement *s, int ix, char *name, char *val, void *prm)
{
    char buf[12];
    StringBuffer *sb = (StringBuffer *)prm;
    sprintf(buf, "%d", (int)strlen(val));
    stringBufferAppend(sb, buf, 0);
    stringBufferAppend(sb, name, 0);
    stringBufferAppend(sb, ATTR, 1);
    stringBufferAppend(sb, val, 0);
}
static void contentCb(NpxElement *s, int ix, char *name, void *obj, void *prm)
{
    StringBuffer *sb = (StringBuffer *)prm;
    NpxElement *elm;
    int ect;
    char buf[12];
    if (self(s).isElement(s, name))
    {
        elm = (NpxElement *)obj;
        ect = self(s).getElementCount(elm);
        sprintf(buf, "%d", ect);
        stringBufferAppend(sb, buf, 0);
        stringBufferAppend(sb, name, 0);
        if (self(elm).hasAttributes(elm))
        {
            stringBufferAppend(sb, ATTRS, 1);
            sprintf(buf, "%d", self(elm).getAttributeCount(elm));
            stringBufferAppend(sb, buf, 0);
            stringBufferAppend(sb, ATTR, 1);
            self(elm).forEachAttribute(elm, prm, attributeCb);
        }
        stringBufferAppend(sb, ELEMENT, 1);
        if (0 < ect)
        {
            self(elm).forEachContent(elm, prm, contentCb);
        }
    }
    else if (self(s).isText(s, name) ||
        self(s).isCdata(s, name) ||
        self(s).isComment(s, name) ||
        self(s).isProcInst(s, name) ||
        self(s).isDoctype(s, name))
    {
        sprintf(buf, "%d", (int)strlen((char *)obj));
        stringBufferAppend(sb, buf, 0);
        stringBufferAppend(sb, name, 0);
        stringBufferAppend(sb, (char *)obj, 0);
    }
    else
    {
        /* invalid NpxElement */
        fprintf(stderr, "invalid NpxElement name %s\n", name);
    }
}
int charLength(char *buf, int val, int len)
{
    static int tens[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
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
    if (1 < (dct = doc->mtb->getElementCount(doc)))
    {
        sprintf(buf, "%d", dct - 1);
        stringBufferAppend(sb, buf, 0);
        stringBufferAppend(sb, ELEMENT, 1);
    }
    doc->mtb->forEachContent(doc, sb, contentCb);
    *stringBufferPostInc(sb) = '\0';
    ser = stringBufferToString(sb, 0);
    delStringBuffer(sb);
    return ser;
}
