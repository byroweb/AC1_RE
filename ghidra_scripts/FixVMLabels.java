// FixVMLabels.java — the two script-VM addresses (0x8008b42c dispatch,
// 0x8008b830 spawn-group opcode) are mid-routine jump targets, not function
// entry points. Auto-analysis put them inside the larger VM function
// (FUN_8008A0B0 / FUN_8008AB68 region). Remove the bogus 1-byte stub functions
// AnnotateMission202 created there, replace with a primary LABEL + plate comment
// so they read correctly in the listing.
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.*;

public class FixVMLabels extends GhidraScript {
    static final String[][] LBL = {
        {"0x8008b42c", "mission_script_vm_dispatch",
            "MISSION-202: script-VM central dispatch (lhu opcode@s0, bounds<32, beq->handlers). NOT a func entry; lives inside the VM routine ~0x8008b41c."},
        {"0x8008b830", "mission_op_spawn_group",
            "MISSION-202: opcode 0x1003 'spawn group N' handler. s1=0x8019FAB8(+t3) + N*44; lbu flag; if!=1 jal 0x80078cfc(a0=s1). Reached from dispatch beq @0x8008b45c."},
    };
    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing lst = currentProgram.getListing();
        SymbolTable st = currentProgram.getSymbolTable();
        for (String[] e : LBL) {
            Address ad = toAddr(Long.decode(e[0]) & 0xffffffffL);
            Function f = fm.getFunctionAt(ad);
            if (f != null && f.getBody().getNumAddresses() <= 4) {
                removeFunctionAt(ad);
                println("FIX removed stub func @ " + e[0]);
            }
            createLabel(ad, e[1], true, SourceType.USER_DEFINED);
            lst.setComment(ad, CodeUnit.PLATE_COMMENT, e[2]);
            println("FIX label " + e[0] + " -> " + e[1]);
        }
    }
}
