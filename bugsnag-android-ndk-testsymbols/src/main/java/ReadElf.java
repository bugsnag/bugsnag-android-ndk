/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * A poor man's implementation of the readelf command. This program is designed
 * to parse ELF (Executable and Linkable Format) files.
 */
public class ReadElf implements AutoCloseable {
    /** The magic values for the ELF identification. */
    private static final byte[] ELFMAG = {
            (byte) 0x7F, (byte) 'E', (byte) 'L', (byte) 'F', };
    private static final int EI_NIDENT = 16;
    private static final int EI_CLASS = 4;
    private static final int EI_DATA = 5;
    private static final int EM_386 = 3;
    private static final int EM_MIPS = 8;
    private static final int EM_ARM = 40;
    private static final int EM_X86_64 = 62;
    // http://en.wikipedia.org/wiki/Qualcomm_Hexagon
    private static final int EM_QDSP6 = 164;
    private static final int EM_AARCH64 = 183;
    private static final int ELFCLASS32 = 1;
    private static final int ELFCLASS64 = 2;
    private static final int ELFDATA2LSB = 1;
    private static final int ELFDATA2MSB = 2;
    private static final int EV_CURRENT = 1;
    private static final long PT_LOAD = 1;
    private static final int SHT_SYMTAB = 2;
    private static final int SHT_STRTAB = 3;
    private static final int SHT_DYNAMIC = 6;
    private static final int SHT_DYNSYM = 11;

    private final String mPath;
    private final RandomAccessFile mFile;
    private final byte[] mBuffer = new byte[512];
    private int mEndian;
    private boolean mIsDynamic;
    private boolean mIsPIE;
    private int mType;
    private int mAddrSize;

    /** Symbol Table offset */
    private SectionHeader mSymTabHeader;

    /** Dynamic Symbol Table offset */
    private SectionHeader mDynSymHeader;

    /** Section Header String Table offset */
    private SectionHeader mShStrTabHeader;

    /** String Table offset */
    private SectionHeader mStrTabHeader;

    /** Dynamic String Table offset */
    private SectionHeader mDynStrHeader;

    /** Symbol Table symbol names */
    private List<Symbol> mSymbols;

    /** Dynamic Symbol Table symbol names */
    private List<Symbol> mDynamicSymbols;

    List<SectionHeader> mSectionHeaders;

    public static ReadElf read(File file) throws IOException {
        return new ReadElf(file);
    }

    public boolean isDynamic() {
        return mIsDynamic;
    }
    public int getType() {
        return mType;
    }
    public boolean isPIE() {
        return mIsPIE;
    }
    public ReadElf(File file) throws IOException {
        mPath = file.getPath();
        mFile = new RandomAccessFile(file, "r");
        if (mFile.length() < EI_NIDENT) {
            throw new IllegalArgumentException("Too small to be an ELF file: " + file);
        }
        readHeader();
    }
    public void close() {
        try {
            mFile.close();
        } catch (IOException ignored) {
        }
    }
    @Override
    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }
    private void readHeader() throws IOException {
        mFile.seek(0);
        mFile.readFully(mBuffer, 0, EI_NIDENT);
        if (mBuffer[0] != ELFMAG[0] || mBuffer[1] != ELFMAG[1] ||
                mBuffer[2] != ELFMAG[2] || mBuffer[3] != ELFMAG[3]) {
            throw new IllegalArgumentException("Invalid ELF file: " + mPath);
        }
        int elfClass = mBuffer[EI_CLASS];
        if (elfClass == ELFCLASS32) {
            mAddrSize = 4;
        } else if (elfClass == ELFCLASS64) {
            mAddrSize = 8;
        } else {
            throw new IOException("Invalid ELF EI_CLASS: " + elfClass + ": " + mPath);
        }
        mEndian = mBuffer[EI_DATA];
        if (mEndian == ELFDATA2LSB) {
        } else if (mEndian == ELFDATA2MSB) {
            throw new IOException("Unsupported ELFDATA2MSB file: " + mPath);
        } else {
            throw new IOException("Invalid ELF EI_DATA: " + mEndian + ": " + mPath);
        }
        mType = readHalf();
        int e_machine = readHalf();
        if (e_machine != EM_386 && e_machine != EM_X86_64 &&
                e_machine != EM_AARCH64 && e_machine != EM_ARM &&
                e_machine != EM_MIPS &&
                e_machine != EM_QDSP6) {
            throw new IOException("Invalid ELF e_machine: " + e_machine + ": " + mPath);
        }
        // AbiTest relies on us rejecting any unsupported combinations.
        if ((e_machine == EM_386 && elfClass != ELFCLASS32) ||
                (e_machine == EM_X86_64 && elfClass != ELFCLASS64) ||
                (e_machine == EM_AARCH64 && elfClass != ELFCLASS64) ||
                (e_machine == EM_ARM && elfClass != ELFCLASS32) ||
                (e_machine == EM_QDSP6 && elfClass != ELFCLASS32)) {
            throw new IOException("Invalid e_machine/EI_CLASS ELF combination: " +
                    e_machine + "/" + elfClass + ": " + mPath);
        }
        long e_version = readWord();
        if (e_version != EV_CURRENT) {
            throw new IOException("Invalid e_version: " + e_version + ": " + mPath);
        }
        long e_entry = readAddr();
        long ph_off = readOff();
        long sh_off = readOff();
        long e_flags = readWord();
        int e_ehsize = readHalf();
        int e_phentsize = readHalf();
        int e_phnum = readHalf();
        int e_shentsize = readHalf();
        int e_shnum = readHalf();
        int e_shstrndx = readHalf();
        readSectionHeaders(sh_off, e_shnum, e_shentsize, e_shstrndx);
        readProgramHeaders(ph_off, e_phnum, e_phentsize);
    }
    private void readSectionHeaders(long sh_off, int e_shnum, int e_shentsize, int e_shstrndx)
            throws IOException {

        // Read the Section Header String Table offset first.
        {
            mFile.seek(sh_off + e_shstrndx * e_shentsize);
            long sh_name = readWord();
            long sh_type = readWord();
            long sh_flags = readX(mAddrSize);
            long sh_addr = readAddr();
            long sh_offset = readOff();
            long sh_size = readX(mAddrSize);
            // ...
            if (sh_type == SHT_STRTAB) {
                mShStrTabHeader = new SectionHeader("", sh_type, sh_flags, sh_addr, sh_offset, sh_size);
            }
        }

        mSectionHeaders = new ArrayList<SectionHeader>();
        for (int i = 0; i < e_shnum; ++i) {
            // Don't bother to re-read the Section Header StrTab.
            if (i == e_shstrndx) {
                continue;
            }
            mFile.seek(sh_off + i * e_shentsize);
            long sh_name = readWord();
            long sh_type = readWord();
            long sh_flags = readX(mAddrSize);
            long sh_addr = readAddr();
            long sh_offset = readOff();
            long sh_size = readX(mAddrSize);

            String name = readShStrTabEntry(sh_name);
            SectionHeader header = new SectionHeader(name, sh_type, sh_flags, sh_addr, sh_offset, sh_size);

            if (header.sh_type == SHT_SYMTAB || header.sh_type == SHT_DYNSYM) {
                if (".symtab".equals(header.sh_name)) {
                    mSymTabHeader = header;
                } else if (".dynsym".equals(header.sh_name)) {
                    mDynSymHeader = header;
                }
            } else if (sh_type == SHT_STRTAB) {
                if (".strtab".equals(header.sh_name)) {
                    mStrTabHeader = header;
                } else if (".dynstr".equals(header.sh_name)) {
                    mDynStrHeader = header;
                }
            } else if (sh_type == SHT_DYNAMIC) {
                mIsDynamic = true;
            }

            mSectionHeaders.add(header);
        }
    }
    private void readProgramHeaders(long ph_off, int e_phnum, int e_phentsize) throws IOException {
        for (int i = 0; i < e_phnum; ++i) {
            mFile.seek(ph_off + i * e_phentsize);
            long p_type = readWord();
            if (p_type == PT_LOAD) {
                if (mAddrSize == 8) {
                    // Only in Elf64_phdr; in Elf32_phdr p_flags is at the end.
                    long p_flags = readWord();
                }
                long p_offset = readOff();
                long p_vaddr = readAddr();
                // ...
                if (p_vaddr == 0) {
                    mIsPIE = true;
                }
            }
        }
    }
    private List<Symbol> readSymbolTable(SectionHeader symStrHeader, SectionHeader tableHeader) throws IOException {
        List<Symbol> result = new ArrayList<Symbol>();
        mFile.seek(tableHeader.sh_offset);
        while (mFile.getFilePointer() < tableHeader.sh_offset + tableHeader.sh_size) {
            long st_name = readWord();
            int st_info;
            long st_value;
            int st_other;
            int st_shndx;
            long st_size;
            if (mAddrSize == 8) {
                st_info = readByte();
                st_other = readByte();
                st_shndx = readHalf();
                st_value = readAddr();
                st_size = readX(mAddrSize);
            } else {
                st_value = readAddr();
                st_size = readWord();
                st_info = readByte();
                st_other = readByte();
                st_shndx = readHalf();
            }
            if (st_name == 0) {
                continue;
            }
            final String symName = readStrTabEntry(symStrHeader.sh_offset, symStrHeader.sh_size, st_name);

            if (symName != null) {
                Symbol s = new Symbol(symName, st_info, st_value, st_other, st_shndx, st_size);
                result.add(s);
            }
        }
        return result;
    }
    private String readShStrTabEntry(long strOffset) throws IOException {
        if (mShStrTabHeader.sh_offset == 0 || strOffset < 0 || strOffset >= mShStrTabHeader.sh_size) {
            return null;
        }
        return readString(mShStrTabHeader.sh_offset + strOffset);
    }
    private String readStrTabEntry(long tableOffset, long tableSize, long strOffset)
            throws IOException {
        if (tableOffset == 0 || strOffset < 0 || strOffset >= tableSize) {
            return null;
        }
        return readString(tableOffset + strOffset);
    }
    private int readHalf() throws IOException {
        return (int) readX(2);
    }
    private long readWord() throws IOException {
        return readX(4);
    }
    private long readOff() throws IOException {
        return readX(mAddrSize);
    }
    private long readAddr() throws IOException {
        return readX(mAddrSize);
    }
    private long readX(int byteCount) throws IOException {
        mFile.readFully(mBuffer, 0, byteCount);
        int answer = 0;
        if (mEndian == ELFDATA2LSB) {
            for (int i = byteCount - 1; i >= 0; i--) {
                answer = (answer << 8) | (mBuffer[i] & 0xff);
            }
        } else {
            final int N = byteCount - 1;
            for (int i = 0; i <= N; ++i) {
                answer = (answer << 8) | (mBuffer[i] & 0xff);
            }
        }
        return answer;
    }
    private String readString(long offset) throws IOException {
        long originalOffset = mFile.getFilePointer();
        mFile.seek(offset);
        mFile.readFully(mBuffer, 0, (int) Math.min(mBuffer.length, mFile.length() - offset));
        mFile.seek(originalOffset);
        for (int i = 0; i < mBuffer.length; ++i) {
            if (mBuffer[i] == 0) {
                return new String(mBuffer, 0, i);
            }
        }
        return null;
    }
    private int readByte() throws IOException {
        return mFile.read() & 0xff;
    }
//    public Symbol getSymbol(String name) {
//        if (mSymbols == null) {
//            getSymbols();
//        }
//        return mSymbols.get(name);
//    }

    public List<Symbol> getSymbols() {
        if (mSymbols == null) {
            try {
                mSymbols = readSymbolTable(mStrTabHeader, mSymTabHeader);

                mSymbols.sort(new Comparator<Symbol>() {
                    public int compare(Symbol obj1, Symbol obj2) {
                        return Long.compare(obj1.st_value, obj2.st_value);
                    }
                });
            } catch (IOException e) {
                return null;
            }
        }

        return mSymbols;
    }


//    public Symbol getDynamicSymbol(String name) {
//        if (mDynamicSymbols == null) {
//            try {
//                mDynamicSymbols = readSymbolTable(
//                        mDynStrOffset, mDynStrSize, mDynSymOffset, mDynSymSize);
//            } catch (IOException e) {
//                return null;
//            }
//        }
//        return mDynamicSymbols.get(name);
//    }

    public List<Symbol> getDynamicSymbols() {
        if (mSymbols == null) {
            try {
                mDynamicSymbols = readSymbolTable(mDynStrHeader, mDynSymHeader);

                mDynamicSymbols.sort(new Comparator<Symbol>() {
                    public int compare(Symbol obj1, Symbol obj2) {
                        return Long.compare(obj1.st_value, obj2.st_value);
                    }
                });
            } catch (IOException e) {
                return null;
            }
        }

        return mDynamicSymbols;
    }
}