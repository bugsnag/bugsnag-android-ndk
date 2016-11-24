package dwarf2;

import java.io.*;

public class addr2line
{
  public static void main (String[] argv) throws Throwable
  {
    //Dwarf2NameFinder finder = new Dwarf2NameFinder (argv[0]);
    Dwarf2NameFinder finder = new Dwarf2NameFinder ("/Users/dave/Bugsnag/bugsnag-android-ndk/bugsnag-android-ndk-test/app/build/intermediates/cmake/release/obj/x86/libjni-entry-point.so");

    BufferedReader in = new BufferedReader (new InputStreamReader (System.in));
    String line;

    while ((line = in.readLine ()) != null)
      {
	if (line.startsWith ("0x"))
	  line = line.substring (2);
	long addr = 0;
	try
	  {
	    addr = Long.parseLong (line, 16);
	  }
	catch (Exception x)
	  {
	    System.out.println ("??:??");
	    continue;
	  }

	finder.lookup (addr);
	String file = finder.getSourceFile ();
	int lineno = finder.getLineNumber ();
	if (file != null && lineno >= 0)
	  System.out.println (file + ":" + lineno);
	else
	  System.out.println ("??:??");
      }
  }
}