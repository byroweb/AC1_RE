// ApplyMarkup.java — replay RE markup dumped by DumpMarkup.java into the
// current (freshly imported, byte-identical) program. Renames functions, sets
// calling conventions + prototypes (best-effort: prototypes whose types are
// absent are logged and skipped), restores listing comments, and recreates
// user labels. Run AFTER importing the type headers (Parse C Source) so
// prototypes referencing custom structs resolve.
//
// Args: <in.tsv>
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.SourceType;
import ghidra.app.util.parser.FunctionSignatureParser;
import ghidra.app.cmd.function.ApplyFunctionSignatureCmd;
import ghidra.program.model.data.FunctionDefinitionDataType;
import java.io.BufferedReader;
import java.io.FileReader;

public class ApplyMarkup extends GhidraScript {
    static String unesc(String s) {
        StringBuilder b = new StringBuilder();
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c == '\\' && i + 1 < s.length()) {
                char n = s.charAt(++i);
                if (n == 't') b.append('\t');
                else if (n == 'n') b.append('\n');
                else b.append(n);
            } else b.append(c);
        }
        return b.toString();
    }

    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) { println("APPLYMARKUP usage: <in.tsv>"); return; }
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing lst = currentProgram.getListing();
        FunctionSignatureParser fsp = new FunctionSignatureParser(currentProgram.getDataTypeManager(), null);
        int nf = 0, nproto = 0, protoFail = 0, nc = 0, nl = 0;

        BufferedReader r = new BufferedReader(new FileReader(args[0]));
        String line;
        while ((line = r.readLine()) != null) {
            String[] p = line.split("\t", -1);
            if (p.length < 2) continue;
            Address a = currentProgram.getAddressFactory().getAddress(p[1]);
            if (a == null) { println("APPLYMARKUP bad-addr " + p[1]); continue; }
            try {
                if (p[0].equals("F") && p.length >= 5) {
                    String name = unesc(p[2]), conv = unesc(p[3]), proto = unesc(p[4]);
                    Function f = fm.getFunctionContaining(a);
                    if (f == null) f = createFunction(a, null);
                    if (f == null) { println("APPLYMARKUP no-func @ " + p[1] + " (" + name + ")"); continue; }
                    f.setName(name, SourceType.USER_DEFINED);
                    nf++;
                    try { if (conv.length() > 0) f.setCallingConvention(conv); } catch (Exception e) {}
                    try {
                        FunctionDefinitionDataType d = fsp.parse(f.getSignature(), proto);
                        if (d != null && new ApplyFunctionSignatureCmd(a, d, SourceType.USER_DEFINED)
                                .applyTo(currentProgram)) nproto++;
                        else protoFail++;
                    } catch (Exception e) { protoFail++; }
                } else if (p[0].equals("C") && p.length >= 4) {
                    lst.setComment(a, Integer.parseInt(p[2]), unesc(p[3]));
                    nc++;
                } else if (p[0].equals("L") && p.length >= 3) {
                    createLabel(a, unesc(p[2]), true, SourceType.USER_DEFINED);
                    nl++;
                }
            } catch (Exception e) {
                println("APPLYMARKUP skip " + p[0] + " @ " + p[1] + ": " + e.getMessage());
            }
        }
        r.close();
        println("APPLYMARKUP functions=" + nf + " prototypes=" + nproto + " protoFail=" + protoFail
                + " comments=" + nc + " labels=" + nl);
    }
}
