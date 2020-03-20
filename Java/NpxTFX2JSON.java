package edu.pace.csis.evans.npx;

import org.snave.vpt.json.JSONException;
import org.snave.vpt.json.JSONCallback;
import org.snave.vpt.json.JSONDefaultCallback;
import org.snave.vpt.json.JSONDefaultInputSource;
import org.snave.vpt.json.JSONParse;
import org.snave.vpt.json.JSObject;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;

public class NpxTFX2JSON
{
  private static final char[] hexDigits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  private boolean useEmpty = false;
  private char[] chbuf = new char[4];
public NpxTFX2JSON()
{
}
public NpxTFX2JSON(boolean emp)
{
  useEmpty = emp;
}
private String ucsHex(char ch)
{
  int ix;
  for (ix = 3; ix >= 0; --ix, ch >>= 4)
  {
    chbuf[ix] = hexDigits[0x0f & ch];
  }
  return new String(chbuf);
}
private String escape(String str)
{
  int ix, ln, nln;
  char ch;
  StringBuilder nstr;
  // pass 1
  nln = ln = str.length();
  for (ix = 0; ix < ln; ++ix)
  {
    ch = str.charAt(ix);
    if (ch < 127)
    {
      switch (ch)
      {
      case '"':
      case '/':
      case '\\':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
        nln += 1;
        break;
      default:
        break;
      }
    }
    else
    {
      nln += 5;
    }
  }
  if (nln == ln)
    return str;
  nstr = new StringBuilder(nln);
  for (ix = 0; ix < ln; ++ix)
  {
    ch = str.charAt(ix);
    if (ch < 127)
    {
      switch (ch)
      {
      case '"':
      case '/':
      case '\\':
        nstr.append("\\");
        nstr.append(ch);
        break;
      case '\b':
        nstr.append("\\b");
        break;
      case '\f':
        nstr.append("\\f");
        break;
      case '\n':
        nstr.append("\\n");
        break;
      case '\r':
        nstr.append("\\r");
        break;
      case '\t':
        nstr.append("\\t");
        break;
      default:
        nstr.append(ch);
        break;
      }
    }
    else
    {
      nstr.append("\\u");
      nstr.append(ucsHex(ch));
    }
  }
  return nstr.toString();
}
private void attrString(NpxElement elm, StringBuilder sb)
{
  int ac, ax;
  boolean first;
//  if (0 != (ac = elm.getAttrCount()))
//  {
//    sb.append("\"attr\":{");
//    first = true;
//    for (ax = 0; ax < ac; ++ax)
//    {
//      if (!first)
//      {
//        sb.append("\",");
//      }
//      sb.append('"');
//      sb.append(elm.getAttrName(ax));
//      sb.append("\":\"");
//      sb.append(escape(elm.getAttrVal(ax)));
//      sb.append('"');
//      first = false;
//    }
//    sb.append("},");
//  }
}
/**
 * A convenience function to identify elements of ignorable whitespace, a string whose
 * first character is a newline, followed by 0 or more spaces.
 */
boolean
isIgnorable(char[] ch, int start, int length)
{
  int ix;
  if (length <= 0)
    return true;
  if (ch[start] != '\n')
    return false;
  for (ix = start + 1; ix < length; ++ix)
  {
    if (ch[ix] != ' ' && ch[ix] != '\t')
      return false;
  }
  return true;
}
private void
toString(NpxElement elm, StringBuilder sb)
{
  int ix, enmln, cdc, siz;
  Object obj;
  NpxElement chelm;
  String elmName;
  boolean skipComma;
  char ctyp;
  cdc = 0;
  siz = elm.getCount();
  skipComma = true;
  for (ix = 0; ix < siz; ++ix)
  {
    obj = elm.getElemVal(ix);
    elmName = elm.getElemName(ix);
    if (skipComma)
      skipComma = false;
    else
      sb.append(',');
    if (0 == (enmln = elmName.length()))
    {
      throw new IllegalArgumentException(String.format("empty element name at element %d", ix));
    }
    ctyp = elmName.charAt(0);
    switch (ctyp)
    {
    case '[':
      sb.append('"');
      sb.append(escape((String)obj));
      sb.append('"');
      //skipComma = true;
      break;
    case ']':
      //sb.append("\"CDATA" + (++cdc) + "\":");
      sb.append('"');
      sb.append(escape((String)obj));
      sb.append('"');
      break;
    case '!':
      sb.append("\"DOCTYPE\":");
      sb.append('"');
      sb.append(escape((String)obj));
      sb.append('"');
      break;
    case '-':
      // no comments in JSON
      //sb.append("<!--");
      //sb.append((String)obj);
      //sb.append("-->");
      skipComma = true;
      break;
    case '?':
      sb.append("\"PI\":");
      sb.append('"');
      sb.append(escape((String)obj));
      sb.append('"');
      break;
    case '@':
      if (!(obj instanceof NpxElement))
      {
        throw new IllegalArgumentException("null or incorrect NpxElement instance");
      }
      chelm = (NpxElement)obj;
      if (1 < enmln)
      {
        sb.append('"');
        sb.append(elmName.substring(1));
        sb.append("\":");
      }
      sb.append('[');
      toString(chelm, sb);
      sb.append(']');
      break;
    case '<':
      if (!(obj instanceof NpxElement))
      {
        throw new IllegalArgumentException("null or incorrect NpxElement instance");
      }
      chelm = (NpxElement)obj;
      if (1 < enmln)
      {
        sb.append('"');
        sb.append(elmName.substring(1));
        sb.append("\":");
      }
      if (1 != chelm.getCount())
      {
        sb.append("{");
        attrString(chelm, sb);
        toString(chelm, sb);
        sb.append("}");
      }
      else
      {
        elmName = chelm.getElemName(0);
        ctyp = elmName.charAt(0);
        if (ctyp == '<' || ctyp == '@')
        {
          sb.append("{");
          attrString(chelm, sb);
          toString(chelm, sb);
          sb.append("}");
        }
        else
        {
          sb.append('"');
          sb.append(escape(chelm.toString()));
          sb.append('"');
        }
      }
      break;
    default:
      throw new IllegalArgumentException(String.format("unknown field type %c", ctyp));
    }
  }
}
public String
toString(NpxElement elm)
{
  StringBuilder sb = new StringBuilder();
  //sb.append('{');
  toString(elm, sb);
  //sb.append('}');
  return sb.toString();
}
String stringFromFile(String flnm) throws IOException
{
  File file;
  FileReader rdr;
  long flsz;
  char[] buf;
  file = new File(flnm);
  flsz = file.length();
  rdr = new FileReader(file);
  buf = new char[(int)flsz];
  rdr.read(buf);
  rdr.close();
  return new String(buf);
}
private static void usage()
{
  System.out.println(String.format("NpxTFX2JSON file-name"));
  System.exit(1);
}
public static void main(String[] args)
{
  String fileName = "/Users/danevans/dan/test/calc1.txt";
  String jfileName = "/Users/danevans/dan/test/calc1.json";
  File file;
  NpxJDeserial npo;
  NpxElement npxElement;
  NpxTFX2JSON jse;
  String jsonString;
  JSONParse jsonParser;
  JSONCallback jcb;
  JSObject jobj;
  if (args.length < 1)
  {
    usage();
  }
  jse = new NpxTFX2JSON();
  npo = new NpxJDeserial();
  //file = new File(fileName);
  file = new File(args[0]);
  if (null == (npxElement = npo.deserialize(file)))
  {
    System.out.println(String.format("NPX deserialization failure"));
    return;
  }
  jsonString = jse.toString(npxElement);
  System.out.println(String.format("%s", jsonString));
//  jsonParser = new JSONParse(new JSONDefaultInputSource(jsonString), new JSONDefaultCallback());
  try
  {
    jsonParser = new JSONParse(new JSONDefaultInputSource(jsonString), new JSONDefaultCallback());
  }
  catch (Exception iox)
  {
    System.out.println(iox.getMessage());
    return;
  }
  try
  {
    jsonParser.parse();
  }
  catch (JSONException jsex)
  {
    System.out.println(jsex.getMessage());
    return;
  }
  jcb = jsonParser.getCallback();
  jobj = jcb.getJSObject();
  System.out.println(String.format("%s", jobj.toString()));
}
}
