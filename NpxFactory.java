package edu.pace.csis.evans.npx;

/**
 * Created by danevans on 3/10/14.
 */
public interface NpxFactory
{
NpxCommon build(String qName, String localName, String uri, Object attrs, NpxGlobal interp);
}
