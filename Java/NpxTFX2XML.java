package edu.pace.csis.evans.npx;

import java.io.File;

public class NpxTFX2XML
{
  private boolean useEmpty = false;
public NpxTFX2XML()
{
}
public NpxTFX2XML(boolean emp)
{
  useEmpty = emp;
}
private void
toString(NpxElement elm, StringBuilder sb)
{
  int ix, ax, ac, siz;
  Object obj;
  NpxElement chelm;
  String elmName;
  siz = elm.getCount();
  for (ix = 0; ix < siz; ++ix)
  {
    obj = elm.getElemVal(ix);
    elmName = elm.getElemName(ix);
    if (0 == elmName.length())
    {
      throw new IllegalArgumentException(String.format("empty element name at element %d", ix));
    }
    switch (elmName.charAt(0))
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
      if (!(obj instanceof NpxElement))
      {
        throw new IllegalArgumentException("null or incorrect NpxElement instance");
      }
      chelm = (NpxElement)obj;
      sb.append('<');
      sb.append(elm.getElemName(ix));
      if (0 != (ac = chelm.getAttrCount()))
      {
        for (ax = 0; ax < ac; ++ax)
        {
          sb.append(' ');
          sb.append(chelm.getAttrName(ax));
          sb.append("=\"");
          sb.append(chelm.getAttrVal(ax));
          sb.append('"');
        }
      }
      if (0 != chelm.getCount())
      {
        sb.append('>');
        toString(chelm, sb);
        sb.append('<');
        sb.append('/');
        sb.append(elm.getElemName(ix));
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
          sb.append(elm.getElemName(ix));
        }
      }
      sb.append('>');
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
    System.out.println(String.format("NPX deserialization failure"));
    return;
  }
  System.out.println(String.format("%s", xme.toString(npxElement)));
}
}
