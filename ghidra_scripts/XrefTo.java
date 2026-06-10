// XrefTo.java — list code references to given address(es) in an analyzed overlay.
// Args: one or more hex addresses. Reports from-addr, ref type, containing fn.
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;

public class XrefTo extends GhidraScript {
    public void run() throws Exception {
        ReferenceManager rm = currentProgram.getReferenceManager();
        FunctionManager fm = currentProgram.getFunctionManager();
        for (String a : getScriptArgs()) {
            Address t = toAddr(Long.decode(a) & 0xffffffffL);
            println("XREFS_TO " + a);
            ReferenceIterator ri = rm.getReferencesTo(t);
            int n = 0;
            while (ri.hasNext()) {
                Reference r = ri.next();
                Address from = r.getFromAddress();
                Function f = fm.getFunctionContaining(from);
                println("  " + from + "  " + r.getReferenceType()
                        + "  in " + (f != null ? f.getName() : "?"));
                n++;
            }
            println("XREFS_END " + a + " count=" + n);
        }
    }
}
