//
// Created by Dan Evans on 9/25/19.
//
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <wchar.h>

#include "Sstring.h"
#include "StringBuffer.h"
#include "UCStringBuffer.h"
#include "NpxElement.h"
#include "NpxUCElement.h"

#define self(s) (*s->mtb)
#define arraySize(a) ((sizeof a)/(sizeof a[0]))

static const unsigned char XMLE = 0x01;
static unsigned char ctype[128] = {
        /* 0      1      2      3      4      5      6      7      8      9      a      b      c      d      e      f   */
        /* NUL    SOH    STX    ETX    EOT    ENQ    ACK    BEL    BS     TAB    LF     VT     FF     CR     SO     SI  */
/* 0 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        /* DLE    DC1    DC2    DC3    DC4    NAK    SYN    ETB    CAN    EM     SUB    ESC    FS     GS     RS     US  */
/* 1 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        /* SPC    !      "      #      $      %      &      '      (      )      *      +      ,      -      .      /   */
/* 2 */    0,     0,     XMLE,  0,     0,     0,     XMLE,  XMLE,  0,     0,     0,     0,     0,     0,     0,     0,
        /* 0      1      2      3      4      5      6      7      8      9      :      ;      <      =      >      ?   */
/* 3 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     XMLE,  0,     XMLE,  0,
        /* @      A      B      C      D      E      F      G      H      I      J      K      L      M      N      O   */
/* 4 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        /* P      Q      R      S      T      U      V      W      X      Y      Z      [      \      ]      ^      _   */
/* 5 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        /* `      a      b      c      d      e      f      g      h      i      j      k      l      m      n      o   */
/* 6 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
        /* p      q      r      s      t      u      v      w      x      y      z      {      |      }      ~      DEL */
/* 7 */    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
};
static const char ELEMENT[2] = "<";
static const char CDATA[2]   = "]";
static const char PI[2]      = "?";
static const char COMMENT[2] = "-";
static const char TEXT[2]    = "[";
static const char ENDTAG[2]  = ">";
static const char ATTRS[2]   = "=";
static const char DOCTYPE[2] = "!";
static const char START_TAG = '<';
static const char END_TAG   = '>';
static const char AMP       = '&';
static const char QUOT      = '"';
static const char APOS      = '\'';

/*static const char *etypes[]  = {TEXT, CDATA, COMMENT, PI, DOCTYPE};*/
typedef struct XmlOpts
{
    StringBuffer *sb;
    UCStringBuffer *ucsb;
    int useEmpty;
} XmlOpts;


static int hasAttributes(NpxElement *s)
{
    if (1 <= self(s).getElementCount(s) && ATTRS[0] == self(s).getElementType(s, 0))
    {
        return 1;
    }
    return 0;
}
static int hasAttributesUC(NpxUCElement *s)
{
    if (1 <= self(s).getElementCount(s) && ATTRS[0] == self(s).getElementType(s, 0))
    {
        return 1;
    }
    return 0;
}
static int hasOnlyAttributes(NpxElement *s)
{
    if (1 == self(s).getElementCount(s) && ATTRS[0] == self(s).getElementType(s, 0))
    {
        return 1;
    }
    return 0;
}
static int hasOnlyAttributesUC(NpxUCElement *s)
{
    if (1 == self(s).getElementCount(s) && ATTRS[0] == self(s).getElementType(s, 0))
    {
        return 1;
    }
    return 0;
}
static void
xmlEscape(char *str, StringBuffer *sb)
{
    int ix;
    unsigned char chr;
    char numbuf[12];
    for (ix = 0; 0 != str[ix]; ++ix)
    {
        chr = str[ix];
        if (chr >= arraySize(ctype))
            continue;
        if (0 != (ctype[chr] & XMLE))
            goto outer;
    }
    stringBufferAppend(sb, str, 0);
    return;
   outer:
    if (ix > 0)
      stringBufferAppend(sb, str, ix);
    for (; 0 != str[ix]; ++ix)
    {
        chr = str[ix];
        if (chr >= arraySize(ctype) || 0 == (ctype[chr] & XMLE))
        {
            stringBufferAppend(sb, (char *)&chr, 1);
            continue;
        }
        switch (chr)
        {
            case START_TAG:
                stringBufferAppend(sb, "&lt;", 0);
                break;
            case END_TAG:
                stringBufferAppend(sb, "&gt;", 0);
                break;
            case AMP:
                stringBufferAppend(sb, "&amp;", 0);
                break;
            case QUOT:
                stringBufferAppend(sb, "&quot;", 0);
                break;
            case APOS:
                stringBufferAppend(sb, "&apos;", 0);
                break;
            default:
                stringBufferAppend(sb, "&#", 0);
                sprintf(numbuf, "%d", (int)chr);
                stringBufferAppend(sb, numbuf, 0);
                stringBufferAppend(sb, ";", 0);
                break;
        }
    }
}
static void
xmlEscapeUC(Char *str, UCStringBuffer *sb)
{
    int ix;
    wchar_t chr;
    wchar_t numbuf[12];
    for (ix = 0; 0 != str[ix]; ++ix)
    {
        chr = str[ix];
        if (chr >= arraySize(ctype))
            continue;
        if (0 != (ctype[chr] & XMLE))
            goto outer;
    }
    ucstringBufferAppend(sb, str, 0);
    return;
  outer:
    if (ix > 0)
        ucstringBufferAppend(sb, str, ix);
    for (; 0 != str[ix]; ++ix)
    {
        chr = str[ix];
        if (chr >= arraySize(ctype) || 0 == (ctype[chr] & XMLE))
        {
            ucstringBufferAppend(sb, (Char *)&chr, 1);
            continue;
        }
        switch (chr)
        {
            case START_TAG:
                ucstringBufferAppend(sb, L"&lt;", 0);
                break;
            case END_TAG:
                ucstringBufferAppend(sb, L"&gt;", 0);
                break;
            case AMP:
                ucstringBufferAppend(sb, L"&amp;", 0);
                break;
            case QUOT:
                ucstringBufferAppend(sb, L"&quot;", 0);
                break;
            case APOS:
                ucstringBufferAppend(sb, L"&apos;", 0);
                break;
            default:
                ucstringBufferAppend(sb, L"&#", 0);
                swprintf(numbuf, sizeof numbuf, L"%d", (int)chr);
                ucstringBufferAppend(sb, numbuf, 0);
                ucstringBufferAppend(sb, L";", 0);
                break;
        }
    }
}
static void emptyClose(StringBuffer *sb, char *tag, int useEmpty)
{
    if (useEmpty)
    {
        stringBufferAppend(sb, "/", 0);
    }
    else
    {
        stringBufferAppend(sb, "</", 0);
        stringBufferAppend(sb, tag, 0);
    }
    stringBufferAppend(sb, ">", 0);
}
static void attributeCb(NpxElement *s, int ix, char *name, void *val, char typ, void *prm)
{
    StringBuffer *sb = ((XmlOpts *)prm)->sb;
    stringBufferAppend(sb, " ", 0);
    stringBufferAppend(sb, name, 0);
    stringBufferAppend(sb, "=\"", 0);
    xmlEscape((char *)val, sb);
    stringBufferAppend(sb, "\"", 0);
}
static void attributeUCCb(NpxUCElement *s, int ix, Char *name, void *val, char typ, void *prm)
{
    UCStringBuffer *sb = ((XmlOpts *)prm)->ucsb;
    ucstringBufferAppend(sb, L" ", 0);
    ucstringBufferAppend(sb, name, 0);
    ucstringBufferAppend(sb, L"=\"", 0);
    xmlEscapeUC((Char *)val, sb);
    ucstringBufferAppend(sb, L"\"", 0);
}
static void contentCb(NpxElement *s, int ix, char *name, void *obj, char typ, void *prm)
{
    StringBuffer *sb = ((XmlOpts *)prm)->sb;
    int useEmpty = ((XmlOpts *)prm)->useEmpty;
    NpxElement *elm, *par;
    if (typ == ELEMENT[0])
    {
        stringBufferAppend(sb, "<", 0);
        stringBufferAppend(sb, name, 0);
        elm = (NpxElement *) obj;
        if (0 != elm && self(elm).hasContent(elm))
        {
            if (0 == hasAttributes(elm))
            {
                stringBufferAppend(sb, ">", 0);
            }
            self(elm).forEachContent(elm, prm, contentCb);
            if (!useEmpty || !hasOnlyAttributes(elm))
            {
                stringBufferAppend(sb, "</", 0);
                stringBufferAppend(sb, name, 0);
                stringBufferAppend(sb, ">", 0);
            }
        }
        else
        {
            if (useEmpty)
            {
                stringBufferAppend(sb, "/", 0);
            }
            else
            {
                stringBufferAppend(sb, "></", 0);
                stringBufferAppend(sb, name, 0);
            }
            stringBufferAppend(sb, ">", 0);
        }
    }
    else if (typ == ATTRS[0])
    {
        elm = (NpxElement *)obj;
        self(elm).forEachContent(elm, prm, attributeCb);
        if (useEmpty && hasOnlyAttributes(s))
            stringBufferAppend(sb, "/", 0);
        stringBufferAppend(sb, ">", 0);
    }
    else if (typ == TEXT[0])
    {
        //stringBufferAppend(sb, (char *)obj, 0);
        xmlEscape((char *)obj, sb);
    }
    else if (typ == CDATA[0])
    {
        stringBufferAppend(sb, "<![CDATA[", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "]]>", 0);
    }
    else if (typ == COMMENT[0])
    {
        stringBufferAppend(sb, "<!--", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "-->", 0);
    }
    else if (typ == PI[0])
    {
        stringBufferAppend(sb, "<?", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "?>", 0);
    }
    else if (typ == DOCTYPE[0])
    {
        stringBufferAppend(sb, "<!DOCTYPE ", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, ">", 0);
    }
    else
    {
        stringBufferAppend(sb, "<!--UNKNOWN:", 0);
        stringBufferAppend(sb, (char *)obj, 0);
        stringBufferAppend(sb, "-->", 0);
    }
}
static void contentUCCb(NpxUCElement *s, int ix, Char *name, void *obj, char typ, void *prm)
{
    UCStringBuffer *sb = ((XmlOpts *)prm)->ucsb;
    int useEmpty = ((XmlOpts *)prm)->useEmpty;
    NpxUCElement *elm, *par;
    if (typ == ELEMENT[0])
    {
        ucstringBufferAppend(sb, L"<", 0);
        ucstringBufferAppend(sb, name, 0);
        elm = (NpxUCElement *) obj;
        if (0 != elm && self(elm).hasContent(elm))
        {
            if (0 == hasAttributesUC(elm))
            {
                ucstringBufferAppend(sb, L">", 0);
            }
            self(elm).forEachContent(elm, prm, contentUCCb);
            if (!useEmpty || !hasOnlyAttributesUC(elm))
            {
                ucstringBufferAppend(sb, L"</", 0);
                ucstringBufferAppend(sb, name, 0);
                ucstringBufferAppend(sb, L">", 0);
            }
        }
        else
        {
            if (useEmpty)
            {
                ucstringBufferAppend(sb, L"/", 0);
            }
            else
            {
                ucstringBufferAppend(sb, L"></", 0);
                ucstringBufferAppend(sb, name, 0);
            }
            ucstringBufferAppend(sb, L">", 0);
        }
    }
    else if (typ == ATTRS[0])
    {
        elm = (NpxUCElement *)obj;
        self(elm).forEachContent(elm, prm, attributeUCCb);
        if (useEmpty && hasOnlyAttributesUC(s))
            ucstringBufferAppend(sb, L"/", 0);
        ucstringBufferAppend(sb, L">", 0);
    }
    else if (typ == TEXT[0])
    {
        //ucstringBufferAppend(sb, (char *)obj, 0);
        xmlEscapeUC((Char *)obj, sb);
    }
    else if (typ == CDATA[0])
    {
        ucstringBufferAppend(sb, L"<![CDATA[", 0);
        ucstringBufferAppend(sb, (Char *)obj, 0);
        ucstringBufferAppend(sb, L"]]>", 0);
    }
    else if (typ == COMMENT[0])
    {
        ucstringBufferAppend(sb, L"<!--", 0);
        ucstringBufferAppend(sb, (Char *)obj, 0);
        ucstringBufferAppend(sb, L"-->", 0);
    }
    else if (typ == PI[0])
    {
        ucstringBufferAppend(sb, L"<?", 0);
        ucstringBufferAppend(sb, (Char *)obj, 0);
        ucstringBufferAppend(sb, L"?>", 0);
    }
    else if (typ == DOCTYPE[0])
    {
        ucstringBufferAppend(sb, L"<!DOCTYPE ", 0);
        ucstringBufferAppend(sb, (Char *)obj, 0);
        ucstringBufferAppend(sb, L">", 0);
    }
    else
    {
        ucstringBufferAppend(sb, L"<!--UNKNOWN:", 0);
        ucstringBufferAppend(sb, (Char *)obj, 0);
        ucstringBufferAppend(sb, L"-->", 0);
    }
}
/**
 * Convert a TAM dynamic structure to an equivalent XML string set into a passed StringBuffer.
 * @param dsobj the root of the TAM structure
 * @param prm the StringBUffer into which the XML string will be stored
 * @param num_opts the number of options that follow in the variable argument list
 * @param useEmpty (int) if not 0, generate <.../> format empty elements when needed (otherwise <...></...> )
 */
void npx2XML(NpxElement *tam, StringBuffer *sb, int num_opts, ...)
{
    XmlOpts opts;
    char typ;
    int ct;
    va_list args;
    va_start(args, num_opts);
    opts.sb = sb;
    opts.ucsb = 0;
    opts.useEmpty = (num_opts >= 1) ? va_arg(args, int) : 0;
    ct = self(tam).getElementCount(tam);
    typ = self(tam).getElementType(tam, 0);
    if (*ATTRS == typ)
    {
    }
    self(tam).forEachContent(tam, (void *)&opts, contentCb);
    *stringBufferPostInc(sb) = '\0';
    va_end(args);
}
void npx2XMLUC(NpxUCElement *tam, UCStringBuffer *sb, int num_opts, ...)
{
    XmlOpts opts;
    char typ;
    int ct;
    va_list args;
    va_start(args, num_opts);
    opts.ucsb = sb;
    opts.sb = 0;
    opts.useEmpty = (num_opts >= 1) ? va_arg(args, int) : 0;
    ct = self(tam).getElementCount(tam);
    typ = self(tam).getElementType(tam, 0);
    if (*ATTRS == typ)
    {
    }
    self(tam).forEachContent(tam, (void *)&opts, contentUCCb);
    *ucstringBufferPostInc(sb) = L'\0';
    va_end(args);
}