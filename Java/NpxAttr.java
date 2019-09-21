package edu.pace.csis.evans.npx;

public class NpxAttr extends XMLData
{
String[] attrNames = new String[2];
Object[] attrValues = new Object[2];
int siz = -1;
private void
newAlloc()
{
  String[] tst;
  Object[] tob;
  int ln = siz + siz;
  System.arraycopy(attrNames,  0, tst = new String[ln], 0, siz);
  System.arraycopy(attrValues, 0, tob = new Object[ln], 0, siz);
  attrNames = tst;
  attrValues = tob;
}
void alloc(int n)
{
  attrNames = new String[n];
  attrValues = new Object[n];
  siz = -1;
}
int
isNumeric(String str)
{
  return 2;
}
void addInteger(String nm, int val)
{
  if (++siz >= attrNames.length)
    newAlloc();
  attrNames[siz] = nm;
  attrValues[siz] = new Integer(val);
}
void addReal(String nm, double val)
{
  if (++siz >= attrNames.length)
    newAlloc();
  attrNames[siz] = nm;
  attrValues[siz] = new Double(val);
}
void addString(String nm, String val)
{
  if (++siz >= attrNames.length)
    newAlloc();
  attrNames[siz] = nm;
  attrValues[siz] = val;
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
int getCount()
{
  return siz + 1;
}
String
toString(StringBuilder sb)
{
  int ix;
  Object obj;
  for (ix = 0; ix <= siz; ++ix)
  {
    sb.append(' ');
    sb.append(attrNames[ix]);
    sb.append('=');
    sb.append('"');
    obj = attrValues[ix];
    if (obj instanceof String)
      sb.append((String)obj);
    else if (obj instanceof Integer)
      sb.append(((Integer)obj).toString());
    else if (obj instanceof Double)
      sb.append(((Double)obj).toString());
    sb.append('"');
  }
  return null;
}
public String
toString()
{
  StringBuilder sb = new StringBuilder();
  toString(sb);
  return sb.toString();
}
}
