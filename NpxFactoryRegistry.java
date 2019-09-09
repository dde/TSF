package edu.pace.csis.evans.npx;

import edu.pace.csis.evans.schematron.dom.LanguageFactoryRegistry;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by danevans on 3/5/14.
 */
public class NpxFactoryRegistry implements LanguageFactoryRegistry
{
  static Map<String,NpxFactory> registry = new HashMap<String,NpxFactory>();
  static final char CLN = ':';
  static NpxFactoryRegistry instance;
  NpxGlobal global;
static
{
  try
  {
    Class.forName(NpxGeneric.class.getName());
  }
  catch (ClassNotFoundException cnx)
  {
    System.out.println(cnx.getClass().getName() + CLN + cnx.getMessage());
    System.exit(1);
  }
  catch (Exception exx)
  {
    System.out.println(exx.getClass().getName() + CLN + exx.getMessage());
    System.exit(1);
  }
}
private NpxFactoryRegistry()
{

}
public static NpxFactoryRegistry
instance()
{
  if (null == instance)
    instance = new NpxFactoryRegistry();
  return instance;
}
static void
registerFactory(NpxFactory factory, String name)
{
  registry.put(name, factory);
}
public NpxCommon
build(String qName, String localName, String uri, Object attrs)
{
  NpxFactory factory;
  if (null == (factory = registry.get(localName)))
  {
    factory = registry.get(NpxGeneric.GENERIC);
  }
  return factory.build(qName, localName, uri, attrs, global);
}
public NpxGlobal getGlobal()
{
  return global;
}
public void setGlobal(NpxGlobal interp)
{
  global = interp;
}
}
