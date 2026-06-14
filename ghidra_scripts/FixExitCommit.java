// FixExitCommit.java
// The prior pass created a 1-byte stub at 0x8008A1F4 because no function
// contained that address. The real function entry (verified by disassembly:
// single prologue 'addiu sp,sp,-48' @0x8008A0B0, no intervening jr ra before
// 0x8008A1F4, first jr ra after at 0x8008A33C) is 0x8008A0B0.
// This script: delete the bogus stub at 0x8008A1F4, ensure a function at
// 0x8008A0B0, name it mission_exit_commit, and set the plate comment there.
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.SourceType;

public class FixExitCommit extends GhidraScript {
    static final long ENTRY = 0x8008A0B0L;
    static final long STUB  = 0x8008A1F4L;
    static final String NAME = "mission_exit_commit";
    static final String COMMENT =
        "MISSION-202: Crossing the level-exit border while objectives complete "
        + "(byte 0x801D0B50==3) -> fires the 'mission complete' COM sub-handlers "
        + "(jal 0x8008A048 / 0x8008A080) then commits the result via "
        + "FUN_8004C318(0x80) and arms the end-countdown. Runs each frame as the "
        + "objective object's update. [entry 0x8008A0B0; gate lhu 0x8019F528 "
        + "@0x8008A1EC; bnez @0x8008A1F4; jal 0x8004C318 @0x8008A20C with a0=0x80 "
        + "in delay slot @0x8008A210]";

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing lst = currentProgram.getListing();

        // remove the bogus 1-byte stub if it sits exactly at 0x8008A1F4
        Address stub = toAddr(STUB);
        Function sf = fm.getFunctionAt(stub);
        if (sf != null) {
            println("removing bogus stub @ " + sf.getEntryPoint());
            removeFunctionAt(stub);
            // clear any plate comment left on it
            lst.setComment(stub, CodeUnit.PLATE_COMMENT, null);
        }

        Address entry = toAddr(ENTRY);
        Function f = fm.getFunctionContaining(entry);
        if (f == null || !f.getEntryPoint().equals(entry)) {
            try { f = createFunction(entry, NAME); } catch (Exception ex) { f = null; }
        }
        if (f == null) {
            println("FAIL: could not establish function @ " + entry);
            return;
        }
        f.setName(NAME, SourceType.USER_DEFINED);
        lst.setComment(f.getEntryPoint(), CodeUnit.PLATE_COMMENT, COMMENT);
        println("OK " + NAME + " @ entry " + f.getEntryPoint()
            + " (body " + f.getBody().getNumAddresses() + " bytes)");
        // sanity: confirm it now contains 0x8008A1F4
        Function chk = fm.getFunctionContaining(toAddr(STUB));
        println("contains 0x8008A1F4 -> " + (chk == null ? "NULL" : chk.getName()
            + " @ " + chk.getEntryPoint()));
    }
}
