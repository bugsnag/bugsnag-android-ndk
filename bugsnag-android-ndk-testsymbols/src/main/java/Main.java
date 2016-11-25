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

        cleanElfSymbols(elf);

        Map<Long, BugsnagSoSymbol> symbols = new HashMap<>();

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

    private static void cleanElfSymbols(Elf32Context elf) {

        // Sort the elf Entries by Address
        elf.getSymbols().sort(new Comparator<ElfSymbol>() {
            public int compare(ElfSymbol obj1, ElfSymbol obj2) {
                return Long.compare(obj1.st_value, obj2.st_value);
            }
        });

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

                // HACK: It seems that these special symbols make the function offsets out of line with the debug info
                // Check to see if there is another symbol in the next byte, and set the address to this symbols
                // address to bring it in line with the other symbols for matching later
                if (elf.getSymbols().size() > index + 1
                        && elf.getSymbols().get(index + 1).st_value == current.st_value + 1) {
                    elf.getSymbols().get(index + 1).st_value = current.st_value;
                }

                elf.getSymbols().remove(index);
            } else {
                index++;
            }
        }
    }

    private static void findMethodName(List<ElfSymbol> elfSymbols, Collection<CompilationUnit> compileUnits, BugsnagSoSymbol so) {
        for (ElfSymbol symbol : elfSymbols) {

            if (symbol.st_value <= so.getAddress()
                    && symbol.st_value + symbol.st_size > so.getAddress()) {

                so.setMethodName(symbol.symbol_name);
                return;
            }
        }
    }

    private static void findCompileUnitInformation(Collection<CompilationUnit> compileUnits, BugsnagSoSymbol so) {
        // Look for a debug info entry with matching pointer
        for (CompilationUnit unit : compileUnits) {
            for (DebugInfoEntry die : unit.getCompileUnit().getChildren()) {
                if (die.getAttribValue(DwAtType.DW_AT_low_pc) != null
                        && so.getAddress() == (int)die.getAttribValue(DwAtType.DW_AT_low_pc)) {

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