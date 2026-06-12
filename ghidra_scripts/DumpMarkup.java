// DumpMarkup.java — export RE markup (function names, prototypes, calling
// conventions, all listing comments, and user labels) from an analyzed AC1
// program to a TSV file, for replay into a fresh pristine program with
// ApplyMarkup.java. Bytes are identical between the FARSI and pristine builds,
// so transfer is 1:1 by address.
//
// Args: <out.tsv>
// Record forms (tab-separated, fields escaped for \\ \t \n). <addr> is the full
// Ghidra address string (incl. overlay space, e.g. "ovl::80050000") so markup
// in overlay blocks round-trips:
//   F <addr> <name> <callconv> <prototype>   non-default function
//   C <addr> <commentType> <text>            listing comment (0=EOL,1=PRE,2=POST,3=PLATE,4=REPEAT)
//   L <addr> <name>                          user-defined label
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.*;
import ghidra.program.model.symbol.*;
import java.io.PrintWriter;

public class DumpMarkup extends GhidraScript {
    static String esc(String s) {
        if (s == null) return "";
        return s.replace("\\", "\\\\").replace("\t", "\\t").replace("\r", "").replace("\n", "\\n");
    }
    static boolean isDefaultFn(String n) { return n.startsWith("FUN_") || n.startsWith("thunk_FUN_"); }
    static boolean isDefaultLbl(String n) {
        return n.startsWith("DAT_") || n.startsWith("LAB_") || n.startsWith("SUB_")
            || n.startsWith("UNK_") || n.startsWith("EXT_") || n.startsWith("s_") || n.startsWith("u_");
    }

    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) { println("DUMPMARKUP usage: <out.tsv>"); return; }
        PrintWriter w = new PrintWriter(args[0], "UTF-8");
        int nf = 0, nc = 0, nl = 0;

        FunctionManager fm = currentProgram.getFunctionManager();
        for (Function f : fm.getFunctions(true)) {
            if (isDefaultFn(f.getName())) continue;          // FUN_/thunk regenerate on reimport
            String addr = f.getEntryPoint().toString();
            String conv = f.getCallingConventionName();
            String proto = f.getSignature(true).getPrototypeString(false);
            w.println("F\t" + addr + "\t" + esc(f.getName()) + "\t" + esc(conv) + "\t" + esc(proto));
            nf++;
        }

        Listing lst = currentProgram.getListing();
        AddressSetView all = currentProgram.getMemory();
        int[] types = { CodeUnit.EOL_COMMENT, CodeUnit.PRE_COMMENT, CodeUnit.POST_COMMENT,
                        CodeUnit.PLATE_COMMENT, CodeUnit.REPEATABLE_COMMENT };
        for (int t : types) {
            AddressIterator ai = lst.getCommentAddressIterator(t, all, true);
            while (ai.hasNext()) {
                Address a = ai.next();
                String c = lst.getComment(t, a);
                if (c == null) continue;
                w.println("C\t" + a.toString() + "\t" + t + "\t" + esc(c));
                nc++;
            }
        }

        SymbolTable st = currentProgram.getSymbolTable();
        for (Symbol s : st.getAllSymbols(true)) {
            if (s.getSource() != SourceType.USER_DEFINED) continue;
            if (s.getSymbolType() != SymbolType.LABEL) continue;
            if (isDefaultLbl(s.getName())) continue;
            w.println("L\t" + s.getAddress().toString() + "\t" + esc(s.getName()));
            nl++;
        }

        w.close();
        println("DUMPMARKUP wrote " + args[0] + " functions=" + nf + " comments=" + nc + " labels=" + nl);
    }
}
