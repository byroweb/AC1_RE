// MemMap.java — print the memory block layout of the current program so a
// fresh project can be rebuilt to match (names, bases, overlay spaces).
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.mem.*;

public class MemMap extends GhidraScript {
    public void run() throws Exception {
        println("MEMMAP program=" + currentProgram.getName()
                + " path=" + currentProgram.getDomainFile().getPathname());
        println("MEMMAP ghidra_created_with=" + currentProgram.getMetadata().get("Created With Ghidra Version"));
        for (MemoryBlock b : currentProgram.getMemory().getBlocks()) {
            boolean ov = b.getStart().getAddressSpace().isOverlaySpace();
            println("MEMMAP block=" + b.getName()
                    + " start=" + b.getStart()
                    + " end=" + b.getEnd()
                    + " len=0x" + Long.toHexString(b.getSize())
                    + " init=" + b.isInitialized()
                    + " overlay=" + ov
                    + " perm=" + (b.isRead()?"r":"-") + (b.isWrite()?"w":"-") + (b.isExecute()?"x":"-"));
        }
    }
}
