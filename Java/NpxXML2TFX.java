package edu.pace.csis.evans.npx;

import org.w3c.dom.Attr;
import org.w3c.dom.CharacterData;
import org.w3c.dom.Comment;
import org.w3c.dom.Document;
import org.w3c.dom.DocumentType;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.ProcessingInstruction;
import org.w3c.dom.Text;
import org.xml.sax.InputSource;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.File;
import java.io.FileInputStream;
import java.io.StringReader;

public class NpxXML2TFX extends NpxRoot
{
  //int version = 2;
  static final int DEPTH = 50;
  static final int WIDTH = 10000;
  boolean seeWhitespace = false;
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
Document
domProcess(Object infile)
{
  DocumentBuilderFactory dbf;
  DocumentBuilder db;
  Document doc;
  dbf = DocumentBuilderFactory.newInstance();
  try
  {
    //dbf.setValidating(true);
    dbf.setNamespaceAware(true);
    //dbf.setAttribute(prop, Boolean.TRUE);
    db = dbf.newDocumentBuilder();
    if (infile instanceof String)
      doc = db.parse(new InputSource(new StringReader((String)infile)));
    else if (infile instanceof File)
      doc = db.parse(new InputSource(new FileInputStream((File)infile)));
    else
      return null;
  }
  catch (Exception exc)
  {
    System.err.println(String.format("%s:%s", exc.getClass().getName(), exc.getMessage()));
    return null;
  }
  return doc;
}
String
domToString(Document doc)
{
  int length;
  NpxEl npxroot;
  StringBuilder npxdoc;
  npxroot = this.new NpxEl(doc);
  length = npxroot.process((Node)doc);
  npxdoc = new StringBuilder();
  npxroot.transform(npxdoc);
  return npxdoc.toString();
}
String
exec(File infile)
{
  Document doc;
  if (null == (doc = domProcess(infile)))
    return null;
  //return domToString(doc);
  return transform(doc).toString();
}
String
exec(String xmlstr)
{
  Document doc;
  if (null == (doc = domProcess(xmlstr)))
    return null;
  //return domToString(doc);
  return transform(doc).toString();
}
int
transformAttrs(NamedNodeMap attrs, int ct, CharacterBuffer sb)
{
  Attr attr;
  String val, nm;
  int i, ln, length;
  sb.append(ATTRS);
  sb.append(ct);
  length = 2 + numLength(ct);
  sb.append(ATTR);
  for (i = 0; i < ct; ++i)  // the DOM parser apparently sorts attributes by name, losing their original order
  {
    attr = (Attr)attrs.item(i);
    nm = attr.getName();
    val = attr.getValue();
    ln = utf8Length(val);
    sb.append(ln);
    sb.append(nm);
    sb.append(ATTR);
    sb.append(val);
    length += 1 + numLength(ln) + utf8Length(nm) + ln;
  }
  return length;
}
int
transformElement(Element elm, CharacterBuffer rslt)
{
  Node node;
  NamedNodeMap attrs;
  int ln, count, length, act;
  String nm;
  ++elements;
  if (++depth < DEPTH)
    eldepth[depth] += 1;
  else
    eldepth[DEPTH] += 1;
  count = 0;
  node = elm.getFirstChild();
  while (node != null)
  {
    ++count;
    node = node.getNextSibling();
  }
  if (count < WIDTH)
    elwidth[count] += 1;
  else
    elwidth[WIDTH] += 1;
  rslt.append(count);
  nm = elm.getNodeName();
  rslt.append(nm);
  length = 1 + utf8Length(nm) + numLength(count);
  attrs = elm.getAttributes();
  if (attrs != null && (act = attrs.getLength()) > 0)
  {
    length += transformAttrs(attrs, act, rslt);
    attributes += act;
  }
  rslt.append(ELEMENT);
  node = elm.getFirstChild();
  while (node != null)
  {
    if (node instanceof Element)
    {
      length += transformElement((Element)node, rslt);
    }
    else if (node instanceof CharacterData)
    {
      length += transformCdata((CharacterData)node, rslt);
    }
    else if (node instanceof ProcessingInstruction)
    {
      length += transformPI((ProcessingInstruction)node, rslt);
    }
    else if (node instanceof DocumentType)
    {
      length += transformDoctype((DocumentType)node, rslt);
    }
    node = node.getNextSibling();
  }
  --depth;
  return length;
}
int
transformDoctype(DocumentType dtp, CharacterBuffer sb)
{
  int ln;
  String sys, pub, pit, ids, intsubset;
  ++doctypes;
  pit = dtp.getName();
  ln = utf8Length(pit);
  pub = dtp.getPublicId();
  sys = dtp.getSystemId();
  intsubset = dtp.getInternalSubset();
  ids = "";
  if (null != pub)
    ids += " PUBLIC " + pub;
  if (null != sys)
    ids += " SYSTEM " + sys;
  if (null != intsubset)
    ids += " [" + intsubset + "]";
  ln += utf8Length(ids);
  sb.append(ln);
  sb.append(DOCTYPE);
  sb.append(pit);
  sb.append(ids);
  return 1 + ln + numLength(ln);
}
int
transformCdata(CharacterData cdn, CharacterBuffer sb)
{
  int ln;
  char ctype;
  char ch;
  int i;
  String cd;
  if (cdn instanceof Comment)
  {
    ctype = COMMENT;
    ++comments;
  }
  else if (cdn instanceof Text)
  {
    ctype = TEXT;
    ++pcdatas;
  }
  else
  {
    ctype = CDATA;
    ++cdatas;
  }
  cd = cdn.getData();
  ln = utf8Length(cd);
  sb.append(ln);
  sb.append(ctype);
  if (seeWhitespace)
  {
    for (i = 0; i < ln; ++i)
    {
      ch = cd.charAt(i);
      switch (ch)
      {
      case '\n':
        sb.append("\\n");
        break;
      case '\t':
        sb.append("\\t");
        break;
      case '\r':
        sb.append("\\r");
        break;
      default:
        sb.append(ch);
        break;
      }
    }
  }
  else
    sb.append(cd);
  return 1 + ln + numLength(ln);
}
int
transformPI(ProcessingInstruction pin, CharacterBuffer sb)
{
  int ln;
  String pit, pid;
  ++procinsts;
  pit = pin.getTarget();
  ln = utf8Length(pit);
  pid = pin.getData();
  ln += utf8Length(pid) + 1;
  sb.append(ln);
  sb.append(PI);
  sb.append(pit);
  sb.append(BLANK);
  sb.append(pid);
  return 1 + ln + numLength(ln);
}
CharacterBuffer
transform(Document doc)
{
  Node sq;
  CharacterBuffer rslt = new CharacterBuffer();
  int ln, docCt;
  docCt = -1;
  sq = doc.getFirstChild();
  while (sq != null)
  {
    ++docCt;
    sq = sq.getNextSibling();
  }
  ln = 0;
  if (docCt > 0)
  {
    rslt.append(docCt);
    rslt.append(ATTRS);
    ln += 1 + numLength(docCt);
  }
  sq = doc.getFirstChild();
  while (sq != null)
  {
    if (sq instanceof ProcessingInstruction)
      ln += transformPI((ProcessingInstruction)sq, rslt);
   else if (sq instanceof CharacterData)
      ln += transformCdata((CharacterData)sq, rslt);
   else if (sq instanceof DocumentType)
      ln += transformDoctype((DocumentType)sq, rslt);
    else
      ln += transformElement((Element)sq, rslt);
    sq = sq.getNextSibling();
  }
  bytes = ln;
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
class NpxEl
{
  Node el;
  NamedNodeMap attrs;
  NpxEl down;
  NpxEl next;
  int length;
  int count;
  int attrlen;
  boolean cdata;
  boolean pi;
  boolean dt;
  char ctype;
  String pit;
  String pid;
  NpxEl(Node el)
  {
    this.el = el;
  }
  void
  setChild(NpxEl ch)
  {
    this.down = ch;
  }
  void
  setSibling(NpxEl sb)
  {
    this.next = sb;
  }
  int
  attrLength()
  {
    int ln;
    int aln;
    String nm;
    String val;
    Attr attr;
    int i;
    int n;
    if (attrs != null && (aln = attrs.getLength()) > 0)
    {
      ln = 1;
      for (i = 0; i < aln; ++i)
      {
        attr = (Attr)attrs.item(i);
        nm = attr.getName();
        val = attr.getValue();
        n = utf8Length(val);
        ln += n + numLength(n);
      }
    }
    else
      ln = 0;
    return ln;
  }
  int processCharacterData(NpxEl npxel, CharacterData cdn)
  {
    int ln;
    if (cdn instanceof Comment)
      npxel.ctype = COMMENT;
    else if (cdn instanceof Text)
      npxel.ctype = TEXT;
    else
      npxel.ctype = CDATA;
    npxel.cdata = true;
    ln = npxel.length = cdn.getLength();
    return ln + numLength(ln);
  }
  int processPI(NpxEl npxel, ProcessingInstruction pin)
  {
    int ln;
    npxel.pit = pin.getTarget();
    ln = npxel.pit.length();
    npxel.pid = pin.getData();
    ln += npxel.pid.length();
    npxel.length = ln += 1;
    npxel.ctype = PI;
    npxel.pi = true;
    return ln + numLength(ln);
  }
  int processDoctype(NpxEl npxel, DocumentType dtp)
  {
    int ln;
    String sys, pub, ids, intsubset;
    npxel.pit = dtp.getName();
    ln = npxel.pit.length();
    pub = dtp.getPublicId();
    sys = dtp.getSystemId();
    intsubset = dtp.getInternalSubset();
    ids = "";
    if (null != pub)
      ids += " PUBLIC " + pub;
    if (null != sys)
      ids += " SYSTEM " + sys;
    if (null != intsubset)
      ids += "[" + intsubset + "]";
    npxel.pid = ids;
    ln += ids.length();
    npxel.length = ln;
    npxel.ctype = DOCTYPE;
    npxel.dt = true;
    return ln + numLength(ln);
  }
  int
  process(Node nd)
  {
    NpxEl npxel;
    NpxEl npxprv;
    Node node;
    Element elm;
    int ln;
    this.el = nd;
    this.length = 0;
    if (nd instanceof Element)
    {
      elm = (Element)nd;
      this.length += 1 + elm.getTagName().length();
      attrs = el.getAttributes();
      if (0 != (this.attrlen = attrLength()))
      {
        this.length += attrlen + 1 + numLength(attrs.getLength());
      }
      attrlen = attrs.getLength();
    }
    //this.length += 1;
    this.count = 0;
    npxprv = null;
    node = el.getFirstChild();
    while (node != null)
    {
      npxel = new NpxEl(node);
      if (npxprv == null)
        this.setChild(npxel);
      else
        npxprv.setSibling(npxel);
      if (node instanceof Element)
      {
        ln = npxel.process(node);
        this.length = ln + numLength(npxel.count);
      }
      else if (node instanceof CharacterData)
      {
        this.length += processCharacterData(npxel, (CharacterData)node);
      }
      else if (node instanceof ProcessingInstruction)
      {
        this.length += processPI(npxel, (ProcessingInstruction)node);
      }
      else if (node instanceof DocumentType)
      {
        this.length += processDoctype(npxel, (DocumentType)node);
      }
      npxprv = npxel;
      ++this.count;
      node = node.getNextSibling();
    }
    return this.length;
  }
  StringBuilder
  transformAttrs(StringBuilder sb)
  {
    Attr attr;
    String nm;
    String val;
    int ln;
    int lnattr;
    int i;
    if (attrs != null && (ln = attrs.getLength()) > 0)
    {
      sb.append(ATTRS);
      sb.append(attrlen);
      sb.append(ATTR);
      for (i = 0; i < ln; ++i)  // the DOM parser apparently sorts attributes by name, losing their original order
      {
        attr = (Attr)attrs.item(i);
        nm = attr.getName();
        val = attr.getValue();
        lnattr = val.length();
        sb.append(lnattr);
        sb.append(nm);
        sb.append(ATTR);
        sb.append(val);
      }
    }
    return sb;
  }
  void
  transformDoctype(StringBuilder sb)
  {
    sb.append(length);
    sb.append(this.ctype);
    sb.append(this.pit);
    sb.append(this.pid);
  }
  void
  transformPI(StringBuilder sb)
  {
    sb.append(length);
    sb.append(this.ctype);
    sb.append(this.pit);
    sb.append(BLANK);
    sb.append(this.pid);
  }
  void
  transformCdata(StringBuilder sb)
  {
    CharacterData data;
    char ch;
    int i;
    String cd;
    data = (CharacterData)el;
    sb.append(length);
    sb.append(this.ctype);
    cd = data.getData();
    if (seeWhitespace)
    {
      for (i = 0; i < length; ++i)
      {
        ch = cd.charAt(i);
        switch (ch)
        {
        case '\n':
          sb.append("\\n");
          break;
        case '\t':
          sb.append("\\t");
          break;
        case '\r':
          sb.append("\\r");
          break;
        default:
          sb.append(ch);
          break;
        }
      }
    }
    else
      sb.append(cd);
  }
  void
  transformElement(StringBuilder rslt)
  {
    NpxEl sq;
    rslt.append(count);
    rslt.append(el.getNodeName());
    transformAttrs(rslt);
    rslt.append(ELEMENT);
    if (down != null)
    {
      sq = down;
      while (sq != null)
      {
        if (sq.cdata)
          sq.transformCdata(rslt);
        else if (sq.pi)
          sq.transformPI(rslt);
        //else if (sq.dt)
        //  sq.transformDoctype(rslt);
        else
          sq.transformElement(rslt);
        sq = sq.next;
      }
    }
  }
  void
  transform(StringBuilder rslt)
  {
    NpxEl sq;
    int docCt;
    docCt = -1;
    sq = down;
    while (sq != null)
    {
      ++docCt;
      sq = sq.next;
    }
    if (docCt > 0)
    {
      rslt.append(docCt);
      rslt.append(ATTRS);
    }
    sq = down;
    while (sq != null)
    {
      if (sq.cdata)
        sq.transformCdata(rslt);
      else if (sq.pi)
        sq.transformPI(rslt);
      else if (sq.dt)
        sq.transformDoctype(rslt);
      else
        sq.transformElement(rslt);
      sq = sq.next;
    }
  }
}
}