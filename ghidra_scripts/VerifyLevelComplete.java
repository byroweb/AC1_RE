// VerifyLevelComplete.java - read-only dump of the level-complete annotations.
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Symbol;

public class VerifyLevelComplete extends GhidraScript {
    long[] addrs = {0x8008A0B0L,0x8008A1F4L,0x8008AB68L,0x8008A8F8L,0x8004C318L};
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing lst = currentProgram.getListing();
        println("=== MEMORY BLOCKS ===");
        for (MemoryBlock b : currentProgram.getMemory().getBlocks())
            println("  " + b.getName() + " " + b.getStart() + ".." + b.getEnd()
                + " init=" + b.isInitialized());
        println("=== TARGETS ===");
        for (long a : addrs) {
            Address ad = toAddr(a);
            Function f = fm.getFunctionAt(ad);
            Symbol[] syms = currentProgram.getSymbolTable().getSymbols(ad);
            StringBuilder sb = new StringBuilder();
            for (Symbol s : syms) sb.append(s.getName()).append(" ");
            String pc = lst.getComment(CodeUnit.PLATE_COMMENT, ad);
            String prc = lst.getComment(CodeUnit.PRE_COMMENT, ad);
            println(String.format("0x%08X func=%s syms=[%s]", a,
                (f==null?"-":f.getName()+"("+f.getBody().getNumAddresses()+"b)"),
                sb.toString().trim()));
            if (pc!=null)  println("   PLATE: " + pc.substring(0,Math.min(70,pc.length())) + "...");
            if (prc!=null) println("   PRE:   " + prc.substring(0,Math.min(70,prc.length())) + "...");
        }
    }
}
