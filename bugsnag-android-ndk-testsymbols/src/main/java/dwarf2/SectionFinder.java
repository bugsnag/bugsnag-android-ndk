/* SectionFinderELF32.java -- finds sections by name in 32-bit ELF files.
   Copyright (C) 2005  Free Software Foundation, Inc.

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

// Written by Casey Marshall <csm@gnu.org>


package dwarf2;

// import gnu.classpath.Configuration;

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteOrder;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;

import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A utility class to find, and map, named sections in executable files.
 *
 * This version is for the Executable and Linking Format, ELF, for 32-bit
 * machines.
 */
class SectionFinder
{

  private static final Logger logger = Logger.getLogger (SectionFinder.class.getName ());
  private static final debug DEBUG = new debug ();

  private static final byte ELFMAG0 = 0x7f;
  private static final byte ELFMAG1 = (byte) 'E';
  private static final byte ELFMAG2 = (byte) 'L';
  private static final byte ELFMAG3 = (byte) 'F';

  private static final class debug extends Level
  {
    private debug () { super ("DWARF-2(ELF-32)", Level.INFO.intValue()); }
  }

  private static final boolean WORDS_BIGENDIAN =
    ByteOrder.nativeOrder ().equals (ByteOrder.BIG_ENDIAN);

  /**
   * This class is similar to the 'struct Elf32_Ehdr' in the GNU C
   * library.
   */
  private static class Elf32_Ehdr
  {
    private static final int EI_NIDENT = 16;

    private final byte[] e_ident   /* Magic number and other info. */
      = new byte[EI_NIDENT];
    private int e_type;            /* Object file type. */
    private int e_machine;         /* Architecture type. */
    private int e_version;         /* Object file version. */
    private long e_entry;          /* Entry point virtual address. */
    private long e_phoff;          /* Program header table file offset. */
    private long e_shoff;          /* Section header table file offset. */
    private int e_flags;           /* Processor-specific flags. */
    private int e_ehsize;          /* ELF header size in bytes. */
    private int e_phentsize;       /* Program header table entry size. */
    private int e_phentnum;        /* Program header table entry count. */
    private int e_shentsize;       /* Section header table entry size. */
    private int e_shnum;           /* Section header table entry count. */
    private int e_shstrndx;        /* Section header string table index. */

    private Elf32_Ehdr () { }

    private void read (RandomAccessFile f) throws IOException
    {
      f.readFully (e_ident);
      e_type = readUHalf (f);
      e_machine = readUHalf (f);
      e_version = readWord (f);
      e_entry = readUWord (f);
      e_phoff = readUWord (f);
      e_shoff = readUWord (f);
      e_flags = readWord (f);
      e_ehsize = readUHalf (f);
      e_phentsize = readUHalf (f);
      e_phentnum = readUHalf (f);
      e_shentsize = readUHalf (f);
      e_shnum = readUHalf (f);
      e_shstrndx = readUHalf (f);
    }

    public String toString ()
    {
      java.lang.StringBuffer str = new java.lang.StringBuffer (super.toString ());
      str.append (" [ e_ident: ");
      for (int i = 0; i < EI_NIDENT; i++)
	{
	  if (e_ident[i] < 0x10 && e_ident[i] >= 0)
	    str.append ('0');
	  str.append (Integer.toHexString (e_ident[i] & 0xFF));
	}
      str.append ("; e_type: ").append (e_type & 0xFFFF);
      str.append ("; e_machine: ").append (e_machine & 0xFFFF);
      str.append ("; e_version: ").append (e_version & 0xFFFF);
      str.append ("; e_entry: 0x").append (Long.toHexString (e_entry));
      str.append ("; e_phoff: ").append ((long) e_phoff & 0xFFFFFFFFL);
      str.append ("; e_shoff: ").append ((long) e_shoff & 0xFFFFFFFFL);
      str.append ("; e_flags: 0x").append (Integer.toHexString (e_flags));
      str.append ("; e_ehsize: ").append (e_ehsize & 0xFFFF);
      str.append ("; e_phentsize: ").append (e_phentsize & 0xFFFF);
      str.append ("; e_phentnum: ").append (e_phentnum & 0xFFFF);
      str.append ("; e_shentsize: ").append (e_shentsize & 0xFFFF);
      str.append ("; e_shnum: ").append (e_shnum & 0xFFFF);
      str.append ("; e_shstrndx: ").append (e_shstrndx & 0xFFFF);
      str.append (" ]");
      return str.toString ();
    }
  }

  private static class Elf32_Shdr
  {
    private long sh_name;
    private int sh_type;
    private int sh_flags;
    private long sh_addr;
    private long sh_offset;
    private long sh_size;
    private int sh_link;
    private int sh_info;
    private int sh_addralign;
    private long sh_entsize;

    private Elf32_Shdr () { }

    private static int sizeof () { return 40; }

    private void read (RandomAccessFile f) throws IOException
    {
      sh_name = readUWord (f);
      sh_type = readWord (f);
      sh_flags = readWord (f);
      sh_addr = readUWord (f);
      sh_offset = readUWord (f);
      sh_size = readUWord (f);
      sh_link = readWord (f);
      sh_info = readWord (f);
      sh_addralign = readWord (f);
      sh_entsize = readWord (f);
    }

    public String toString ()
    {
      java.lang.StringBuffer str = new java.lang.StringBuffer (super.toString ());
      str.append (" [ sh_name: ").append (sh_name);
      str.append ("; sh_type: ").append (sh_type);
      str.append ("; sh_flags: 0x").append (Long.toHexString (sh_flags));
      str.append ("; sh_addr: 0x").append (Long.toHexString (sh_addr));
      str.append ("; sh_offset: ").append (sh_offset);
      str.append ("; sh_size: ").append (sh_size);
      str.append ("; sh_link: ").append (sh_link);
      str.append ("; sh_info: ").append (sh_info);
      str.append ("; sh_addralgin: ").append (sh_addralign);
      str.append ("; sh_entsize: ").append (sh_entsize);
      str.append (" ]");
      return str.toString ();
    }
  }

  /**
   * Map the named section from the given ELF file.
   *
   * @param file The file to look in.
   * @param section The name of the section to map.
   * @return The mapped byte buffer of the named section.
   * @throws IOException If the named section cannot be found, or if
   *  some other IO error occurs.
   */
  static MappedByteBuffer mapSection (String file, String section)
    throws IOException
  {
    if (Configuration.DEBUG)
      logger.log (DEBUG, "mapSection {0}", file);
    RandomAccessFile f = new RandomAccessFile (file, "r");

    /** Read the ELF header. */
    Elf32_Ehdr ehdr = new Elf32_Ehdr ();
    ehdr.read (f);
    if (ehdr.e_ident[0] != ELFMAG0 || ehdr.e_ident[1] != ELFMAG1
	|| ehdr.e_ident[2] != ELFMAG2 || ehdr.e_ident[3] != ELFMAG3)
      {
	f.close ();
	throw new IOException (file + ": not an ELF file");
      }
    if (Configuration.DEBUG)
      logger.log (DEBUG, "read ELF header: {0}", ehdr);

    /* Read the string table section header. */
    Elf32_Shdr strtabhdr = new Elf32_Shdr ();
    f.seek (ehdr.e_shoff + (ehdr.e_shstrndx * Elf32_Shdr.sizeof ()));
    strtabhdr.read (f);
    if (Configuration.DEBUG)
      logger.log (DEBUG, "read string table section header: {0}", strtabhdr);

    Elf32_Shdr shdr = new Elf32_Shdr ();
    byte[] target_bytes = section.getBytes ("ISO8859-1");
    byte[] buf = new byte[target_bytes.length + 1];
    boolean found = false;

    outer: for (int i = 0; i < ehdr.e_shnum; i++)
      {
	// Read information about this section.
	f.seek (ehdr.e_shoff + (i * Elf32_Shdr.sizeof ()));
	shdr.read (f);
	if (Configuration.DEBUG)
	  logger.log (DEBUG, "checking section: {0}", shdr);

	// Read the section name from the string table.
	f.seek (strtabhdr.sh_offset + shdr.sh_name);
	f.readFully (buf);
	if (Configuration.DEBUG)
	  logger.log (DEBUG, "this section is \"{0}\"", new String (buf));
	for (int j = 0; j < target_bytes.length; j++)
	  if (target_bytes[j] != buf[j])
	    continue outer;
	if (buf[buf.length - 1] != '\0')
	  continue;

	if (Configuration.DEBUG)
	  logger.log (DEBUG, "found section {0}: {1}", new Object[]
	    { section, shdr });
	found = true;
	break;
      }

    if (!found)
      {
	f.close ();
	throw new IOException ("no section " + section + " found in " + file);
      }

    try
      {
	FileChannel chan = f.getChannel ();
	MappedByteBuffer buffer = chan.map (FileChannel.MapMode.READ_ONLY,
					    shdr.sh_offset, shdr.sh_size);
	buffer.order (ByteOrder.nativeOrder ());
	chan.close ();
	return buffer;
      }
    finally
      {
	f.close ();
      }
  }

  private static int readUHalf (RandomAccessFile f) throws IOException
  {
    if (WORDS_BIGENDIAN)
      return f.readUnsignedShort ();
    else
      return f.readUnsignedByte () | (f.readUnsignedByte () << 8);
  }

  private static int readWord (RandomAccessFile f) throws IOException
  {
    if (WORDS_BIGENDIAN)
      return f.readInt ();
    else
      return (f.readUnsignedByte () | (f.readUnsignedByte () << 8)
	      | (f.readUnsignedByte () << 16) | (f.readUnsignedByte () << 24));
  }

  private static long readUWord (RandomAccessFile f) throws IOException
  {
    if (WORDS_BIGENDIAN)
      return (long) f.readInt () & 0xFFFFFFFFL;
    else
      {
	long l = (f.readUnsignedByte () | (f.readUnsignedByte () << 8)
		  | (f.readUnsignedByte () << 16) | (f.readUnsignedByte () << 24));
	return (l & 0xFFFFFFFFL);
      }
  }
}
