package edu.pace.csis.evans.npx;

public class NpxRoot
{
protected static final byte STOP = 0x01;
protected static final byte SLU = 0x02;
protected static final byte PLU = 0x04;
protected static final byte XMLE = 0x08;
protected static final byte ctype[] = new byte[128];
protected static final char ELEMENT = '<';
protected static final char CDATA = ']';
protected static final char PI = '?';
protected static final char COMMENT = '+';
protected static final char TEXT = '[';
protected static final char ATTRS = '=';
protected static final char ATTR = TEXT;
protected static final char DOCTYPE = '!';
protected static final char ARRAY = '@';
protected static final char ENAME = '"';
protected static final char BLANK = ' ';
protected static final char ESCAPE = '\\';
protected static final char START_TAG = ELEMENT;
protected static final char END_TAG = '>';
protected static final char AMP = '&';
protected static final char QUOT = '"';
protected static final char APOS = '\'';

static {
  ctype[ELEMENT] = STOP | SLU;
  ctype[ATTRS]   = STOP | SLU;
  ctype[ARRAY]   = STOP | SLU;
  ctype[CDATA]   = STOP | PLU;
  ctype[PI]      = STOP | PLU;
  ctype[COMMENT] = STOP | PLU;
  ctype[TEXT]    = STOP | PLU;
  ctype[DOCTYPE] = STOP | PLU;
  ctype[START_TAG] |= XMLE;
  ctype[END_TAG]   |= XMLE;
  ctype[AMP]       |= XMLE;
  ctype[QUOT]      |= XMLE;
  ctype[APOS]      |= XMLE;
}
int utf8Length(String str)
{
  int ix, ln, utf8len;
  char ch;
  ln = str.length();
  utf8len = 0;
  for (ix = 0; ix < ln; ++ix)
  {
    ch = str.charAt(ix);
    if (ch < 128)
    {
      utf8len += 1;
    }
    else if (ch < 2048)  //2^11
    {
      utf8len += 2;
    }
    else if (ch < 0xd800 || ch >= 0xe000)
    {
      utf8len += 3;
    }
    else
    {
      ix += 1;
      utf8len += 4;
    }
  }
  return utf8len;
}
int
numLength(int n)
{
  int ln;
  if (n < 100)
    ln = (n < 10) ? 1 : 2;
  else if (n < 10000)
    ln = (n < 1000) ? 3 : 4;
  else if (n < 1000000)
    ln = (n < 100000) ? 5 : 6;
  else if (n < 100000000)
    ln = (n < 10000000) ? 7 : 8;
  else
    ln = (n < 1000000000) ? 9 : 10;
  return ln;
}
/**
 * Convert the passed digit character to its integer value.
 * @param ch a digit character
 * @return the passed character's integer value, 0 for non-digits
 */
protected int numericValue(char ch)
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
}
