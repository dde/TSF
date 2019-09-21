package edu.pace.csis.evans.npx;

import org.snave.xml.SAXFactory;
import org.snave.xml.XMLNode;
import org.xml.sax.Locator;
import org.xml.sax.SAXParseException;
import org.xml.sax.ext.DefaultHandler2;

import java.util.logging.Logger;

/*
 * Created by danevans on 3/4/14.
 */
public class SAXEventAdapter extends DefaultHandler2
{
  static final char START_TAG = '<';
  static final char END_TAG   = '>';
  static final char ENTITY    = '&';
  SAXFactory factory;
  Locator locator;
  Logger logger;
  XMLNode document;
  XMLNode current;
  boolean isCData;
public SAXEventAdapter(SAXFactory fac)
{
  factory = fac;
}
public XMLNode
getDocument()
{
  //logger.debug(String.format("get document"));
  return document;
}
public Locator
getDocumentLocator()
{
  return locator;
}
/**
 * @inheritDoc
 */
@Override
public void
setDocumentLocator(Locator loc)
{
  locator = loc;
}
public void
setLogger(Logger logger)
{
  this.logger = logger;
}
/**
 * @inheritDoc
 * Processing for the configuration document begins by creating the root of the WSObject
 * object tree, and setting it to the current configuration object.
 */
@Override
public void
startDocument()
{
  document = factory.build(SAXFactory.DOCUMENT_NODE, (String)null);
  current = document;
  //logger.debug(String.format("start document"));
}
/**
 * @inheritDoc
 */
@Override
public void
endDocument()
{
  //current.process();
  current.endElement(document);
  //logger.debug(String.format("end document"));
}
/**
 * @inheritDoc
 * Create a new Element and append it to the current Element.  Create an Attr node for each attribute
 * and add it to the current Element.
 */
@Override
public void
startElement(String uri, String localName, String qName, org.xml.sax.Attributes attrs)
{
  XMLNode elm;
  int ix, atln;
  //logger.debug(String.format("start element tag %s", qName));
  //elm = new Element(qName, localName, uri);
  elm = factory.build(qName, localName, uri);
  current.appendChild(elm);
  current = elm;
  atln = attrs.getLength();
  for (ix = 0; ix < atln; ++ix)
  {
    current.setAttribute(attrs.getQName(ix), attrs.getLocalName(ix), attrs.getURI(ix), attrs.getValue(ix));
    //logger.debug(String.format("  attribute %s", attrs.getQName(ix)));
  }
}
/**
 * @inheritDoc
 * The ending element is closed by making its parent the current object.
 */
@Override
public void
endElement(String uri, String localName, String qName)
{
  //logger.debug(String.format("end element tag %s", current.nodeName));
  current.endElement(document);
  current = current.getParentNode();
  //logger.debug(String.format("parent element tag %s", current.nodeName));
}
/**
 * @inheritDoc
 * Create a Text node from the passed character array, offset, and length, and append it to
 * the current node.
 */
@Override
public void
characters(char[] ch, int start, int length)
{
  XMLNode text;
  XMLNode cdata;
  //if (isCDATA(ch, start, length))
  if (isCData)
  {
    cdata = factory.build(SAXFactory.CDATA_SECTION_NODE, new String(ch, start, length));
    //logger.debug(String.format("cdata length %d", length));

    if (null != cdata)
      current.appendChild(cdata);
  }
  else
  {
    text = factory.build(SAXFactory.TEXT_NODE, new String(ch, start, length));
    //logger.debug(String.format("text length %d", length));
    if (null != text)
      current.appendChild(text);
  }
}
@Override
public void
startCDATA()
{
  isCData = true;
}
@Override
public void
endCDATA()
{
  isCData = false;
}
/**
 * @inheritDoc
 * Create a Text node from the passed character array, offset, and length, and append it to
 * the current node.
 */
@Override
public void
ignorableWhitespace(char[] ch, int start, int length)
{
  XMLNode text;
  text = factory.build(SAXFactory.TEXT_NODE, new String(ch, start, length));
  if (null != text)
    current.appendChild(text);
}
@Override
public void
processingInstruction(String target, String data)
{
  XMLNode pi;
  pi = factory.build(SAXFactory.PROCESSING_INSTRUCTION_NODE, target, data);
  //logger.debug(String.format("pi %s", target));
  if (null != pi)
    current.appendChild(pi);
}
@Override
public void
skippedEntity(String name)
{
  XMLNode entity;
  entity = factory.build(SAXFactory.ENTITY_REFERENCE_NODE, name);
  //logger.debug(String.format("entity %s", name));
  if (null != entity)
    current.appendChild(entity);
}
/**
 * @inheritDoc
 * Create a Comment node from the passed character array, offset, and length, and append it to
 * the current node.
 */
@Override
public void
comment(char[] ch, int start, int length)
{
  XMLNode comment;
  comment = factory.build(SAXFactory.COMMENT_NODE, new String(ch, start, length));
  //logger.debug(String.format("comment length %d", length));
  if (null != comment)
    current.appendChild(comment);
}
/**
 * Must the passed chracter subarray be enclosed in a CDATA section?
 * @param ch an array of characters
 * @param start starting index of valid characters in the array
 * @param length length of valid characters in the array
 * @return true if the array contains at least one character that needs to be escaped, false otherwise
 */
boolean
isCDATA(char[] ch, int start, int length)
{
  int ix, end;
  char chr;
  end = start + length;
  for (ix = start; ix < end; ++ix)
  {
    chr = ch[ix];
    if (START_TAG == chr || END_TAG == chr || ENTITY == chr)
      return true;
  }
  return false;
}
/**
 * A convenience function to identify a class of ignorable whitespace, which is a string whose
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
@Override
public void
warning(SAXParseException wrn)
{
  if (null != logger)
    logger.warning(wrn.getMessage());
}
@Override
public void
error(SAXParseException err)
{
  if (null != logger)
    logger.severe(err.getMessage());
}
@Override
public void
fatalError(SAXParseException ftl)
{
  if (null != logger)
    logger.severe(ftl.getMessage());
}
}