// AddRawBlockAnalyze.java — materialize a raw binary file as initialized,
// executable bytes at a fixed base, then auto-analyze that range. Handles the
// common case where the base falls inside an existing *uninitialized* block
// (the PSX loader maps one big RAM block over 0x80050000): it splits that block
// and converts the middle to initialized rather than creating an overlapping
// one. Used to add the AC1 front_end runtime image (0x80050000, verified
// pristine vs disc) to the pristine project before replaying markup.
//
// Args: <bin> <baseHex> <blockName>
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.mem.*;
import ghidra.app.plugin.core.analysis.AutoAnalysisManager;
import java.io.File;
import java.io.FileInputStream;
import java.nio.file.Files;

public class AddRawBlockAnalyze extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        if (a.length < 3) { println("ADDBLOCK usage: <bin> <baseHex> <name>"); return; }
        File f = new File(a[0]);
        long base = Long.decode(a[1]) & 0xffffffffL;
        long len  = f.length();
        String name = a[2];
        Memory mem = currentProgram.getMemory();
        if (mem.getBlock(name) != null) { println("ADDBLOCK block '" + name + "' already present"); return; }
        Address start = toAddr(base);
        Address end   = toAddr(base + len);            // one past last byte

        MemoryBlock cont = mem.getBlock(start);
        MemoryBlock target;
        if (cont == null) {                            // nothing here — create fresh
            FileInputStream fis = new FileInputStream(f);
            try { target = mem.createInitializedBlock(name, start, fis, len, monitor, false); }
            finally { fis.close(); }
        } else {                                       // carve out of the existing block
            if (cont.isInitialized())
                throw new Exception("range already initialized in block " + cont.getName());
            if (!cont.getStart().equals(start)) mem.split(cont, start);     // [.., start) | [start, ..]
            MemoryBlock mid = mem.getBlock(start);
            if (mid.getEnd().compareTo(end) >= 0 && !mid.getStart().equals(end))
                mem.split(mid, end);                                        // [start, end) | [end, ..]
            target = mem.getBlock(start);
            mem.convertToInitialized(target, (byte) 0);
            mem.setBytes(start, Files.readAllBytes(f.toPath()));
            target.setName(name);
        }
        target.setRead(true); target.setWrite(true); target.setExecute(true);
        println("ADDBLOCK '" + name + "' @ " + target.getStart() + ".." + target.getEnd()
                + " len=0x" + Long.toHexString(len) + " init=" + target.isInitialized());

        AddressSet set = new AddressSet(target.getStart(), target.getEnd());
        AutoAnalysisManager mgr = AutoAnalysisManager.getAnalysisManager(currentProgram);
        mgr.reAnalyzeAll(set);
        mgr.startAnalysis(monitor);
        println("ADDBLOCK analyzed " + set);
    }
}
