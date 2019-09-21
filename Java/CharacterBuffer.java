package edu.pace.csis.evans.npx;

public class CharacterBuffer
{
  private CharacterSegment head;
  private CharacterSegment cur;
  CharacterBuffer()
  {
    head = new CharacterSegment();
    cur = head;
  }
  void append(String str)
  {
    char[] buf;
    int  ixb, ixs, strlen;
    buf = cur.buf;
    ixs = 0;
    strlen = str.length();
    while (ixs < strlen)
    {
      for (ixb = cur.count; ixs < strlen && ixb < buf.length; ++ixs, ++ixb)
      {
        buf[ixb] = str.charAt(ixs);
      }
      if (buf.length == (cur.count = ixb))
      {
        cur.next = new CharacterSegment();
        cur = cur.next;
        buf = cur.buf;
      }
    }
  }
  void append(int val)
  {
    append("" + val);
  }
  void append(char ch)
  {
    if (cur.buf.length <= cur.count)
    {
      cur.next = new CharacterSegment();
      cur = cur.next;
    }
    cur.buf[cur.count++] = ch;
  }
  int length()
  {
    CharacterSegment cs;
    int len;
    len = 0;
    cs = head;
    while (null != cs)
    {
      len += cs.count;
      cs = cs.next;
    }
    return len;
  }
  int utf8Length()
  {
    CharacterSegment cs;
    int len;
    len = 0;
    cs = head;
    while (null != cs)
    {
      len += cs.utf8Length();
      cs = cs.next;
    }
    return len;
  }
  public String toString()
  {
    char[] ca;
    CharacterSegment cs;
    int len;
    len = 0;
    ca = new char[length()];
    cs = head;
    while (null != cs)
    {
      System.arraycopy(cs.buf, 0, ca, len, cs.count);
      len += cs.count;
      cs = cs.next;
    }
    if (ca.length != len)
      throw new IllegalArgumentException(String.format("CharacterBuffer toString() %d != %d", ca.length, len));
    return new String(ca);
  }
class CharacterSegment
{
  CharacterSegment next;
  int count;
  char[] buf;
  CharacterSegment()
  {
    buf = new char[1024];
  }
  int utf8Length()
  {
    int ix, utf8len;
    char ch;
    utf8len = 0;
    for (ix = 0; ix < count; ++ix)
    {
      ch = buf[ix];
      if (ch < 128)
      {
        utf8len += 1;
      }
      else if (ch < 2048)  //2^11
      {
        utf8len += 2;
      }
      else if (ch < 0xd800 || ch >= 0xe00)
      {
        utf8len += 3;
      }
      else
      {
        ix += 1;
        utf8len += 4;
      }
    }
    return utf8len;
  }
}
}
