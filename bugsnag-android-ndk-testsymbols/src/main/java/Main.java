import com.bicirikdwarf.dwarf.CompilationUnit;
import com.bicirikdwarf.dwarf.DebugInfoEntry;
import com.bicirikdwarf.dwarf.DebugLineEntry;
import com.bicirikdwarf.dwarf.DwAtType;
import com.bicirikdwarf.dwarf.Dwarf32Context;
import com.bicirikdwarf.elf.Elf32Context;
import com.bicirikdwarf.elf.ElfSymbol;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.util.Collection;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

class Main {

    private static class SharedObjectFilter implements FilenameFilter {

        @Override
        public boolean accept(File dir, String name) {
            return name.endsWith(".so");
        }
    }



    public static void main(String args[]) {

        File directory = new File("../bugsnag-android-ndk-test/app/build/intermediates/cmake/release/obj/");
        //File directory = new File("../bugsnag-android-ndk-test/app/build/intermediates/binaries/release/obj/");
        //File f = new File("../bugsnag-android-ndk-test/app/build/intermediates/binaries/release/obj/armeabi/libjni-entry-point.so");
        //File f = new File("../bugsnag-android-ndk-test/app/build/intermediates/binaries/release/obj/armeabi-v7a/libjni-entry-point.so");
        //File f = new File("../bugsnag-android-ndk-test/app/build/intermediates/binaries/release/obj/mips/libjni-entry-point.so");
        //File f = new File("../bugsnag-android-ndk-test/app/build/intermediates/binaries/release/obj/x86/libjni-entry-point.so");


        for (File archDir : directory.listFiles()) {
            if (archDir.isDirectory()) {
                String arch = archDir.getName();

                for (File sharedObject : archDir.listFiles(new SharedObjectFilter())) {

                    System.out.println("");
                    System.out.println(" ------------------------ Symbols in " + sharedObject.getAbsoluteFile());
                    try {
                        List<BugsnagSoSymbol> symbols = getSymbols(sharedObject);
                        System.out.println("arch = " + arch + "  count = " + symbols.size());

                        for (BugsnagSoSymbol symbol : symbols) {
                            System.out.println(symbol.getAddress() //+ "/0x" + Long.toHexString(symbol.st_value)
                                    + " " + symbol.getMethodName()
                                    + " " + symbol.getFilename()
                                    + " " + symbol.getLineNumber());
                        }

                    } catch (Exception e) {
                        System.out.println("arch = " + arch + "  failed to generate symbols = " + e.getMessage());
                        e.printStackTrace();
                    }
                }

                return;
            }
        }
    }

    private static List<BugsnagSoSymbol> getSymbols(File f) throws IOException {
        RandomAccessFile aFile = new RandomAccessFile(f, "r");
        FileChannel inChannel = aFile.getChannel();
        long fileSize = inChannel.size();
        ByteBuffer buffer = ByteBuffer.allocate((int) fileSize);
        inChannel.read(buffer);
        buffer.flip();
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        inChannel.close();
        aFile.close();

        Elf32Context elf = new Elf32Context(buffer);
        Dwarf32Context dwarf = new Dwarf32Context(elf);


        Map<Long, BugsnagSoSymbol> symbols = new HashMap<>();

        // Remove all ARM ELF special symbols from the list
        // $a - At the start of a region of code containing ARM instructions.
        // $t - At the start of a region of code containing THUMB instructions.
        // $d - At the start of a region of data.
        int index = 0;
        while (index < elf.getSymbols().size()) {
            ElfSymbol current = elf.getSymbols().get(index);

            if (current.symbol_name.equals("$a")
                    || current.symbol_name.equals("$t")
                    || current.symbol_name.equals("$d")) {
                elf.getSymbols().remove(index);
            } else {
                index++;
            }
        }

        // Create shared object symbols for all the elf symbols
        for (ElfSymbol symbol : elf.getSymbols()) {
            BugsnagSoSymbol so = new BugsnagSoSymbol(symbol);
            findCompileUnitInformation(dwarf.getCompilationUnits(), so);

            if (so.getMethodName() != null) {
                symbol.symbol_name = so.getMethodName();
            }
            symbols.put(so.getAddress(), so);
        }

        // Also add shared object symbols for all the debug line entries
        for (DebugLineEntry entry : dwarf.getDebugLineEntries()) {
            BugsnagSoSymbol so = new BugsnagSoSymbol(entry);

            if (symbols.containsKey(so.getAddress())) {
                BugsnagSoSymbol existingEntry = symbols.get(so.getAddress());

                existingEntry.setFilename(entry.getFilename());

                // Choose the largest line number in the case where there are two conflicting entries
                // This can happen if the code has been optimised, and the largest line number seems more useful
                if (existingEntry.getLineNumber() < so.getLineNumber()) {
                    symbols.get(so.getAddress()).setLineNumber(entry.getLineNumber());
                }
            } else {
                findMethodName(elf.getSymbols(), dwarf.getCompilationUnits(), so);
                symbols.put(so.getAddress(), so);
            }
        }

        // Tweak symbols that have not quite been populated fully
        for (BugsnagSoSymbol symbol : symbols.values()) {

            // Check the following address for anything with a missing method name
            if (symbol.getMethodName() == null
                    && symbol.getFilename() != null
                    && symbol.getLineNumber() != 0) {
                if (symbols.containsKey(symbol.getAddress() + 1)
                        && symbols.get(symbol.getAddress() + 1).getMethodName() != null) {
                    symbol.setMethodName(symbols.get(symbol.getAddress() + 1).getMethodName());
                }
            }

            // Check the previous address for anything with a missing line number
            if (symbol.getLineNumber() == 0
                    && symbol.getFilename() != null
                    && symbol.getMethodName() != null) {
                if (symbols.containsKey(symbol.getAddress() - 1)
                        && symbols.get(symbol.getAddress() - 1).getLineNumber() != 0) {
                    symbol.setLineNumber(symbols.get(symbol.getAddress() - 1).getLineNumber());
                }
            }
        }

        List<BugsnagSoSymbol> output = symbols.values().stream().collect(Collectors.toList());

        // Sort the debug Entries by Address
        output.sort(new Comparator<BugsnagSoSymbol>() {
            public int compare(BugsnagSoSymbol obj1, BugsnagSoSymbol obj2) {
                return Long.compare(obj1.getAddress(), obj2.getAddress());
            }
        });


        // Remove any duplicate entries
//        index = 0;
//        while (index < output.size() - 1) {
//            BugsnagSoSymbol current = output.get(index);
//            BugsnagSoSymbol next = output.get(index + 1);
//
//            if (current.getMethodName().equals(next.getMethodName())
//                    && current.getFilename().equals(next.getFilename())
//                    && current.getLineNumber() == next.getLineNumber()) {
//                output.remove(index + 1);
//            } else {
//                index++;
//            }
//        }

        System.out.println("build note = " + elf.getBuildNote());

        return output;
    }

    private static void findMethodName(List<ElfSymbol> elfSymbols, Collection<CompilationUnit> compileUnits, BugsnagSoSymbol so) {
        for (ElfSymbol symbol : elfSymbols) {

            if (symbol.st_value <= so.getAddress()
                    && symbol.st_value + symbol.st_size > so.getAddress()) {

                so.setMethodName(symbol.symbol_name);
                return;
            }
        }

        for (CompilationUnit unit : compileUnits) {
            for (DebugInfoEntry die : unit.getCompileUnit().getChildren()) {
                if (die.getAttribValue(DwAtType.DW_AT_low_pc) != null
                        && (so.getAddress() == (int) die.getAttribValue(DwAtType.DW_AT_low_pc))) {
                    if (die.getAttribValue(DwAtType.DW_AT_name) != null) {
                        so.setMethodName((String)die.getAttribValue(DwAtType.DW_AT_name));
                        return;
                    }
                }
            }
        }
    }

    private static void findCompileUnitInformation(Collection<CompilationUnit> compileUnits, BugsnagSoSymbol so) {
        // Look for a debug info entry with matching pointer
        for (CompilationUnit unit : compileUnits) {
            for (DebugInfoEntry die : unit.getCompileUnit().getChildren()) {
                if (die.getAttribValue(DwAtType.DW_AT_low_pc) != null
                        && (so.getAddress() == (int)die.getAttribValue(DwAtType.DW_AT_low_pc)
                        || so.getAddress() - 1 == (int)die.getAttribValue(DwAtType.DW_AT_low_pc))) { // HACK: arm files seem to have 1 byte difference?

                    if (unit.getCompileUnit().getAttribValue(DwAtType.DW_AT_name) != null) {
                        so.setFilename((String)unit.getCompileUnit().getAttribValue(DwAtType.DW_AT_name));
                    }

                    if (die.getAttribValue(DwAtType.DW_AT_decl_line) != null) {
                        so.setLineNumber(Integer.valueOf(die.getAttribValue(DwAtType.DW_AT_decl_line).toString()));
                    }

                    return;
                }
            }
        }
    }

}