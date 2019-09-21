package edu.pace.csis.evans.npx;

import java.io.File;

public class NpxTFX2XML extends NpxRoot
{
  private boolean useEmpty;
public NpxTFX2XML()
{
  useEmpty = true;
}
public NpxTFX2XML(boolean emp)
{
  useEmpty = emp;
}
private void
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
  sb.append(str, 0, ix);
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
      sb.append("&#").append(chr).append(";");
      break;
    }
  }
}
//private boolean
//hasXMLAttrs()
//{
//  return ATTRS == elemTypes[0];
//}
//private boolean
//isEmptyElement()
//{
//  return 0 == siz && ATTRS == elemTypes[0];
//}
private void emptyClose(StringBuilder sb, String tag)
{
  if (useEmpty)
  {
    sb.append('/');
  }
  else
  {
    sb.append("</");
    sb.append(tag);
  }
  sb.append('>');
}
private void
toString(NpxElement npx, StringBuilder sb)
{
  int ix, jx, siz;
  char ctyp;
  NpxElement elm, atr;
  String nam;
  Object obj;
  siz = npx.getCount();
  for (ix = 0; ix < siz; ++ix)
  {
    ctyp = npx.getElemType(ix);
    obj = npx.getElemVal(ix);
    switch (ctyp)
    {
    case ELEMENT:
      nam = npx.getElemName(ix);
      sb.append('<');
      sb.append(nam);
      if (null != obj)
      {
        elm = (NpxElement)obj;
        if (ATTRS == elm.getElemType(0))
        {
          atr = (NpxElement)elm.getElemVal(0);
          for (jx = 0; jx < atr.getCount(); ++jx)
          {
            sb.append(' ');
            sb.append(atr.getElemName(jx));
            sb.append("=\"");
            xmlEscape((String)atr.getElemVal(jx), sb);
            sb.append("\"");
          }
          if (1 == elm.getCount())
          {
            emptyClose(sb, nam);
            continue;
          }
        }
        sb.append('>');
        toString(elm, sb);
        if (1 < elm.getCount() || ATTRS != elm.getElemType(0))  // element is not empty
        {
          sb.append("</");
          sb.append(nam);
          sb.append('>');
        }
      }
      else
      {
        if (!useEmpty)
          sb.append('>');
        emptyClose(sb, nam);
      }
      break;
    case ATTRS:
      break;
    case TEXT:
      xmlEscape((String)obj, sb);
      break;
    case DOCTYPE:
      sb.append("<!DOCTYPE ");
      sb.append((String)obj);
      sb.append(">");
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
      sb.append("&#").append(ctyp).append(";");
      break;
    }
  }
}
public String
toString(NpxElement elm)
{
  StringBuilder sb = new StringBuilder();
  toString(elm, sb);
  return sb.toString();
}
public static void main(String[] args)
{
  String fileName = "/Users/danevans/dan/src/edu/pace/csis/evans/npx/exi-example.txt";
  File file;
  NpxDeserial npo;
  NpxElement npxElement;
  NpxTFX2XML xme;
  xme = new NpxTFX2XML();
  npo = new NpxDeserial();
  file = new File(fileName);
  if (null == (npxElement = npo.deserialize(file)))
  {
    System.out.println("NPX deserialization failure");
    return;
  }
  System.out.println(String.format("%s", xme.toString(npxElement)));
}
}
