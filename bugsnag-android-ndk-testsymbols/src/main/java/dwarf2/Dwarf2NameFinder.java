/* Dwarf2NameFinder.java -- decodes the DWARF-2 debug_line section.
   Copyright (C) 2005  Free Software Foundation, Inc.

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

// Written by Casey Marshall <csm@gnu.org>


package dwarf2;

// import gnu.classpath.Configuration;

import java.io.File;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * An interpreter for DWARF-2 "debug_line" byte codes, which, if given
 * the program counter of a running program, can determine the source
 * file and line number (and column number, but that information is not
 * currently emitted by GCC) of that statement.
 */
public class Dwarf2NameFinder {
    private static final Logger logger = Logger.getLogger(Dwarf2NameFinder.class.getName());
    private static final String DEBUG_LINE = ".debug_line";

    private String binaryFile;
    private String sourceFile = null;
    private int lineNumber = -1;

    /**
     * This is the '.debug_line' section of 'binaryFile', mapped into
     * memory. It contains the DWARF-2 byte codes for the file and line
     * number information.
     */
    private MappedByteBuffer dw2;

    /**
     * A mapping of target ranges to sections where the debug info for
     * that range of addresses can be found.
     */
    private final RangeMap cache;

    private List<FileLocation> fileLocationMap;

    private static final int DW_LNS_extended_op = 0;
    private static final int DW_LNS_copy = 1;
    private static final int DW_LNS_advance_pc = 2;
    private static final int DW_LNS_advance_line = 3;
    private static final int DW_LNS_set_file = 4;
    private static final int DW_LNS_set_column = 5;
    private static final int DW_LNS_negate_stmt = 6;
    private static final int DW_LNS_set_basic_block = 7;
    private static final int DW_LNS_const_add_pc = 8;
    private static final int DW_LNS_fixed_advance_pc = 9;
    private static final int DW_LNS_set_prologue_end = 10;
    private static final int DW_LNS_set_epilogue_begin = 11;
    private static final int DW_LNS_set_isa = 12;

    private static final int DW_LNE_end_sequence = 1;
    private static final int DW_LNE_set_address = 2;
    private static final int DW_LNE_define_file = 3;

    private static final debug DEBUG = new debug();

    private static final class debug extends Level {
        private debug() {
            super("DWARF-2", INFO.intValue());
        }
    }

    private static class dw2_debug_line {
        long total_length;
        int version;
        long prologue_length;
        int minimum_instruction_length;
        boolean default_is_stmt;
        byte line_base;
        int line_range;
        int opcode_base;
        final byte[] standard_opcode_lengths = new byte[12];

        private void get(ByteBuffer b) {
            total_length = (long) b.getInt() & 0xFFFFFFFFL;
            version = b.getShort() & 0xFFFF;
            prologue_length = (long) b.getInt() & 0xFFFFFFFFL;
            minimum_instruction_length = b.get() & 0xFF;
            default_is_stmt = b.get() != 0;
            line_base = b.get();
            line_range = b.get() & 0xFF;
            opcode_base = b.get() & 0xFF;
            b.get(standard_opcode_lengths);
        }

        public String toString() {
            java.lang.StringBuffer str = new java.lang.StringBuffer(super.toString());
            str.append(" [ total_length: ").append(total_length);
            str.append("; version: ").append(version);
            str.append("; prologue_length: ").append(prologue_length);
            str.append("; minimum_instruction_length: ").append(minimum_instruction_length);
            str.append("; default_is_stmt: ").append(default_is_stmt);
            str.append("; line_base: ").append(line_base);
            str.append("; line_range: ").append(line_range);
            str.append("; opcode_base: ").append(opcode_base);
            str.append("; standard_opcode_lengths: { ");
            str.append(standard_opcode_lengths[0]).append(", ");
            str.append(standard_opcode_lengths[1]).append(", ");
            str.append(standard_opcode_lengths[2]).append(", ");
            str.append(standard_opcode_lengths[3]).append(", ");
            str.append(standard_opcode_lengths[4]).append(", ");
            str.append(standard_opcode_lengths[5]).append(", ");
            str.append(standard_opcode_lengths[6]).append(", ");
            str.append(standard_opcode_lengths[7]).append(", ");
            str.append(standard_opcode_lengths[8]).append(", ");
            str.append(standard_opcode_lengths[9]).append(", ");
            str.append(standard_opcode_lengths[10]).append(", ");
            str.append(standard_opcode_lengths[11]).append(" } ]");
            return str.toString();
        }
    }

    public Dwarf2NameFinder(final String binaryFile) {
        cache = new RangeMap();
        fileLocationMap = new ArrayList<>();

        try {
            dw2 = SectionFinder.mapSection(binaryFile, DEBUG_LINE);
            this.binaryFile = binaryFile;

            Runtime r = Runtime.getRuntime();
            System.out.println("mem free = " + r.freeMemory() + " total = " + r.totalMemory());
            System.out.print("Scanning " + binaryFile + "...");
            long now = -System.currentTimeMillis();
            scan();
            now += System.currentTimeMillis();
            System.out.println("done");
            System.out.println("scanned " + cache.size() + " compilation units in " + now + " nss");
            System.gc();
            System.out.println("mem free = " + r.freeMemory() + " total = " + r.totalMemory());
            System.err.println(cache);
        } catch (IOException ioe) {
            if (Configuration.DEBUG)
                logger.log(DEBUG, "can''t map .debug_line in {0}: {1}", new Object[]
                        {binaryFile, ioe.getMessage()});
            dw2 = null;
            this.binaryFile = null;
            return;
        }
    }

    public void lookup(final long target) {
        lookup(target, false);
    }

    private void scan() {
        lookup(-1, true);

        for (FileLocation loc : fileLocationMap) {
            System.out.println(loc.address + " - " + loc.filename + " : " + loc.lineNumber);
        }

    }

    private void lookup(final long target, boolean scan_only) {
        if (Configuration.DEBUG)
            logger.log(DEBUG, "Dwarf2NameFinder.lookup: {0} 0x{1}",
                    new Object[]{binaryFile, Long.toHexString(target)});

        lineNumber = -1;
        sourceFile = null;

        if (dw2 == null)
            return;

        CacheEntry e = (CacheEntry) cache.get(target);
        if (e != null) {
            System.out.println("got cache entry " + e);
            if (interpret(target, e.section, e.fileNames, e.header, false))
                return;
        }

        dw2.position(0);
        dw2.limit(dw2.capacity());

        if (Configuration.DEBUG)
            logger.log(DEBUG, "Mapped .debug_line section is {0} bytes", new Integer(dw2.capacity()));

        while (dw2.hasRemaining()) {
            final int begin = dw2.position();
            dw2_debug_line header = new dw2_debug_line();
            header.get(dw2);
            if (Configuration.DEBUG)
                logger.log(DEBUG, "read debug_line header: {0}", header);
            final int end = (int) (begin + header.total_length + 4);
            final int prologue_end = (int) (begin + header.prologue_length + 9);

            if (Configuration.DEBUG)
                logger.log(DEBUG, "this section starts at {0}, ends at {1}, and the prologue ends at {2}",
                        new Object[]{new Integer(begin), new Integer(end),
                                new Integer(prologue_end)});

            if (header.version != 2 /*|| header.opcode_base != 10*/) {
                if (Configuration.DEBUG)
                    logger.log(DEBUG, "skipping this section; not DWARF-2 (version={0}, opcode_base={1})",
                            new Object[]{new Integer(header.version),
                                    new Integer(header.opcode_base)});
                dw2.position(end);
                continue;
            }

            dw2.limit(prologue_end);
            ByteBuffer prologue = dw2.slice();

            // get the directories; they end with a single null byte.
            String dname;
            LinkedList dnames = new LinkedList();
            while ((dname = getString(prologue)).length() > 0) {
                dnames.add(dname);
            }

            // Read the file names.
            LinkedList fnames = new LinkedList();
            while (prologue.hasRemaining()) {
                String fname = getString(prologue);

                long dir = getUleb128(prologue);
                long time = getUleb128(prologue);
                long size = getUleb128(prologue);

                fnames.add(dnames.get((int)dir - 1) + File.separator + fname);
            }
            prologue = null;

            dw2.limit(end);
            dw2.position(prologue_end + 1);
            ByteBuffer section = dw2.slice();
            dw2.limit(dw2.capacity());
            dw2.position(end);

            if (interpret(target, section, fnames, header, scan_only))
                break;
        }
    }

    private boolean interpret(long target, ByteBuffer section, LinkedList fnames,
                              dw2_debug_line header, boolean scan_only) {
        long address = 0;
        long base_address = 0;
        String define_file = null;
        int fileno = 0;
        int lineno = 1;
        int prev_fileno = 0;
        int prev_lineno = 1;
        final int const_pc_add = 245 / header.line_range;

        long min_address = -1;
        long max_address = 0;

        section.position(0);
        section.limit(section.capacity());

        interpret:
        while (section.hasRemaining()) {
            int opcode = section.get() & 0xFF;

            if (opcode < header.opcode_base) {
                switch (opcode) {
                    case DW_LNS_extended_op: {
                        long insn_len = getUleb128(section);
                        opcode = section.get();
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "special opcode {0}, insn_len={1}",
                                    new Object[]{new Integer(opcode), new Long(insn_len)});

                        switch (opcode) {
                            case DW_LNE_end_sequence:
                                if (Configuration.DEBUG)
                                    logger.log(DEBUG, "End of sequence");
                                if (!scan_only && (base_address <= target && address > target)) {
                                    lineNumber = prev_lineno;
                                    sourceFile = (String) ((prev_fileno >= 0 && prev_fileno < fnames.size())
                                            ? fnames.get(prev_fileno) : define_file);
                                    logger.log(DEBUG, "found {0}:{1} for {2}", new Object[]
                                            {sourceFile, new Integer(lineNumber), Long.toHexString(target)});

                                    cache.put(base_address, address,
                                            new CacheEntry(fnames, section, header));

                                    return true;
                                }

                                if (scan_only && min_address != -1 && max_address != 0) {
                                    cache.put(min_address, max_address,
                                            new CacheEntry(fnames, section, header));
                                    min_address = -1;
                                    max_address = 0;
                                }

                                prev_lineno = lineno = 1;
                                prev_fileno = fileno = 0;
                                base_address = address = 0;
                                break;

                            case DW_LNE_set_address:
                                base_address = section.get() & 0xFF;
                                base_address |= (section.get() & 0xFFL) << 8;
                                base_address |= (section.get() & 0xFFL) << 16;
                                base_address |= (section.get() & 0xFFL) << 24;
                                address = base_address;
                                if (Configuration.DEBUG)
                                    logger.log(DEBUG, "Set address to 0x{0}", Long.toHexString(address));

                                // XXX this might not be correct, as there can be more
                                // than one of these instructions in a single compilation
                                // unit.
                                if (!scan_only && address > target) {
                                    if (Configuration.DEBUG)
                                        logger.log(DEBUG, "not in this unit base=0x{0}, target=0x{1}",
                                                new Object[]{Long.toHexString(address),
                                                        Long.toHexString(target)});
                                    return false;
                                }
                                break;

                            case DW_LNE_define_file:
                                define_file = getString(section);
                                if (Configuration.DEBUG)
                                    logger.log(DEBUG, "Define file: {0}", define_file);
                                getUleb128(section);
                                getUleb128(section);
                                getUleb128(section);
                                break;

                            default:
                                if (Configuration.DEBUG)
                                    logger.log(DEBUG, "Unsupported extended opcode {0}",
                                            new Integer(opcode));
                                section.position(section.position() + (int) insn_len);
                                break;
                        }

                    }
                    case DW_LNS_copy:
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Copy");
                        if (!scan_only && (base_address <= target && address > target)) {
                            lineNumber = prev_lineno;
                            sourceFile = (String) ((prev_fileno >= 0 && prev_fileno < fnames.size())
                                    ? fnames.get(prev_fileno) : define_file);
                            logger.log(DEBUG, "found {0}:{1} for {2}", new Object[]
                                    {sourceFile, new Integer(lineNumber), Long.toHexString(target)});

                            cache.put(base_address, address,
                                    new CacheEntry(fnames, section, header));

                            return true;
                        }
                        prev_lineno = lineno;
                        prev_fileno = fileno;
                        break;

                    case DW_LNS_advance_pc: {
                        long amt = getUleb128(section);
                        address += amt * header.minimum_instruction_length;
                        if (scan_only) {
                            if (ucomp(min_address, address) > 0)
                                min_address = address;
                            if (ucomp(max_address, address) < 0)
                                max_address = address;
                        }

                        fileLocationMap.add(new FileLocation(address, (String)fnames.get(fileno), lineno));

                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Advance PC by {0} to 0x{1}",
                                    new Object[]{new Long(amt),
                                            Long.toHexString(address)});
                    }
                    break;

                    case DW_LNS_advance_line: {
                        long amt = getSleb128(section);
                        prev_lineno = lineno;
                        lineno += (int) amt;

                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Advance line by {0} to {1}", new Object[]
                                    {new Long(amt), new Integer(lineno)});
                    }
                    break;

                    case DW_LNS_set_file:
                        prev_fileno = fileno;
                        fileno = (int) getUleb128(section) - 1;
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Set file to {0}", new Integer(fileno));
                        break;

                    case DW_LNS_set_column:
                        getUleb128(section);
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Set column (ignored)");
                        break;

                    case DW_LNS_negate_stmt:
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Negate statement (ignored)");
                        break;

                    case DW_LNS_set_basic_block:
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Set basic block (ignored)");
                        break;

                    case DW_LNS_const_add_pc:
                        address += const_pc_add;
                        if (scan_only) {
                            if (ucomp(min_address, address) > 0)
                                min_address = address;
                            if (ucomp(max_address, address) < 0)
                                max_address = address;
                        }

                        fileLocationMap.add(new FileLocation(address, (String)fnames.get(fileno), lineno));

                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Advance PC by (constant) {0} to 0x{1}",
                                    new Object[]{new Integer(const_pc_add),
                                            Long.toHexString(address)});
                        break;

                    case DW_LNS_fixed_advance_pc: {
                        int amt = section.getShort() & 0xFFFF;
                        address += amt;

                        fileLocationMap.add(new FileLocation(address, (String)fnames.get(fileno), lineno));

                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Advance PC by (fixed) {0} to 0x{1}",
                                    new Object[]{new Integer(amt),
                                            Long.toHexString(address)});
                    }
                    break;

                    case DW_LNS_set_prologue_end: {
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Set prologue_end to true");
                    }
                    break;

                    case DW_LNS_set_epilogue_begin: {
                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Set epilogue_begin to true");
                    }
                    break;

                    case DW_LNS_set_isa: {
                        int amt = section.getShort() & 0xFFFF;

                        if (Configuration.DEBUG)
                            logger.log(DEBUG, "Set set_isa to {0}", new Object[]{new Integer(amt)});
                    }
                    break;
                }
            } else {
                int adj = (opcode & 0xFF) - header.opcode_base;
                int addr_adv = adj / header.line_range;
                int line_adv = header.line_base + (adj % header.line_range);
                long new_addr = address + addr_adv;
                int new_line = lineno + line_adv;
                if (Configuration.DEBUG)
                    logger.log(DEBUG,
                            "Special opcode {0} advance line by {1} to {2} and address by {3} to 0x{4}",
                            new Object[]{new Integer(opcode & 0xFF),
                                    new Integer(line_adv),
                                    new Integer(new_line),
                                    new Integer(addr_adv),
                                    Long.toHexString(new_addr)});
                if (!scan_only && base_address <= target && new_addr >= target) {
                    lineNumber = new_addr == target ? new_line : lineno;
                    sourceFile = (String) ((fileno >= 0 && fileno < fnames.size())
                            ? fnames.get(fileno) : define_file);
                    logger.log(DEBUG, "found {0}:{1} for {2}", new Object[]
                            {sourceFile, new Integer(lineNumber), Long.toHexString(target)});

                    cache.put(base_address, new_addr,
                            new CacheEntry(fnames, section, header));

                    return true;
                }

                prev_lineno = lineno;
                prev_fileno = fileno;
                lineno = new_line;
                address = new_addr;

                fileLocationMap.add(new FileLocation(address, (String)fnames.get(fileno), lineno));

                if (scan_only) {
                    if (ucomp(min_address, address) > 0)
                        min_address = address;
                    if (ucomp(max_address, address) < 0)
                        max_address = address;
                }
            }
        }

        if (scan_only) {
            if (min_address != -1 && max_address != 0)
                cache.put(min_address, max_address,
                        new CacheEntry(fnames, section, header));
        }
        return false;
    }

    private class CacheEntry {
        final dw2_debug_line header;
        final LinkedList fileNames;
        final ByteBuffer section;

        CacheEntry(LinkedList fileNames, ByteBuffer section, dw2_debug_line header) {
            this.fileNames = fileNames;
            this.section = section;
            this.header = header;
        }
    }

    public void close() {
        dw2 = null;
        binaryFile = null;
    }

    public String getSourceFile() {
        return sourceFile;
    }

    public int getLineNumber() {
        return lineNumber;
    }

    private static String getString(ByteBuffer buf) {
        int pos = buf.position();
        int len = 0;
        byte b;
        while (buf.get() != 0) len++;
        byte[] bytes = new byte[len];
        buf.position(pos);
        buf.get(bytes);
        buf.get();
        return new String(bytes);
    }

    private static long getUleb128(ByteBuffer buf) {
        long val = 0;
        byte b;
        int shift = 0;

        while (true) {
            b = buf.get();
            val |= (b & 0x7f) << shift;
            if ((b & 0x80) == 0)
                break;
            shift += 7;
        }

        return val;
    }

    private static long getSleb128(ByteBuffer buf) {
        long val = 0;
        int shift = 0;
        byte b;
        int size = 8 << 3;

        while (true) {
            b = buf.get();
            val |= (b & 0x7f) << shift;
            shift += 7;
            if ((b & 0x80) == 0)
                break;
        }

        if (shift < size && (b & 0x40) != 0)
            val |= -(1 << shift);

        return val;
    }

    static int ucomp(long l1, long l2) {
        if (l1 == l2)
            return 0;

        if (l1 < 0) {
            if (l2 < 0) {
                if (l1 < l2)
                    return 1;
                else
                    return -1;
            }
            return 1;
        } else {
            if (l2 >= 0) {
                if (l1 < l2)
                    return -1;
                else
                    return 1;
            }
            return -1;
        }
    }
}
