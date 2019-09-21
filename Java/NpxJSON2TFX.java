package edu.pace.csis.evans.npx;

import org.snave.vpt.json.JSONCallback;
import org.snave.vpt.json.JSONDefaultCallback;
import org.snave.vpt.json.JSONDefaultInputSource;
import org.snave.vpt.json.JSONException;
import org.snave.vpt.json.JSONParse;
import org.snave.vpt.json.JSObject;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;

public class NpxJSON2TFX extends NpxRoot
{
  //int version = 2;
  static final int DEPTH = 50;
  static final int WIDTH = 10000;
  int[] eldepth = new int[DEPTH + 1];
  int[] elwidth = new int[WIDTH + 1];
  short depth = -1;
  int elements;
  int procinsts;
  int pcdatas;
  int cdatas;
  int comments;
  int attributes;
  int doctypes;
  int bytes;


int transformArray(JSObject.ARRAY doc, CharacterBuffer sb)
    throws JSONException
{
  JSObject val;
  int tln, ln, sz;
  String sval;
  tln = 0;
  sz = doc.getSize();
  for (int ix = 0; ix < sz; ++ix)
  {
    val = doc.getPosition(ix);
    if (null == val)
    {
      System.out.println(String.format("null array position %d out of %d", ix, sz));
      continue;
    }
    if (val.isObject())
    {
      ln = val.getSize();
      sb.append(ln);
      sb.append(ELEMENT);
      tln += 1 + numLength(sz) + transformObject((JSObject.OBJECT)val, sb);
    }
    else if (val.isArray())
    {
      ln = val.getSize();
      sb.append(ln);
      sb.append('@');
      tln += 1 + numLength(sz) + transformArray((JSObject.ARRAY)val, sb);
    }
    else if (val.isString() || val.isInteger() || val.isNumber() || val.isTrue() || val.isFalse() || val.isNull())
    {
      sval = val.getStringValue();
      ln = sval.length();
      sb.append(ln);
      sb.append(TEXT);
      sb.append(sval);
      tln += 1 + ln + numLength(ln) + 2;
    }
    else
    {
      throw new JSONException("unknown type in JSON input");
    }
  }
  return tln;
}
int
transformObject(JSObject.OBJECT doc, CharacterBuffer sb) throws JSONException
{
  String prop;
  JSObject val;
  int tln, ln, sz, pln;
  String sval;
  tln = 0;
  for (JSObject.PAIR pair : doc)
  {
    prop = pair.getName();
    pln = prop.length();
    val = pair.getValue();
    if (val.isObject())
    {
      sz = val.getSize();
      sb.append(sz);
      sb.append(prop);
      sb.append(ELEMENT);
      ln = pln + 1 + numLength(sz);
      tln += ln + transformObject((JSObject.OBJECT)val, sb);
    }
    else if (val.isArray())
    {
      sz = val.getSize();
      sb.append(sz);
      sb.append(prop);
      sb.append('@');
      ln = pln + 1 + numLength(sz);
      tln += ln + transformArray((JSObject.ARRAY)val, sb);
    }
    else if (val.isString() || val.isInteger() || val.isNumber() || val.isTrue() || val.isFalse() || val.isNull())
    {
      sval = val.getStringValue();
      sb.append('1');
      sb.append(prop);
      sb.append(ELEMENT);
      ln = sval.length();
      sb.append(ln);
      sb.append(TEXT);
      sb.append(sval);
      tln += pln + 1 + ln + numLength(ln) + 2;
    }
    else
    {
      throw new JSONException("unknown type in JSON input");
    }
  }
  return tln;
}
CharacterBuffer
transform(JSObject doc) throws JSONException
{
  CharacterBuffer rslt = new CharacterBuffer();
  int ln, sz;
  ln = 0;
  if (doc.isObject())
  {
    sz = doc.getSize();
    rslt.append(sz);
    rslt.append(ELEMENT);
    ln = 1 + numLength(sz);
    ln += transformObject((JSObject.OBJECT)doc, rslt);
  }
  else if (doc.isArray())
  {
    sz = doc.getSize();
    rslt.append(sz);
    rslt.append('@');
    ln = 1 + numLength(sz);
    ln += transformArray((JSObject.ARRAY)doc, rslt);
  }
  if (rslt.utf8Length() != ln)
  {
    System.out.println(String.format("length %d != CharacterBuffer length %d", ln, rslt.utf8Length()));
  }
  return rslt;
}
String stats()
{
  StringBuilder sb = new StringBuilder();
  int total = 0, cnt, ix, mx;
  if (0 != elements)
  {
    total += elements;
    sb.append(String.format("elements:%d", elements));
  }
  if (0 != attributes)
  {
    total += attributes;
    sb.append(String.format(", attributes:%d", attributes));
  }
  if (0 != pcdatas)
  {
    total += pcdatas;
    sb.append(String.format(", PCdata:%d", pcdatas));
  }
  if (0 != cdatas)
  {
    total += cdatas;
    sb.append(String.format(", Cdata:%d", cdatas));
  }
  if (0 != comments)
  {
    total += comments;
    sb.append(String.format(", comments:%d", comments));
  }
  if (0 != procinsts)
  {
    total += procinsts;
    sb.append(String.format(", PIs:%d", procinsts));
  }
  if (0 != doctypes)
  {
    total += doctypes;
    sb.append(String.format(", Doctypes:%d", doctypes));
  }
  sb.append(String.format(", total:%d, mean:%7.1f", total, (double)bytes / total));
  cnt = 0;
  for (mx = ix = 0; ix <= DEPTH; ++ix)
  {
    if (0 == eldepth[ix])
      continue;
    cnt += (ix + 1) * eldepth[ix];
    mx = ix;
  }
  sb.append(String.format(", mean depth:%5.1f, max depth:%d", (double)cnt / elements - 1, mx + 1));
  cnt = 0;
  for (mx = ix = 0; ix <= WIDTH; ++ix)
  {
    if (0 == elwidth[ix])
      continue;
    cnt += (ix + 1) * elwidth[ix];
    mx = ix;
  }
  sb.append(String.format(", mean children:%4.1f (%4.1f) , max children:%d",
      (double)cnt / elements - 1, (double)(cnt - (mx + 1)) / elements - 1, mx));
  return sb.toString();
}
String stringFromFile(String flnm) throws IOException
{
  File file;
  FileReader rdr;
  long flsz;
  int chs;
  char[] buf;
  file = new File(flnm);
  flsz = file.length();
  rdr = new FileReader(file);
  buf = new char[(int)flsz];
  chs = rdr.read(buf);
  rdr.close();
  if (chs != flsz)
    System.out.println(String.format("read %d characters from file of size %d", chs, flsz));
  return new String(buf);
}
private static void usage()
{
  System.out.println(String.format("NpxJSON2TFX file-name"));
  System.exit(1);
}
public static void main(String[] args)
{
  String jfileName = "/Users/danevans/dan/test/calc1.json";
  NpxJSON2TFX jse;
  JSONParse jsonParser;
  JSONCallback jcb;
  JSObject jobj;
  CharacterBuffer tfx;
  if (args.length < 1)
  {
    usage();
  }
  jse = new NpxJSON2TFX();
  jfileName = args[0];
  try
  {
    jsonParser = new JSONParse(new JSONDefaultInputSource(jse.stringFromFile(jfileName)), new JSONDefaultCallback());
  }
  catch (IOException iox)
  {
    System.out.println(iox.getMessage());
    return;
  }
  try
  {
    jsonParser.parse();
    jcb = jsonParser.getCallback();
    jobj = jcb.getJSObject();
    tfx = jse.transform(jobj);
    System.out.println(tfx.toString());
  }
  catch (JSONException jsex)
  {
    System.out.println(jsex.getMessage());
    return;
  }
}
}