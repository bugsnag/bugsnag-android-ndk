/* RangeMap.java -- map ranges of long values to objects.
   Copyright (C) 2006  C. Scott Marshall <casey.s.marshall@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA  */


package dwarf2;

import java.util.Comparator;
import java.util.SortedMap;
import java.util.TreeMap;

public final class RangeMap
{
  private SortedMap ranges;

  public RangeMap()
  {
    ranges = new TreeMap (new RangeComparator ());
  }

  /**
   * Put, or update, a mapping between a range of values and an
   * object. The <code>begin</code> and <code>end</code> values are
   * treated as unsigned 64-bit integers.
   */
  public void put (long begin, long end, Object value)
  {
    Range r1 = new Range (begin, end);

    SortedMap m = ranges.tailMap (r1);
    if (!m.isEmpty ())
      {
	Range r2 = (Range) m.firstKey ();
	if (r2.overlaps (r1))
	  {
	    ranges.remove (r2);
	    ranges.put (r1.mergeWith (r2), value);
	    return;
	  }
      }
    ranges.put (r1, value);
  }

  /**
   * Get the object that is mapped to a range containing the argument,
   * or <code>null</code> if no range is mapped.
   */
  public Object get (long value)
  {
    Range r1 = new Range (value, value);
    SortedMap m = ranges.tailMap (r1);
    if (m.isEmpty ())
      return null;

    Range r2 = (Range) m.firstKey ();
    if (r2.contains (value))
      return m.get (r2);

    return null;
  }

  public int size ()
  {
    return ranges.size ();
  }

  public String toString ()
  {
    return ranges.toString ();
  }

  private class RangeComparator implements Comparator
  {
    public int compare (Object o1, Object o2)
    {
      Range r1 = (Range) o1;
      Range r2 = (Range) o2;

      if (r1.overlaps (r2))
	return 0;

      if (ucomp (r1.begin, r2.end) > 0)
	return 1;
      return -1;
    }
  }

  private class Range
  {
    final long begin;
    final long end;

    Range (final long begin, final long end)
    {
      if (ucomp (begin, end) > 0)
	throw new IllegalArgumentException ("begin is not less than end (unsigned)");
      this.begin = begin;
      this.end = end;
    }

    boolean contains (long value)
    {
      return (ucomp (begin, value) <= 0
	      && ucomp (end, value) >= 0);
    }

    boolean contains (Range that)
    {
      return (ucomp (this.begin, that.begin) <= 0
	      && ucomp (this.end, that.end) >= 0);
    }

    public boolean equals (Object o)
    {
      if (! (o instanceof Range))
	return false;
      return equals ((Range) o);
    }

    boolean equals (Range that)
    {
      return (this.begin == that.begin && this.end == that.end);
    }

    boolean overlaps (Range that)
    {
      return ((ucomp (this.begin, that.begin) <= 0
	       && ucomp (this.end, that.begin) >= 0)
	      || (ucomp (this.end, that.begin) >= 0
		  && ucomp (this.end, that.end) <= 0)
	      || (ucomp (this.begin, that.end) <= 0
		  && ucomp (this.end, that.end) >= 0));
    }

    Range mergeWith (Range that)
    {
      if (!overlaps (that))
	throw new IllegalArgumentException ("ranges don't overlap this=" + this + " that=" + that);
      long begin = this.begin;
      if (ucomp (begin, that.begin) > 0)
	begin = that.begin;
      long end = this.end;
      if (ucomp (end, that.end) < 0)
	end = that.end;
      return new Range (begin, end);
    }

    public String toString ()
    {
      StringBuffer str = new StringBuffer (24);
      str.append ("(0x");

      String s = Long.toHexString (begin);
      int n = 16 - s.length ();
      while ((n--) > 0)
	str.append ('0');
      str.append (s);
      str.append (", 0x");

      s = Long.toHexString (end);
      n = 16 - s.length ();
      while ((n--) > 0)
	str.append ('0');
      str.append (s);
      str.append (")");
      return str.toString ();
    }
  }

  static int ucomp (long l1, long l2)
  {
    if (l1 == l2)
      return 0;

    if (l1 < 0)
      {
	if (l2 < 0)
	  {
	    if (l1 < l2)
	      return 1;
	    else
	      return -1;
	  }
	return 1;
      }
    else
      {
	if (l2 >= 0)
	  {
	    if (l1 < l2)
	      return -1;
	    else
	      return 1;
	  }
	return -1;
      }
  }
}
