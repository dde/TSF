package edu.pace.csis.evans.npx;

public class NpxRoot
{
protected static final String prop = "http://apache.org/xml/features/validation/schema";
protected static final char ELEMENT = '<';
protected static final char CDATA = ']';
protected static final char PI = '?';
protected static final char COMMENT = '-';
protected static final char TEXT = '[';
protected static final char ATTRS = '>';
protected static final char ATTR = '=';
protected static final char DOCTYPE = '!';
protected static final char BLANK = ' ';
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
}
