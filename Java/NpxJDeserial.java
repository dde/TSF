package edu.pace.csis.evans.npx;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;

public class NpxJDeserial extends NpxRoot
{
  //NpxDataType dt = new NpxDataType();
/**
 * Scan and convert a sequence of digit characters in the passed string to an integer.
 * @param str the string to be scanned
 * @param idx the starting index for the scan
 * @param rtn a 2-integer array for the return of the scanned number and the following index
 * @return the index of the first non-digit character encountered
 */
private int fieldLength(String str, int idx, int rtn[])
{
  int acc = 0;
  char ch;
  while (Character.isDigit(ch = str.charAt(idx)))
  {
    acc = acc * 10 + numericValue(ch);
    ++idx;
  }
  rtn[0] = acc;
  return rtn[1] = idx;
}
private int fieldLength(char[] str, int idx, int rtn[])
{
  int acc = 0;
  char ch;
  while (Character.isDigit(ch = str[idx]))
  {
    acc = acc * 10 + numericValue(ch);
    ++idx;
  }
  rtn[0] = acc;
  return rtn[1] = idx;
}
/**
 * Extract each of n attribute name/value pairs from the next position in the TFX string.
 * @param npxElement the NpxElement object being built into which the attributes are stored
 * @param str the TFX string
 * @param act the number of attributes to extract
 * @param idx the beginning index of the attribute list in the TFX string
 * @return the index of the character after the extracted attributes
 */
private int
processAttrList(NpxElement npxElement, String str, int act, int idx)
{
  int ndx, ix;
  int[] rtn = new int[2];
  for (ix = 0; ix < act; ++ix)
  {
    ndx = fieldLength(str, idx, rtn);
    while (ATTR != str.charAt(ndx))
    {
      ++ndx;
    }
    idx = rtn[0] + ndx + 1;
    npxElement.addAttr(str, rtn[1], ndx, ndx + 1, idx);
  }
  return idx;
}
private int
processAttrList(NpxElement npxElement, char[] str, int act, int idx)
{
  int ndx, ix;
  int[] rtn = new int[2];
  for (ix = 0; ix < act; ++ix)
  {
    ndx = fieldLength(str, idx, rtn);
    while (ATTR != str[ndx])
    {
      ++ndx;
    }
    idx = rtn[0] + ndx + 1;
    npxElement.addAttr(str, rtn[1], ndx, ndx + 1, idx);
  }
  return idx;
}
/**
 * Extract each of n content elements from the next position in the TFX string.
 * @param npxElement the NpxElement object being built from the content
 * @param str the TFX string
 * @param ct the number of content elements to extract
 * @param ndx the beginning index of the content in the TFX string
 * @return the index of the character after the extracted content
 */
private int
processElems(NpxElement npxElement, String str, int ct, int ndx)
{
  int idx, ix;
  int[] rtn = new int[2], atn = new int[2];
  char ch;
  NpxElement elm;
  idx = ndx;
  for (ix = 0; ix < ct; ++ix)
  {
    ndx = fieldLength(str, idx, rtn);
    ch = str.charAt(ndx);
    if (ch == TEXT || ch == CDATA || ch == COMMENT || ch == PI || ch == DOCTYPE)
    {
      idx = rtn[0] + ++ndx;
      //npxElement.add(str, ch, ndx, idx);
      continue;
    }
    while ((ch = str.charAt(ndx)) != ATTRS && ch != ELEMENT && ch != ARRAY)
    {
      ++ndx;
    }
//    if (ATTRS == ch)
//    {
//      idx = fieldLength(str, ndx + 1, atn);
//      elm = new NpxElement(rtn[0], atn[0]);
//      idx = processAttrList(elm, str, atn[0], idx + 1);
//    }
//    else
//    {
      elm = new NpxElement(rtn[0], npxElement);
      idx = ndx;
//    }
    idx = processElems(elm, str, rtn[0], idx + 1);
    npxElement.addElemX(str, rtn[1], ndx, elm);
  }
  return idx;
}
private int
processElems(NpxElement npxElement, char[] str, int ct, int ndx)
{
  int idx, ix;
  int[] rtn = new int[2], atn = new int[2];
  char ch;
  NpxElement elm;
  idx = ndx;
  for (ix = 0; ix < ct; ++ix)
  {
    ndx = fieldLength(str, idx, rtn);
    ch = str[ndx];
    if (ch == TEXT || ch == CDATA || ch == COMMENT || ch == PI || ch == DOCTYPE)
    {
      idx = rtn[0] + ++ndx;
      //npxElement.add(str, ch, ndx, idx);
      continue;
    }
    while ((ch = str[ndx]) != ATTRS && ch != ELEMENT && ch != ARRAY)
    {
      ++ndx;
    }
//    if (ATTRS == ch)
//    {
//      idx = fieldLength(str, ndx + 1, atn);
//      elm = new NpxElement(rtn[0], atn[0]);
//      idx = processAttrList(elm, str, atn[0], idx + 1);
//    }
//    else
//    {
      elm = new NpxElement(rtn[0], npxElement);
      idx = ndx;
//    }
    idx = processElems(elm, str, rtn[0], idx + 1);
    npxElement.addElemX(str, rtn[1], ndx, elm);
  }
  return idx;
}
/**
 * Deserialize the passed TFX string into a hierarchy of NpxElement objects representing the XML elements
 * of the TFX string.
 * @param str the TFX string to be deserialized
 * @return the NpxElement document object
 */
NpxElement
deserialize(String str)
{
  NpxElement docElement;
  int lst, ndx, idx;
  int[] rtn = new int[2];
  char typ;
  lst = str.length();
  ndx = fieldLength(str, 0, rtn);
  if (ATTRS != str.charAt(ndx)) /* no count of document level lexical units present - 1 is implied */
  {
    rtn[0] = 1;
    idx = 0;
    typ = ELEMENT;
  }
  else
  {
    ++rtn[0];
    idx = ndx + 1;
    typ = ATTRS;
  }
  docElement = new NpxElement(rtn[0], null);
  idx = processElems(docElement, str, rtn[0], idx);
  if (lst != idx)
    System.err.printf("TFX string not completely processed %d vs %d\n", lst, idx);
  return docElement;
}
/**
 * Deserialize the passed TFX file into a hierarchy of NpxElement objects.
 * @param flnm
 * @return
 */
NpxElement
deserialize(File flnm)
{
  NpxElement docElement;
  int lst, ndx, idx;
  int filsz;
  int[] rtn = new int[2];
  InputStreamReader ism;
  char[] buf;
  char typ;
  filsz = (int)flnm.length();
  buf = new char[filsz];
  try
  {
    ism = new InputStreamReader(new FileInputStream(flnm));
    ism.read(buf, 0, filsz);
    ism.close();
  }
  catch (IOException ie)
  {
    System.out.println(ie.getMessage());
    return null;
  }
  lst = filsz;
  ndx = fieldLength(buf, 0, rtn);
  if (ATTRS != buf[ndx]) /* no count of document level lexical units present - 1 is implied */
  {
    rtn[0] = 1;
    idx = 0;
    typ = ELEMENT;
  }
  else
  {
    ++rtn[0];
    idx = ndx + 1;
    typ = ATTRS;
  }
  docElement = new NpxElement(rtn[0], null);
  idx = processElems(docElement, buf, rtn[0], idx);
  if (lst != idx)
    System.err.printf("TFX string not completely processed %d vs %d\n", lst, idx);
  return docElement;
}
}
