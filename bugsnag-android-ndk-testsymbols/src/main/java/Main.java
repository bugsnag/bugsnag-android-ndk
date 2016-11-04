import com.bicirikdwarf.dwarf.CompilationUnit;
import com.bicirikdwarf.dwarf.DebugInfoEntry;
import com.bicirikdwarf.dwarf.DwAtType;
import com.bicirikdwarf.dwarf.Dwarf32Context;
import com.bicirikdwarf.elf.Elf32Context;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.util.List;

class Main {
    public static void main(String args[]) throws IOException {
       File f = new File("../bugsnag-android-ndk-test/app/build/intermediates/binaries/release/obj/x86/libjni-entry-point.so");

        ReadElf reader = new ReadElf(f);
//        for (SectionHeader header : reader.mSectionHeaders) {
//            System.out.println(header.sh_name + " - "
//                    + header.sh_type
//                    + " , " + header.sh_flags
//                    + " , " + header.sh_addr
//                    + " , " + Long.toHexString(header.sh_offset)
//                    + " , " + header.sh_size);
//        }

        List<Symbol> symbols = reader.getSymbols();
        reader.close();


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

//       for (CompilationUnit unit : dwarf.getCompilationUnits()) {
//            System.out.println(unit.getCompileUnit().getAttribValue(DwAtType.DW_AT_name));
//
//            for (DebugInfoEntry die : unit.getCompileUnit().getChildren()) {
//                if (die.getAttribValue(DwAtType.DW_AT_name) != null && die.getAttribValue(DwAtType.DW_AT_low_pc) != null) {
//                    System.out.print("  " + die.getAttribValue(DwAtType.DW_AT_name) +
//                            "  address=" + die.getAttribValue(DwAtType.DW_AT_low_pc) +
//                            "  lineno=" + die.getAttribValue(DwAtType.DW_AT_decl_line)
//                    );
//
//                    System.out.println();
//
//                }
//            }
//
//        }


        for (Symbol symbol : symbols) {
            for (CompilationUnit unit : dwarf.getCompilationUnits()) {
                for (DebugInfoEntry die : unit.getCompileUnit().getChildren()) {
                    if (die.getAttribValue(DwAtType.DW_AT_low_pc) != null
                            && symbol.st_value == (int)die.getAttribValue(DwAtType.DW_AT_low_pc)) {
                        symbol.filename = (String)unit.getCompileUnit().getAttribValue(DwAtType.DW_AT_name);
                        symbol.line_number = (int)die.getAttribValue(DwAtType.DW_AT_decl_line);

                    }
                }
            }

            System.out.println(symbol.st_value + "/" + Long.toHexString(symbol.st_value)
                    + " - " + symbol.name
                    + " - " + symbol.filename
                    + " - " + symbol.line_number);
        }


    }
}