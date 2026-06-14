// LabelExitCase.java
// 0x8008A0B0 (mission_exit_commit) is a jump-table dispatcher: it indexes
// table 0x8004C164 (the mission event/VM jump table) by a0-1 and `jr v0`s into
// case bodies. 0x8008A1F4 is the EXIT-COMPLETE case body (reached via jr v0,
// not fallthrough) - which is why Ghidra's function body for 0x8008A0B0 does
// not span it. We add a primary LABEL + pre-comment at 0x8008A1F4 so the
// address is documented where the level-complete commit happens, mirroring the
// label convention used for the script-VM case bodies (0x8008B42C/0x8008B830).
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.SourceType;

public class LabelExitCase extends GhidraScript {
    public void run() throws Exception {
        Listing lst = currentProgram.getListing();
        Address a = toAddr(0x8008A1F4L);
        // primary label
        createLabel(a, "mission_exit_complete_case", true, SourceType.USER_DEFINED);
        lst.setComment(a, CodeUnit.PRE_COMMENT,
            "MISSION-202: EXIT-COMPLETE case body (jump-table target of "
            + "mission_exit_commit @0x8008A0B0, dispatched via jr v0 @0x8008A104 "
            + "off table 0x8004C164). gate: lhu v0,[0x8019F528] @0x8008A1EC; "
            + "bnez v0 @0x8008A1F4 -> if end-countdown already armed, bail to "
            + "0x8008A31C. Else jal 0x8008A048 / 0x8008A080 (COM 'mission "
            + "complete' sub-handlers) then jal mission_set_result(0x8004C318) "
            + "@0x8008A20C with a0=0x80 in delay slot @0x8008A210.");
        println("labeled 0x8008A1F4 = mission_exit_complete_case");
        // sanity: confirm dispatcher name/entry
        Function f = currentProgram.getFunctionManager().getFunctionAt(toAddr(0x8008A0B0L));
        println("dispatcher @0x8008A0B0 = " + (f==null?"NULL":f.getName()
            + " body=" + f.getBody().getNumAddresses()));
    }
}
