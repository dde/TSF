package edu.pace.csis.evans.npx;

public class NpxElement
{
  private static final boolean useEmpty = true;
  private String[] elemNames;
  private Object[] elemValues;
  private int siz;
  private int asiz;
  private String[] attrNames;
  private String[] attrVals;
NpxElement()
{
  asiz = siz = -1;
  elemNames = new String[2];
  elemValues = new Object[2];
}
NpxElement(int siz)
{
  this.siz = this.asiz = -1;
  elemNames = new String[siz];
  elemValues = new Object[siz];
}
NpxElement(int esiz, int asiz)
{
  this.siz = this.asiz = -1;
  elemNames = new String[esiz];
  elemValues = new Object[esiz];
  attrNames = new String[asiz];
  attrVals = new String[asiz];
}
NpxElement(String nm, Object obj)
{
  siz = asiz = 0;
  elemNames = new String[1];
  elemValues = new Object[1];
  elemNames[0] = nm;
  elemValues[0] = obj;
}
private void
newAlloc()
{
  String[] tst;
  Object[] tob;
  int ln = siz + siz;
  System.arraycopy(elemNames,  0, tst = new String[ln], 0, siz);
  System.arraycopy(elemValues, 0, tob = new Object[ln], 0, siz);
  elemNames = tst;
  elemValues = tob;
}
private void
newAAlloc()
{
  String[] tst;
  String[] tob;
  int ln = asiz + asiz;
  System.arraycopy(attrNames,  0, tst = new String[ln], 0, asiz);
  System.arraycopy(attrVals, 0, tob = new String[ln], 0, asiz);
  attrNames = tst;
  attrVals = tob;
}
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
  if (++asiz >= attrNames.length)
    newAAlloc();
  attrNames[asiz] = str.substring(nfm, nto);
  attrVals[asiz] = str.substring(vfm, vto);
}
void addAttr(char[] str, int nfm, int nto, int vfm, int vto)
{
  if (++asiz >= attrNames.length)
    newAAlloc();
  attrNames[asiz] = new String(str, nfm, nto - nfm);
  attrVals[asiz] = new String(str, vfm, vto - vfm);
}
void addElem(String str, int fm, int to, NpxElement elm)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = str.substring(fm, to);
  elemValues[siz] = elm;
}
void addElem(char[] str, int fm, int to, NpxElement elm)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = new String(str, fm, to - fm);
  elemValues[siz] = elm;
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
void add(String str, char ch, int fm, int to)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = "" + ch;
  elemValues[siz] = str.substring(fm, to);
}
void add(char[] str, char ch, int fm, int to)
{
  if (++siz >= elemNames.length)
    newAlloc();
  elemNames[siz] = "" + ch;
  elemValues[siz] = new String(str, fm, to - fm);
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
  return asiz + 1;
}
String getAttrName(int ix)
{
  return attrNames[ix];
}
String getAttrVal(int ix)
{
  return attrVals[ix];
}
String getElemName(int ix)
{
  return elemNames[ix];
}
Object getElemVal(int ix)
{
  return elemValues[ix];
}
private void
toString(StringBuilder sb)
{
  int ix, ax, ac;
  Object obj;
  NpxElement elm;
  for (ix = 0; ix <= siz; ++ix)
  {
    obj = elemValues[ix];
    if (0 == elemNames[ix].length())
    {
      throw new IllegalArgumentException(String.format("empty element name at element %d", ix));
    }
    switch (elemNames[ix].charAt(0))
    {
    case '[':
      sb.append((String)obj);
      break;
    case ']':
      sb.append("<![CDATA[");
      sb.append((String)obj);
      sb.append("]]>");
      break;
    case '!':
      sb.append("<!DOCTYPE ");
      sb.append((String)obj);
      sb.append(">");
      break;
    case '-':
      sb.append("<!--");
      sb.append((String)obj);
      sb.append("-->");
      break;
    case '?':
      sb.append("<?");
      sb.append((String)obj);
      sb.append("?>");
      break;
    default:
      if (null == obj || !(obj instanceof NpxElement))
      {
        throw new IllegalArgumentException("null or incorrect NpxElement instance");
      }
      elm = (NpxElement)obj;
      sb.append('<');
      sb.append(elemNames[ix]);
      if (0 != (ac = elm.getAttrCount()))
      {
        for (ax = 0; ax < ac; ++ax)
        {
          sb.append(' ');
          sb.append(elm.attrNames[ax]);
          sb.append("=\"");
          sb.append(elm.attrVals[ax]);
          sb.append('"');
        }
      }
      if (0 != elm.getCount())
      {
        sb.append('>');
        elm.toString(sb);
        sb.append('<');
        sb.append('/');
        sb.append(elemNames[ix]);
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
      }
      sb.append('>');
      break;
    }
  }
}
public String
toString()
{
  StringBuilder sb = new StringBuilder();
  toString(sb);
  return sb.toString();
}
public String
xml2TFX()
{
  StringBuilder sb = new StringBuilder();
  toString(sb);
  return sb.toString();
}
}
