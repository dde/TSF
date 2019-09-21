package edu.pace.csis.evans.npx;

class NpxDataType
{
  private static final String[] stateNames = {"Initial", "int", "dec", "dpt", "dpi", "sgn"};
  /*
  * FSM state names defined as integer constants.  The sequential order allows the state to index an array.
  * of state names.
  */
  private static final int STATE_Initial = 0;
  private static final int STATE_int     = STATE_Initial + 1;
  private static final int STATE_dec     = STATE_int + 1;
  private static final int STATE_dpt     = STATE_dec + 1;
  private static final int STATE_dpi     = STATE_dpt + 1;
  private static final int STATE_sgn     = STATE_dpi + 1;
  private final StringBuilder sb = new StringBuilder();
  private int intResult;
  private double realResult;
/**
 * Try to accept the passed sequence of characters as an integer or a real.
 * <pre>
 *      | I   | int | dec | dpt | dpi | sgn |
 * -----|-----|-----|-----|-----|-----|-----|
 *  0-9 | int | int | dec | dec | dec | int |
 *   +  | sgn | err | err | err | err | err |
 *   -  | sgn | err | err | err | err | err |
 *   .  | dpi | dpt | err | err | err | dpi |

 * </pre>
 * @param inputSequence the sequence of characters in the FSM alphabet to be accepted
 * @return true if the sequence is accepted
 */
public int
isNumeric(String inputSequence)
{
  char ch;
  int ix, ln, state, intValue;
  double decValue, decPos;
  boolean digit, minus;
  minus = false;
  sb.setLength(0);
  ln = inputSequence.length();
  state = STATE_Initial;  // initialize the state variable to the initial state
  intValue = 0;
  decValue = 0f;
  decPos = 1;
  for (ix = 0; ix < ln; ++ix)
  {
    ch = inputSequence.charAt(ix);
    digit = Character.isDigit(ch);
    if (!digit && ch != '+' && ch != '-' && ch != '.')
    {
      //throw new IllegalArgumentException(String.format(BAD_CHAR, ch));
      return 2;  // String
    }
    switch (state)
    {
    case STATE_Initial:
      if (digit)
      {
        trace(sb, state);
        intValue = intValue * 10 + (ch - '0');  // assume UTF-8 ?
        state = STATE_int;
      }
      else if ('+' == ch)
      {
        trace(sb, state);
        state = STATE_sgn;
      }
      else if ('-' == ch)
      {
        trace(sb, state);
        minus = true;
        state = STATE_sgn;
      }
      else  // '.'
      {
        trace(sb, state);
        state = STATE_dpi;
      }

      break;
    case STATE_int:
      if (digit)
      {
        trace(sb, state);
        intValue = intValue * 10 + (ch - '0');
      }
      else if ('.' == ch)
      {
        trace(sb, state);
        state = STATE_dpt;
      }
      else
      {
        //throw new IllegalArgumentException(String.format(BAD_CHAR, ch));
        return 2;  // String
      }
      break;
    case STATE_dec:
      if (digit)
      {
        trace(sb, state);
        decPos /= 10f;
        decValue = decValue + decPos * (ch - '0');
      }
      else
      {
        //throw new IllegalArgumentException(String.format(BAD_CHAR, ch));
        return 2;  // String
      }
      break;
    case STATE_dpt:
    case STATE_dpi:
      if (digit)
      {
        trace(sb, state);
        decPos /= 10f;
        decValue = decValue + decPos * (ch - '0');
        state = STATE_dec;
      }
      else
      {
        //throw new IllegalArgumentException(String.format(BAD_CHAR, ch));
        return 2;  // String
      }
      break;
    case STATE_sgn:
      if (digit)
      {
        trace(sb, state);
        intValue = intValue * 10 + (ch - '0');
        state = STATE_int;
      }
      else if ('.' == ch)
      {
        trace(sb, state);
        state = STATE_dpi;
      }
      else
      {
        //throw new IllegalArgumentException(String.format(BAD_CHAR, ch));
        return 2;  // String
      }
      break;
    }
  }
  trace(sb, state); // record the last state
  trace(sb, -1);    // complete the trace string
  if (state != STATE_int && state != STATE_dec && state != STATE_dpt)
    //throw new IllegalArgumentException(String.format(BAD_STATE));
    return 2;  // String
  if (0.0 == decValue)
  {
    intResult = intValue;
    if (minus)
      intResult = -intResult;
    return 0;  // integer
  }
  realResult = (double)intValue + decValue;
  if (minus)
    realResult = -realResult;
  return 1;  // real
}
private void
trace(StringBuilder sb, int state)
{
  if (state >= 0)
  {
    sb.append(stateNames[state]);
    sb.append(",");
  }
  else
  {
    sb.setLength(sb.length() - 1);
  }
}
public String
getTrace()
{
  return sb.toString();
}
int
intValue()
{
  return intResult;
}
double
realValue()
{
  return realResult;
}
}
