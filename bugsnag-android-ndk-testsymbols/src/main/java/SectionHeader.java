/**
 * Used to store section header information
 */
public class SectionHeader {

    String sh_name;
    long sh_type;
    long sh_flags;
    long sh_addr;
    long sh_offset;
    long sh_size;

    public SectionHeader(String sh_name, long sh_type, long sh_flags, long sh_addr, long sh_offset, long sh_size) {
        this.sh_name = sh_name;
        this.sh_type = sh_type;
        this.sh_flags = sh_flags;
        this.sh_addr = sh_addr;
        this.sh_offset = sh_offset;
        this.sh_size = sh_size;
    }


}
