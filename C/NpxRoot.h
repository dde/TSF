//#include "Sstring.h"
static const unsigned char ST = 0x01;
static const unsigned char SL = 0x02;
static const unsigned char PL = 0x04;
static const unsigned char XMLE = 0x08;
static const char ELEMENT[2] = "<";
static const char CDATA[2] = "]";
static const char PI[2] = "?";
static const char COMMENT[2] = "+";
static const char TEXT[2] = "[";
static const char ATTRS[2] = "=";
static const char ATTR[2] = "[";  // TEXT
static const char DOCTYPE[2] = "!";
static const char ARRAY[2] = "@";
static const char ENAME[2] = "\"";
static const char BLANK[2] = " ";
static const char ESCAPE[2] = "\\";
static const char START_TAG[2] = "<";  // ELEMENT
static const char END_TAG[2] = ">";
static const char AMP[2] = "&";
static const char QUOT[2] = "\"";
static const char APOS[2] = "'";

static unsigned char ctype[128] = {
      /* 0      1      2      3      4      5      6      7      8      9      a      b      c      d      e      f   */
      /* NUL    SOH    STX    ETX    EOT    ENQ    ACK    BEL    BS     TAB    LF     VT     FF     CR     SO     SI  */
/* 0 */  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      /* DLE    DC1    DC2    DC3    DC4    NAK    SYN    ETB    CAN    EM     SUB    ESC    FS     GS     RS     US  */
/* 1 */  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      /* SPC    !      "      #      $      %      &      '      (      )      *      +      ,      -      .      /   */
/* 2 */  0,     ST|PL, 0,     0,     0,     0,     0,     0,     0,     0,     0,     ST|PL, 0,     0,     0,     0,
      /* 0      1      2      3      4      5      6      7      8      9      :      ;      <      =      >      ?   */
/* 3 */  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     ST|SL, ST|SL, 0,     ST|PL,
      /* @      A      B      C      D      E      F      G      H      I      J      K      L      M      N      O   */
/* 4 */  ST|SL, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      /* P      Q      R      S      T      U      V      W      X      Y      Z      [      \      ]      ^      _   */
/* 5 */  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     ST|PL, 0,     ST|PL, 0,     0,
      /* `      a      b      c      d      e      f      g      h      i      j      k      l      m      n      o   */
/* 6 */  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      /* p      q      r      s      t      u      v      w      x      y      z      {      |      }      ~      DEL */
/* 7 */  0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
};
/*
static int utf8Length(String str)
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
*/
static int
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

