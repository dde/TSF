package edu.pace.csis.evans.npx;

import edu.pace.csis.evans.schematron.dom.CharacterData;
import edu.pace.csis.evans.schematron.dom.Comment;
import edu.pace.csis.evans.schematron.dom.Element;
import edu.pace.csis.evans.schematron.dom.Node;

import java.util.HashMap;
import java.util.Map;

/*
 * Created by danevans on 3/5/14.
 */

/**
 * SchemaCommon factors the methods and fields which are common to many VXML elements.
 */
abstract public class NpxCommon extends Element implements Cloneable
{
  static final char BKSL = '\\';
  static final char SQ   = '\'';
  static final char QM   = '?';
  static final char AMP  = '&';
  static final char HASH = '#';
  static final char EQ   = '=';
  static final char SEMI = ';';
  static final char NL   = '\n';
  static final char COMMA = ',';
  static final char PERIOD = '.';
  static final String EMPTY_CONTEXT = "<>";

  static final char MACRO = '$';

  static final String TRUE = "true";
  static final String FALSE = "false";

  static final String GET_METHOD = "GET";
  static final String UNDEFINED = "undefined";
  static final String USERDATA = "_userdata_";
  static final String INTERNAL = "_internal_";
  static final String INTERNAL_VALUE = "_VALUE";

  int sequence = 0;
  NpxGlobal global;
  Map<String,String> letScope;
/**
 * Construct a SchemaCommon element from the passed qualified name, local name, and namespaxce URI.
 * @param qName the qualified name of the element
 * @param localName the local name (tag with namespace prefix) of the elemenet
 * @param uri the namespa er URI of the element
 */
NpxCommon(String qName, String localName, String uri, Object attrs)
{
  super(qName, localName, uri, attrs);
}
NpxCommon(String qName, String localName, String uri, Object attrs, NpxGlobal interp)
{
  super(qName, localName, uri, attrs);
  global = interp;
}
NpxGlobal
getNpxGlobal()
{
  return global;
}
String getScopeVariable(String var)
{
  String val = null;
  if (null != letScope)
    val = letScope.get(var);
  if (null != val)
    return val;
  if (isSchema())
    return null;
  return parentCommon().getScopeVariable(var);
}
void setScopeVariable(String var, String val)
{
  if (null == letScope)
    letScope = new HashMap<String,String>();
  letScope.put(var, val);
}
void
compile()
{

}
void execute()
{

}
/**
 * Is this the schema root element?
 * @return false by default
 */
boolean
isSchema()
{
  return false;
}
NpxCommon
parentCommon()
{
  Node par;
  par = parentNode();
  while (!(par instanceof NpxCommon))
  {
    par = par.parentNode();
  }
  return (NpxCommon)par;
}
NpxCommon
getCommonParent(String name)
{
  NpxCommon par;
  par = parentCommon();
  while (!par.isSchema())
  {
    if (name.equals(par.localName()))
      return par;
    par = par.parentCommon();
  }
  return null;
}
String
uniqueName(NpxCommon cmn)
{
  return INTERNAL + cmn.localName() + (++sequence);
}
// Schema Common methods
String
getAttribute(Element elm, String attr)
{
  String value;
  if (null != (value = elm.getAttribute(attr)) && value.length() > 0)
    return value;
  return null;
}
boolean
getTrueFalseAttribute(String attr, boolean dflt)
{
  String value;
  value = getAttribute(attr);
  if (0 == value.length())
    return dflt;
  if (dflt)
    return !FALSE.equalsIgnoreCase(value);
  return TRUE.equalsIgnoreCase(value);
}
String
expandMacros(String subject)
{
  int ix, ln, state, start;
  char ch;
  String val;
  StringBuilder sb = new StringBuilder();
  ln = subject.length();
  start = 0;
  state = 0;
  ix = 0;
  while (ix < ln)
  {
    ch = subject.charAt(ix);
    switch (state)
    {
    case 0: // scanning for a macro
      if (ch == MACRO)
      {
        sb.append(subject.substring(start, ix));
        state = 1;
        start = ix;
      }
      break;
    case 1: // scanning for end of macro
      if (!(Character.isAlphabetic(ch) || Character.isDigit(ch)))
      {
        val = getScopeVariable(subject.substring(start, ix));
        if (null != val)
          sb.append(val);
        state = 0;
        start = ix;
      }
      break;
    default:
      break;
    }
    ++ix;
  }
  if (state == 1)
  {
    val = getScopeVariable(subject.substring(start));
    if (null != val)
      sb.append(val);
    start = ln;
  }
  if (0 == sb.length())
    return subject;
  sb.append(subject.substring(start, ln));
  return sb.toString();
}
org.w3c.dom.Node
getContextNode()
{
  return null;
}
/**
 * Find the next sibling Element of the passed node, skipping over Nodes that are not Elements.
 * @param node the starting node
 * @return the next sibling Element or null if there is none
 */
Element
nextElement(Node node)
{
  for (node = node.nextSibling(); null != node; node = node.nextSibling())
  {
    if (node instanceof org.w3c.dom.Element)
      return (Element)node;
  }
  return null;
}
/**
 * Find the first child Element of the passed node, skipping over child Nodes that are not Elements.
 * @param node the starting parent node
 * @return the first child Element or null if there is none
 */
Element
firstElement(Node node)
{
  for ( ; null != node; node = node.nextSibling())
  {
    if (node instanceof org.w3c.dom.Element)
      return (Element)node;
  }
  return null;
}
NpxCommon
firstCommon()
{
  Node node;
  node = this.firstChild();
  for ( ; null != node; node = node.nextSibling())
  {
    if (node instanceof NpxCommon)
      return (NpxCommon)node;
  }
  return null;
}
NpxCommon
nextCommon()
{
  Node node;
  node = this.nextSibling();
  for ( ; null != node; node = node.nextSibling())
  {
    if (node instanceof NpxCommon)
      return (NpxCommon)node;
  }
  return null;
}
void compileChildren()
{
  NpxCommon elm;
  elm = this.firstCommon();
  while (null != elm)
  {
    elm.compile();
    elm = elm.nextCommon();
  }
}
void executeChildren()
{
  NpxCommon elm;
  elm = this.firstCommon();
  while (null != elm)
  {
    elm.execute();
    elm = elm.nextCommon();
  }
}
String trimLeft(String str)
{
  int ix, ln;
  ln = str.length();
  ix = 0;
  if (0 == ln || !Character.isWhitespace(str.charAt(ix)))
    return str;
  while (ix < ln)
  {
    if (!Character.isWhitespace(str.charAt(ix)))
      return str.substring(ix);
    ++ix;
  }
  return "";
}
String trimRight(String str)
{
  int ix, ln;
  ln = str.length();
  ix = ln - 1;
  if (0 == ln || !Character.isWhitespace(str.charAt(ix)))
    return str;
  while (ix >= 0)
  {
    if (!Character.isWhitespace(str.charAt(ix)))
      return str.substring(0, ix + 1);
    --ix;
  }
  return "";
}
void
getText(StringBuilder sb)
{
  int none = 0, cdata = 1, elem = 2;
  String pdata, data;
  Node node, next;
  int last;
  CharacterData cd;
  last = none;
  next = null;
  for (node = this.firstChild(); null != node; node = next)
  {
    next = node.nextSibling();
    if (node instanceof CharacterData)
    {
      cd = (CharacterData)node;
      if (!(cd instanceof Comment))
      {
        /*
         * 1. if first, trim left
         * 2. if next is cdata or last, trim right
         * 3. if prev is cdata , add blank, trim left
         * 4. add cdata
         */
        data = cd.data();
        if (none == last)
          data = trimLeft(data);
        if (!(next instanceof NpxCommon))
          data = trimRight(data);
        if (cdata == last)
        {
          data = trimLeft(data);
          sb.append(" ");
        }
        sb.append(data);
        last = cdata;
      }
    }
    else if (node instanceof NpxCommon)
    {
      ((NpxCommon)node).getText(sb);
      last = elem;
    }
  }
}
/**
 * Clone this object. The reference to the current global is copied.
 * @return the cloned SchemaCommon object
 * @throws CloneNotSupportedException required by the overridden signature, but not thrown
 */
public NpxCommon
clone()
    throws CloneNotSupportedException
{
  NpxCommon cloned;
  cloned = (NpxCommon)super.clone();
  letScope = null;
  return cloned;
}
}
