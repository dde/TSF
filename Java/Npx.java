package edu.pace.csis.evans.npx;

import edu.pace.csis.evans.schematron.dom.DocumentFactory;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;
import java.util.List;

public class Npx
{
void
list(String flnm) {
  BufferedReader rdr;
  String buf;
  try
  {
    rdr = new BufferedReader(new InputStreamReader(new FileInputStream(flnm)));
    while ((buf = rdr.readLine()) != null)
    {
      System.out.println(buf);
    }
    rdr.close();
  }
  catch (IOException ie)
  {
    System.out.println(ie.getMessage());
  }
}
public static void
main(String[] args) throws Exception
{
  NpxXML2TFX npx;
  NpxDeserial npo;
  String files[];
  String npout;
  String[] fileSuffix;
  NpxElement npxElement;
  int ix, jx;
  File fil, ifil, ofil, tfil;
  FileWriter wtr;
  boolean directory, matches;
  Options opt = null;
  try
  {
    opt = new Options(args);
  }
  catch (IllegalArgumentException iax)
  {
    System.exit(1);
  }
  if (opt.isPerformance())
  {
    if (opt.isConvert2TFX())
    {
      Convert2TFXFiles cnv = new Convert2TFXFiles();
      cnv.exec(opt);
      return;
    }
    if (opt.isDeserialize())
    {
      DeserializeTFXFiles cnv = new DeserializeTFXFiles();
      cnv.exec(opt);
      return;
    }
    if (opt.isXML())
    {
      DeserializeXMLFiles cnv = new DeserializeXMLFiles();
      cnv.exec(opt);
      return;
    }
  }
  ifil = new File(opt.getInfile());
  if (ifil.isDirectory())
  {
    files = ifil.list();
    directory = true;
  }
  else
  {
    files = new String[1];
    files[0] = opt.getInfile();
    directory = false;
  }
  fileSuffix = opt.getFileSuffix();
  for (ix = 0; ix < files.length; ++ix)
  {
    if (directory)
    {
      matches = false;
      for (String fs : fileSuffix)
      {
        if (files[ix].endsWith((fs)))
        {
          matches = true;
          break;
        }
      }
      if (!matches)
        continue;
    }
    npout = null;
    if (opt.isConvert2TFX())
    {
      System.out.println(String.format("converting %s", files[ix]));
      if (directory)
        fil = new File(ifil, files[ix]);
      else
        fil = new File(files[ix]);
      npx = new NpxXML2TFX();
      if (null == (npout = npx.exec(fil)))
        continue;
      System.out.print(npout);
      System.out.println();
    }
    npxElement = null;
    if (opt.isDeserialize())
    {
      if (directory)
        fil = new File(ifil, files[ix]);
      else
        fil = new File(files[ix]);
      System.out.println(String.format("deserializing %s", files[ix]));
      npo = new NpxDeserial();
      if (null == (npxElement = npo.deserialize(fil)))
        continue;
      System.out.println(String.format("%s", new NpxTFX2XML(opt.isUseEmpty()).toString(npxElement)));
    }
    if (opt.isFileOutput())
    {
      tfil = ofil = null;
      jx = files[ix].lastIndexOf('.');
      if (directory)
      {
        if (opt.isConvert2TFX())
          tfil = new File(ifil, files[ix].substring(0, jx) + ".tsf");
        if (opt.isRebuild())
          ofil = new File(ifil, files[ix].substring(0, jx) + ".tsf.out");
      }
      else
      {
        if (opt.isConvert2TFX())
          tfil = new File(files[ix].substring(0, jx) + ".tsf");
        if (opt.isRebuild())
          ofil = new File(files[ix].substring(0, jx) + ".tsf.out");
      }
      try
      {
        if (null != ofil && null != npxElement)
        {
          wtr = new FileWriter(ofil);
          wtr.write(npxElement.toString());
          wtr.close();
        }
        if (null != tfil)
        {
          wtr = new FileWriter(tfil);
          wtr.write(npout.toString());
          wtr.close();
        }
      }
      catch (IOException iox)
      {
        System.err.println(String.format("%s", iox.getMessage()));
      }
    }
  }
}
static String[]
expand(String[] orig, int nwsz)
{
  String[] exp;
  exp = new String[nwsz];
  System.arraycopy(orig, 0, exp, 0, (nwsz < orig.length) ? nwsz : orig.length);
  return exp;
}
static String fileToString(File flnm)
{
  int filsz;
  char[] buf;
  InputStreamReader ism;
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
  filsz -= 1;
  while (buf[filsz] == 0)
  {
    filsz -= 1;
  }
  return new String(buf, 0, filsz + 1);
}
static class Options
{
  String infile;
  String[] fileSuffix = new String[0];
  String[] tmp;
  int ix, jx;
  boolean directory = false;
  boolean convert2TFX = false;
  boolean useEmpty = false;
  boolean rebuild = false;
  boolean fileOutput = false;
  boolean deserialize = false;
  boolean performance = false;
  boolean xmlFlag = false;
  static void usage()
  {
    System.err.println("Usage: " +
                           "java edu.pace.csis.evans.npx.Npx [options] file\n" +
                           " file is an input file, or a directory of potential input files" +
                           "where options include\n" +
                           " -c convert files to TFX format\n" +
                           " -d deserialize TFX files\n" +
                           " -e when generating XML, use empty elemets\n" +
                           " -o write result to output file(s)\n" +
                           " -p performance tests\n" +
                           " -r rebuild XML file from TFX file - creates .xml.out file\n" +
                           " -s suffix of files on which to operate (default .xml, may occur multiple times)\n" +
                           " -x deserialize XML files\n"
    );
    throw new IllegalArgumentException();
  }
Options(String[] args)
{
  infile = null;
  for (ix = 0; ix < args.length; ++ix)
  {
    if (args[ix].charAt(0) == '-')
    {
      if (args[ix].equals("-c"))
      {
        convert2TFX = true;
      }
      else if (args[ix].startsWith("-s"))
      {
        tmp = expand(fileSuffix, fileSuffix.length + 1);
        tmp[fileSuffix.length] = args[ix].substring(2);
        fileSuffix = tmp;
      }
      else if (args[ix].equals("-d"))
      {
        deserialize = true;
      }
      else if (args[ix].equals("-e"))
      {
        useEmpty = true;
      }
      else if (args[ix].equals("-o"))
      {
        fileOutput = true;
      }
      else if (args[ix].equals("-p"))
      {
        performance = true;
      }
      else if (args[ix].equals("-r"))
      {
        rebuild = true;
      }
      else if (args[ix].equals("-x"))
      {
        xmlFlag = true;
      }
      else
      {
        usage();
      }
    }
    else
    {
      if (infile == null)
        infile = args[ix];
      else
        usage();
    }
  }
  if (0 == fileSuffix.length)
  {
    tmp = expand(fileSuffix, fileSuffix.length + 1);
    tmp[fileSuffix.length] = ".xml";
    fileSuffix = tmp;
  }
}
  public String getInfile()
  {
    return infile;
  }
  public String[] getFileSuffix()
  {
    return fileSuffix;
  }
  public boolean isDirectory()
  {
    return directory;
  }
  public boolean isConvert2TFX()
  {
    return convert2TFX;
  }
  public boolean isRebuild()
  {
    return rebuild;
  }
  public boolean isUseEmpty()
  {
    return useEmpty;
  }
  public boolean isFileOutput()
  {
    return fileOutput;
  }
  public boolean isDeserialize()
  {
    return deserialize;
  }
  public boolean isPerformance()
  {
    return performance;
  }
  public boolean isXML()
  {
    return xmlFlag;
  }
}
static class Convert2TFXFiles
{
  ThreadMXBean bean;
  Convert2TFXFiles()
  {
    /** Get CPU time in nanoseconds. */
    bean = ManagementFactory.getThreadMXBean();
    if (!bean.isCurrentThreadCpuTimeSupported())
      throw new UnsupportedOperationException("no thread CPU time");
    if (!bean.isThreadCpuTimeEnabled())
      bean.setThreadCpuTimeEnabled(true);
  }
void exec(Options opt)
{
  int ix, jx;
  Document doc;
  boolean matches,
      directory = false,
      extra = false;
  File dfil, tfil, xfil;
  String[] files;
  NpxXML2TFX npx;
  String npout;
  String[] fileSuffix;
  CharacterBuffer sbout;
  FileWriter wtr;
  long domStartTime, domStopTime, cnvStartTime, cnvStopTime, totalTime;
  totalTime = 0;
  dfil = new File(opt.getInfile());
  fileSuffix = opt.getFileSuffix();
  if (dfil.isDirectory())
  {
    files = dfil.list();
    directory = true;
  }
  else
  {
    files = new String[1];
    files[0] = opt.getInfile();
  }
  ix = files.length;
  files = expand(files, ix + 1);
  files[ix] = null;
  for (ix = 0; ix < files.length; ++ix)
  {
    matches = false;
    if (directory)
    {
      for (String fs : fileSuffix)
      {
        if (files[ix].endsWith((fs)))
        {
          matches = true;
          break;
        }
      }
      if (!matches)
        continue;
    }
    if (!extra)
    {
      files[files.length - 1] = files[ix];
      extra = true;
    }
    System.out.println(String.format("converting %s", files[ix]));
    if (directory)
      xfil = new File(dfil, files[ix]);
    else
      xfil = new File(files[ix]);
    npx = new NpxXML2TFX();
    npout = fileToString(xfil);
    domStartTime = bean.getCurrentThreadCpuTime();
    doc = npx.domProcess(npout);
    domStopTime = bean.getCurrentThreadCpuTime();
    if (null == doc)
    {
      System.out.println(String.format("DOM parse failure for %s", files[ix]));
      continue;
    }
    totalTime += domStopTime - domStartTime;
    //System.out.println(String.format("  CPU time (ms) DOM %d, convert %d, file size %d", domStopTime - domStartTime, xfil.length()));
    //System.out.print(npout);
    //System.out.println();
    tfil = null;
    jx = files[ix].lastIndexOf('.');
    if (directory)
    {
      tfil = new File(dfil, files[ix].substring(0, jx) + ".txt");
    }
    else
    {
      tfil = new File(files[ix].substring(0, jx) + ".txt");
    }
    try
    {
      cnvStartTime = bean.getCurrentThreadCpuTime();
      sbout = npx.transform(doc);
      cnvStopTime = bean.getCurrentThreadCpuTime();
      totalTime += cnvStopTime - cnvStartTime;
      System.out.println(String.format("  CPU time - DOM parse %8.3fms, convert DOM to TFX %8.3fms, file size %8d, TFX size %8d",
          (domStopTime - domStartTime) / 1000000.0, (cnvStopTime - cnvStartTime) / 1000000.0, xfil.length(), sbout.length()));
      System.out.println("  " + npx.stats());
      //wtr = new FileWriter(tfil);
      //wtr.write(npout);
      //wtr.close();
    }
    catch (Exception iox)
    {
      System.err.println(String.format("%s", iox.getMessage()));
    }
  }
  System.out.println(String.format("Total CPU time (ms) %d", totalTime));

}
}
static class DeserializeTFXFiles
{
  ThreadMXBean bean;
  DeserializeTFXFiles()
  {
    /** Get CPU time in nanoseconds. */
    bean = ManagementFactory.getThreadMXBean();
    if (!bean.isCurrentThreadCpuTimeSupported())
      throw new UnsupportedOperationException();
  }
  void exec(Options opt)
  {
    int ix, jx;
    NpxElement tam;
    boolean matches,
        directory = false,
        extra = false;
    File dfil, tfil, xfil;
    String[] files;
    NpxDeserial npx;
    String npout;
    String[] fileSuffix;
    FileWriter wtr;
    long tamStartTime, tamStopTime, totalTime;
    totalTime = 0;
    dfil = new File(opt.getInfile());
    fileSuffix = opt.getFileSuffix();
    if (dfil.isDirectory())
    {
      files = dfil.list();
      directory = true;
    }
    else
    {
      files = new String[1];
      files[0] = opt.getInfile();
    }
    ix = files.length;
    files = expand(files, ix + 1);
    files[ix] = null;
    for (ix = 0; ix < files.length; ++ix)
    {
      matches = false;
      if (directory)
      {
        for (String fs : fileSuffix)
        {
          if (files[ix].endsWith((fs)))
          {
            matches = true;
            break;
          }
        }
        if (!matches)
          continue;
      }
      if (!extra)
      {
        files[files.length - 1] = files[ix];
        extra = true;
      }
      System.out.println(String.format("deserializing %s", files[ix]));
      if (directory)
        xfil = new File(dfil, files[ix]);
      else
        xfil = new File(files[ix]);
      npx = new NpxDeserial();
      npout = fileToString(xfil);
      tamStartTime = bean.getCurrentThreadCpuTime();
      tam = npx.deserialize(npout);
      tamStopTime = bean.getCurrentThreadCpuTime();
      if (null == tam)
      {
        System.out.println(String.format("TFX deserialize failure for %s", files[ix]));
        continue;
      }
      totalTime += tamStopTime - tamStartTime;
      System.out.println(String.format("  CPU time - TFX deserialize %8.3fms, TFX size %8d",
          (tamStopTime - tamStartTime) / 1000000.0, npout.length()));
    }
    System.out.println(String.format("Total CPU time (ms) %d", totalTime));
  }
}
static class DeserializeXMLFiles
{
  ThreadMXBean bean;
  NpxGlobal global;
  DocumentFactory dcf;
  DeserializeXMLFiles()
  {
    NpxFactoryRegistry sfi;
    this.global = global;
    sfi = NpxFactoryRegistry.instance();
    sfi.setGlobal(global);
    dcf = new DocumentFactory(sfi);
    /** Get CPU time in nanoseconds. */
    bean = ManagementFactory.getThreadMXBean();
    if (!bean.isCurrentThreadCpuTimeSupported())
      throw new UnsupportedOperationException();
  }
  void exec(Options opt)
  {
    int ix;
    boolean matches,
        directory = false,
        extra = false;
    File dfil, xfil;
    String[] files;
    String npout;
    String[] fileSuffix;
    FileWriter wtr;
    long tamStartTime, tamStopTime, cnvStartTime, cnvStopTime, totalTime;
    totalTime = 0;
    dfil = new File(opt.getInfile());
    fileSuffix = opt.getFileSuffix();
    if (dfil.isDirectory())
    {
      files = dfil.list();
      directory = true;
    }
    else
    {
      files = new String[1];
      files[0] = opt.getInfile();
    }
    ix = files.length;
    files = expand(files, ix + 1);
    files[ix] = null;
    for (ix = 0; ix < files.length; ++ix)
    {
      matches = false;
      if (directory)
      {
        for (String fs : fileSuffix)
        {
          if (files[ix].endsWith((fs)))
          {
            matches = true;
            break;
          }
        }
        if (!matches)
          continue;
      }
      if (!extra)
      {
        files[files.length - 1] = files[ix];
        extra = true;
      }
      System.out.println(String.format("deserializing %s", files[ix]));
      if (directory)
        xfil = new File(dfil, files[ix]);
      else
        xfil = new File(files[ix]);
      npout = fileToString(xfil);
      edu.pace.csis.evans.schematron.dom.Document sdoc;
      NpxGeneric npxRoot;
      List<String> errs;
      sdoc = null;
      tamStartTime = tamStopTime = 0;
      try
      {
        tamStartTime = bean.getCurrentThreadCpuTime();
        sdoc = dcf.parseFromString(npout, "text/xml");
        tamStopTime = bean.getCurrentThreadCpuTime();
      }
      catch (SAXException sxe)
      {
        System.out.println(String.format("unable to parse schema file %s", xfil.getName()));
        System.out.println(String.format("%s", sxe.getMessage()));
      }
      catch (IOException iox)
      {
        System.out.println(String.format("%s", iox.getMessage()));
      }
      catch (RuntimeException rex)
      {
        System.out.println(String.format("%s", rex.getMessage()));
      }
      if (null == sdoc)
      {
        System.out.println(String.format("XML deserialize failure for %s", files[ix]));
        continue;
      }
      totalTime += tamStopTime - tamStartTime;
      System.out.println(String.format("  CPU time - XML deserialize %8.3fms, XML size %8d",
          (tamStopTime - tamStartTime) / 1000000.0, npout.length()));
    }
    System.out.println(String.format("Total CPU time (ms) %d", totalTime / 1000));
  }
}
}