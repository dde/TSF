package edu.pace.csis.evans.npx;

public class NpxElement extends NpxRoot
{
  private static final boolean useEmpty = false;
  private String[] elemNames;
  private Object[] elemValues;
  private char[] elemTypes;
  private NpxElement parent;
  private int siz;
NpxElement(NpxElement par)
{
  siz = -1;
  elemNames = new String[2];
  elemValues = new Object[2];
  elemTypes = new char[2];
  parent = par;
}
NpxElement(int siz, NpxElement par)
{
  this.siz = -1;
  elemNames = new String[siz];
  elemValues = new Object[siz];
  elemTypes = new char[siz];
  parent = par;
}
private void
newAlloc()
{
  String[] tst;
  Object[] tob;
  char[] tty;
  int ln = siz + siz;
  System.arraycopy(elemNames,  0, tst = new String[ln], 0, siz);
  System.arraycopy(elemValues, 0, tob = new Object[ln], 0, siz);
  System.arraycopy(elemTypes, 0, tty = new char[ln], 0, siz);
  elemNames = tst;
  elemValues = tob;
  elemTypes = tty;
}
//private void
//newAAlloc()
//{
//  String[] tst;
//  String[] tob;
//  int ln = asiz + asiz;
//  System.arraycopy(attrNames,  0, tst = new String[ln], 0, asiz);
//  System.arraycopy(attrVals, 0, tob = new String[ln], 0, asiz);
//  attrNames = tst;
//  attrVals = tob;
//}
private int
isNumeric(String str)
{
  return 0;
}
void addInteger(String nm, int val)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = nm;
  elemValues[siz] = new Integer(val);
}
private void addReal(String nm, double val)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = nm;
  elemValues[siz] = new Double(val);
}
private void addString(String nm, String val)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = nm;
  elemValues[siz] = val;
}
private void addObject(String nm, Object val)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = nm;
  elemValues[siz] = val;
}
void addAttr(String str, int nfm, int nto, int vfm, int vto)
{
//  if (++asiz >= attrNames.length)
//    newAAlloc();
//  attrNames[asiz] = str.substring(nfm, nto);
//  attrVals[asiz] = str.substring(vfm, vto);
}
void addAttr(char[] str, int nfm, int nto, int vfm, int vto)
{
//  if (++asiz >= attrNames.length)
//    newAAlloc();
//  attrNames[asiz] = new String(str, nfm, nto - nfm);
//  attrVals[asiz] = new String(str, vfm, vto - vfm);
}
void addElem(String str, int fm, int to, NpxElement elm, char typ)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = str.substring(fm, to);
  elemValues[siz] = elm;
  elemTypes[siz] = typ;
}
void addElem(char[] str, int fm, int to, NpxElement elm, char typ)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = new String(str, fm, to - fm);
  elemValues[siz] = elm;
  elemTypes[siz] = typ;
}
void addElemX(String str, int fm, int to, NpxElement elm)
{
  String stm;
  int ln;
  char ctp;
  if (++siz >= elemNames.length)
    newAlloc();
  stm = str.substring(fm, to + 1);
  ln = stm.length() - 1;
  ctp = stm.charAt(ln);
  if ('>' == ctp)
    ctp = '<';
  elemNames[siz] = ctp + stm.substring(0, ln);
  elemValues[siz] = elm;
}
void addElemX(char[] str, int fm, int to, NpxElement elm)
{
  String stm;
  int ln;
  char ctp;
  if (++siz >= elemNames.length)
    newAlloc();
  stm = new String(str, fm, to - fm + 1);
  ln = stm.length() - 1;
  ctp = stm.charAt(ln);
  if ('>' == ctp)
    ctp = '<';
  elemNames[siz] = ctp + stm.substring(0, ln);
  elemValues[siz] = elm;
}
void add(String str, int nfm, int fm, int to, char typ)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = (nfm == fm) ? null : str.substring(nfm, fm);
  elemValues[siz] = str.substring(fm + 1, to);
  elemTypes[siz] = typ;
}
void add(char[] str, int nfm, int fm, int to, char typ)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = (nfm == fm) ? null : new String(str, nfm, fm - nfm);
  elemValues[siz] = new String(str, fm + 1, to - fm - 1);
  elemTypes[siz] = typ;
}
void add(String nm, String val)
{
  switch (isNumeric(val))
  {
  case 0:
    addReal(nm, Integer.parseInt(val));
    break;
  case 1:
    addReal(nm, Double.parseDouble(val));
    break;
  default:
    addString(nm, val);
    break;
  }
}
void add(char ch, String val)
{
  addString("" + ch, val);
}
void add(String nm, NpxElement elm)
{
  if (0 == nm.length())
  {
    System.out.println("0-length tag name");
  }
  addObject(nm, elm);
}
int getCount()
{
  return siz + 1;
}
int getAttrCount()
{
  return siz + 1;
}
String getAttrName(int ix)
{
  return elemNames[ix];
}
String getAttrVal(int ix)
{
  return (String)elemValues[ix];
}
String getElemName(int ix)
{
  return elemNames[ix];
}
Object getElemVal(int ix)
{
  return elemValues[ix];
}
char getElemType(int ix)
{
  return elemTypes[ix];
}
private void
toTSFString(StringBuilder sb)
{
  int ix, ax, ac;
  String val;
  NpxElement elm;
  char typ;
  for (ix = 0; ix <= siz; ++ix)
  {
    typ = elemTypes[ix];
    if (0 == typ)
    {
      sb.append("{undef}");
      continue;
    }
    if (0 != (ctype[typ] & PLU))
    {
      val = (String)elemValues[ix];
      sb.append("" + val.length());
      if (null != elemNames[ix] && 0 != elemNames[ix].length())
      {
        sb.append(elemNames[ix]);
      }
      sb.append(typ);
      sb.append(val);
    }
    else
    {
      elm = (NpxElement)elemValues[ix];
      if (null != elm)
        sb.append("" + elm.getCount());
      else
        sb.append('0');
      if (null != elemNames[ix] && 0 != elemNames[ix].length())
      {
        sb.append(elemNames[ix]);
      }
      sb.append(typ);
      if (null != elm)
        elm.toTSFString(sb);
    }
  }
}
private String
toTSFString()
{
  StringBuilder sb = new StringBuilder();
  if (0 != siz)
  {
    sb.append("" + (siz + 1));
    sb.append(ATTRS);
  }
  toTSFString(sb);
  return sb.toString();
}
private void
toBasicString(StringBuilder sb)
{
  int ix, ax, ac;
  Object obj;
  NpxElement elm;
  char typ;
  sb.append('{');
  for (ix = 0; ix <= siz; ++ix)
  {
    if (0 != ix)
      sb.append(',');
    typ = elemTypes[ix];
    if (0 == typ)
    {
      sb.append("{undef}");
      continue;
    }
    if (0 != (ctype[typ] & PLU))
    {
      if (null != elemNames[ix] && 0 != elemNames[ix].length())
      {
        sb.append(elemNames[ix]);
      }
      sb.append(typ);
      sb.append((String)elemValues[ix]);
    }
    else
    {
      if (null != elemNames[ix] && 0 != elemNames[ix].length())
      {
        sb.append(elemNames[ix]);
      }
      sb.append(typ);
      if (null != elemValues[ix])
        sb.append(((NpxElement)elemValues[ix]).toBasicString());
      else
        sb.append("{}");
    }
  }
  sb.append('}');
}
private String
toBasicString()
{
  StringBuilder sb = new StringBuilder();
  toBasicString(sb);
  return sb.toString();
}
void
xmlEscape(String str, StringBuilder sb)
{
  int ix, end;
  char chr;
  end = str.length();
 outer:
  {
    for (ix = 0; ix < end; ++ix)
    {
      chr = str.charAt(ix);
      if (chr >= ctype.length)
        continue;
      if (0 != (ctype[chr] & XMLE))
        break outer;
    }
    sb.append(str);
    return;
  }
  sb.append(str.substring(0, ix));
  for (; ix < end; ++ix)
  {
    chr = str.charAt(ix);
    if (chr >= ctype.length || 0 == (ctype[chr] & XMLE))
    {
      sb.append(chr);
      continue;
    }
    switch (chr)
    {
    case START_TAG:
      sb.append("&lt;");
      break;
    case END_TAG:
      sb.append("&gt;");
      break;
    case AMP:
      sb.append("&amp;");
      break;
    case QUOT:
      sb.append("&quot;");
      break;
    case APOS:
      sb.append("&apos;");
      break;
    default:
      sb.append("&#" + (int)chr + ";");
      break;
    }
  }
}
private boolean
hasXMLAttrs()
{
  return ATTRS == elemTypes[0];
}
private boolean
isEmptyElement()
{
  return 0 == siz && ATTRS == elemTypes[0];
}
private void
toXMLString(StringBuilder sb)
{
  int ix, jx;
  Object obj;
  NpxElement elm;
  char ctyp;
  for (ix = 0; ix <= siz; ++ix)
  {
    ctyp = elemTypes[ix];
    if (0 == ctyp)
    {
      sb.append("<undef/>");
      continue;
    }
    if (0 == ix && ctyp != ATTRS)
      sb.append('>');
    obj = elemValues[ix];
    switch (ctyp)
    {
    case ELEMENT:
      sb.append('<');
      sb.append(elemNames[ix]);
      if (null != obj)
      {
        elm = (NpxElement)obj;
        if (!elm.hasXMLAttrs())
          sb.append('>');
        elm.toXMLString(sb);
        if (!elm.isEmptyElement())
        {
          sb.append("</");
          sb.append(elemNames[ix]);
          sb.append('>');
        }
      }
      else
      {
        if (useEmpty)
        {
          sb.append('/');
        }
        else
        {
          sb.append("></");
          sb.append(elemNames[ix]);
        }
        sb.append('>');
      }
      break;
    case ATTRS:
      elm = (NpxElement)obj;
      for (jx = 0; jx < elm.getCount(); ++jx)
      {
        sb.append(' ');
        sb.append(elm.elemNames[jx]);
        sb.append("=\"");
        xmlEscape((String)elm.elemValues[jx], sb);
        sb.append("\"");
      }
      if (0 == siz)
      {
        if (useEmpty)
        {
          sb.append('/');
        }
        else
        {
          sb.append("></");
          sb.append(elemNames[0]);
        }
      }
      sb.append('>');
      break;
    case TEXT:
      xmlEscape((String)obj, sb);
      break;
    case CDATA:
      sb.append("<![CDATA[");
      sb.append((String)obj);
      sb.append("]]>");
      break;
    case COMMENT:
      sb.append("<!--");
      sb.append((String)obj);
      sb.append("-->");
      break;
    case PI:
      sb.append("<?");
      sb.append((String)obj);
      sb.append("?>");
      break;
    default:
      throw new IllegalArgumentException(String.format("unexpected type value %d", (int)ctyp));
    }
  }
}
private String
toXMLString()
{
  int ix;
  char ctyp;
  StringBuilder sb = new StringBuilder();
  NpxElement elm;
  if (0 == elemTypes[0])
    return "";
  for (ix = 0; ix <= siz; ++ix)
  {
    ctyp = elemTypes[ix];
    switch (ctyp)
    {
    case ELEMENT:
      sb.append('<');
      sb.append(elemNames[ix]);
      elm = (NpxElement)elemValues[ix];
      if (null != elm)
      {
        elm.toXMLString(sb);
        if (!elm.isEmptyElement())
        {
          sb.append("</");
          sb.append(elemNames[ix]);
          sb.append('>');
        }
      }
      else
      {
        if (useEmpty)
        {
          sb.append('/');
        }
        else
        {
          sb.append("></");
          sb.append(elemNames[ix]);
        }
        sb.append('>');
      }
      break;
    case DOCTYPE:
      sb.append("<!DOCTYPE ");
      sb.append((String)elemValues[ix]);
      sb.append(">");
      break;
    case COMMENT:
      sb.append("<!--");
      sb.append((String)elemValues[ix]);
      sb.append("-->");
      break;
    case PI:
      sb.append("<?");
      sb.append((String)elemValues[ix]);
      sb.append("?>");
      break;
    default:
      sb.append("&#" + (int)ctyp + ";");
      break;
    }
  }
  return sb.toString();
}
public String
toString()
{
  //return new NpxTFX2XML().toString(this);
  return toTSFString();
  //return toBasicString();
}
}
