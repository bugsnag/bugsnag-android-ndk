package dwarf2;

/**
 * Class to store a location in a file
 */
public class FileLocation {

    long address;
    String filename;
    int lineNumber;

    public FileLocation(long address, String filename, int lineNumber) {
        this.address = address;
        this.filename = filename;
        this.lineNumber = lineNumber;
    }
}
