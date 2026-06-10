// DecompAt.java — decompile function(s) at the given hex address(es) in an
// analyzed overlay program. Args: one or more hex addresses (e.g. 0x8008ab68).
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.app.decompiler.*;

public class DecompAt extends GhidraScript {
    public void run() throws Exception {
        String[] args = getScriptArgs();
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String a : args) {
            long v = Long.decode(a) & 0xffffffffL;
            Address ad = toAddr(v);
            Function f = fm.getFunctionContaining(ad);
            if (f == null) f = createFunction(ad, null);   // force one if analysis missed it
            println("DECOMP_BEGIN " + a + " -> " + (f != null ? f.getName() : "none"));
            if (f != null) {
                DecompileResults r = di.decompileFunction(f, 90, monitor);
                if (r != null && r.getDecompiledFunction() != null)
                    println(r.getDecompiledFunction().getC());
            }
            println("DECOMP_END " + a);
        }
    }
}
