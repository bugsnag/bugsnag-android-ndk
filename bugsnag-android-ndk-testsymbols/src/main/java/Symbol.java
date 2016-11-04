/**
 * Class to store symbols in
 */
public class Symbol {
    private static final int STB_LOCAL = 0;
    private static final int STB_GLOBAL = 1;
    private static final int STB_WEAK = 2;
    private static final int STB_LOPROC = 13;
    private static final int STB_HIPROC = 15;
    private static final int STT_NOTYPE = 0;
    private static final int STT_OBJECT = 1;
    private static final int STT_FUNC = 2;
    private static final int STT_SECTION = 3;
    private static final int STT_FILE = 4;
    private static final int STT_COMMON = 5;
    private static final int STT_TLS = 6;

    public final String name;
    public final int bind;
    public final int type;
    public final long st_value;
    public final int st_other;
    public final int st_shndx;
    public final long st_size;

    public String filename;
    public int line_number;

    Symbol(String name, int st_info, long st_value, int st_other, int st_shndx, long st_size) {
        this.name = name;
        this.bind = (st_info >> 4) & 0x0F;
        this.type = st_info & 0x0F;
        this.st_value = st_value;
        this.st_other = st_other;
        this.st_shndx = st_shndx;
        this.st_size = st_size;
    }

    @Override
    public String toString() {
        return "Symbol[" + name + "," + toBind() + "," + toType() + "]";
    }

    private String toBind() {
        switch (bind) {
            case STB_LOCAL:
                return "LOCAL";
            case STB_GLOBAL:
                return "GLOBAL";
            case STB_WEAK:
                return "WEAK";
        }
        return "STB_??? (" + bind + ")";
    }

    private String toType() {
        switch (type) {
            case STT_NOTYPE:
                return "NOTYPE";
            case STT_OBJECT:
                return "OBJECT";
            case STT_FUNC:
                return "FUNC";
            case STT_SECTION:
                return "SECTION";
            case STT_FILE:
                return "FILE";
            case STT_COMMON:
                return "COMMON";
            case STT_TLS:
                return "TLS";
        }
        return "STT_??? (" + type + ")";
    }
}
