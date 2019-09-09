package edu.pace.csis.evans.npx;

/**
 * Created by danevans on 3/8/14.
 */
public class NpxGeneric extends NpxCommon implements Cloneable
{
  public static final String GENERIC = "_generic";
static
{
  NpxFactoryRegistry.registerFactory(new NpxGenericFactory(), GENERIC);
}
NpxGeneric(String qName, String localName, String uri, Object attrs)
{
  super(qName, localName, uri, attrs);
}
org.w3c.dom.Node
getContextNode()
{
  return ((NpxCommon)parentNode()).getContextNode();
}
public NpxGeneric
clone()
    throws CloneNotSupportedException
{
  NpxGeneric cloned;
  cloned = (NpxGeneric)super.clone();
  return cloned;
}
static class NpxGenericFactory implements NpxFactory
{
public NpxCommon
build(String qName, String localName, String uri, Object attrs, NpxGlobal glb)
{
  return new NpxGeneric(qName, localName, uri, attrs);
}
}
}
