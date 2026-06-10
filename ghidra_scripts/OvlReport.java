// OvlReport.java — headless report on an analyzed AC1 overlay (no PyGhidra needed)
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.app.decompiler.*;

public class OvlReport extends GhidraScript {
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        println("OVLREPORT program=" + currentProgram.getName()
                + " function_count=" + fm.getFunctionCount());
        long entryVal = currentProgram.getMemory().getInt(toAddr(0x8004ada4)) & 0xffffffffL;
        println("OVLREPORT entry_ptr=0x" + Long.toHexString(entryVal));
        Function f = fm.getFunctionContaining(toAddr(entryVal));
        println("OVLREPORT entry_function=" + (f != null ? f.getName() : "none"));
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        if (f != null) {
            DecompileResults r = di.decompileFunction(f, 60, monitor);
            if (r != null && r.getDecompiledFunction() != null) {
                println("OVLREPORT_DECOMPILE_BEGIN");
                println(r.getDecompiledFunction().getC());
                println("OVLREPORT_DECOMPILE_END");
            }
        }
    }
}
