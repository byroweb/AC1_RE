// DefineMissionVMTable.java
// Reconstructs the mission script-VM event switch in the isolated Mission202
// project (FDAT-202 overlay).
//
// The dispatcher mission_exit_commit @0x8008A0B0:
//     andi a0,a0,0xff; addiu a0,a0,-1        (index = a0-1)
//     sltiu v0,a0,10                          (bounds: 10 entries)
//     beqz v0,0x8008A318                      (out-of-range -> default)
//     sll v0,a0,2; lui at,0x8005; addu at,at,v0
//     lw v0,-16028(at)   == lw v0, [0x8004C164 + index*4]
//     jr v0  @0x8008A104                       (dispatch)
//
// Table 0x8004C164 lives in the ovl202_lower block (0x8004ADA0..0x8004FFFF),
// created by AnnotateMission202Complete.java. This script:
//  1) defines 0x8004C164 as 10 pointer entries (case map),
//  2) labels each target 0x8008Axxxx as mission_vm_caseNN (+ pre-comment with
//     the known/observed semantics),
//  3) builds a JumpTable at the jr v0 (0x8008A104) so the decompiler renders
//     mission_exit_commit as a switch.
//
// Source of truth: dispatcher + case bodies disassembled from
// overlays/mission_overlay_FDAT202_80050000.bin; table bytes read from
// disc_map/overlays/ovl202_mission.bin (base 0x8004ADA0).
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.PointerDataType;
import ghidra.program.model.symbol.*;
import ghidra.program.model.scalar.Scalar;
import ghidra.program.model.pcode.JumpTable;

public class DefineMissionVMTable extends GhidraScript {

    static final long TABLE   = 0x8004C164L;   // mission VM jump table
    static final long JR_ADDR = 0x8008A104L;   // jr v0 (computed jump)
    static final int  COUNT   = 10;            // from sltiu v0,a0,10

    // index -> case-body target (a0 = index+1), confirmed by disassembly.
    static final long[] TARGET = {
        0x8008A10CL, // case00 a0=1
        0x8008A158L, // case01 a0=2
        0x8008A168L, // case02 a0=3
        0x8008A178L, // case03 a0=4
        0x8008A1E8L, // case04 a0=5
        0x8008A21CL, // case05 a0=6
        0x8008A248L, // case06 a0=7
        0x8008A260L, // case07 a0=8
        0x8008A284L, // case08 a0=9
        0x8008A2A8L, // case09 a0=10
    };

    // observed semantics (from case-body disassembly). Unknown => generic note.
    static final String[] NOTE = {
        // 00
        "MISSION-202 VM case (a0=1): jal 0x8008A048 (COM/broadcast sub-handler, "
        + "a0=s3); then loops a world-object array (lui 0x801d; lw 0xBA0(at)) "
        + "clearing byte +6 (sb zero,6(v0)). Reset/broadcast op.",
        // 01
        "MISSION-202 VM case (a0=2): jal 0x8008C2FC (a0=s0) - secondary VM/COM "
        + "routine. Semantics not yet confirmed.",
        // 02
        "MISSION-202 VM case (a0=3): jal 0x8008A048 (a0=s0) - COM/broadcast "
        + "sub-handler. Semantics not yet confirmed.",
        // 03
        "MISSION-202 VM case (a0=4) SET-TIMER: jal 0x8008A048; computes s0 = "
        + "operand*22 (1*2*..*9*2 chain), jal 0x8008A778(a0=s0,a1=mode,a2=s3); "
        + "sh s0,[0x8019F52C] = the displayed mission timer. (= old cmd4 set-timer; "
        + "s4 mode picks 256/512/128 frames-per-unit.)",
        // 04
        "MISSION-202 VM case (a0=5) EXIT-COMPLETE / SUCCESS(0x80): gate lhu "
        + "[0x8019F528] @0x8008A1EC; if !=0 bail (bnez @0x8008A1F4); else jal "
        + "0x8008A048 / 0x8008A080 (COM 'mission complete') then jal "
        + "mission_set_result(0x8004C318) @0x8008A20C with a0=0x80. The "
        + "level-complete commit. (See label mission_exit_complete_case @0x8008A1F4.)",
        // 05
        "MISSION-202 VM case (a0=6) RESULT(0x100) via COM: gate lhu [0x8019F528] "
        + "@0x8008A220; if !=0 bail; else jal 0x8008A048 / 0x8008A080 then falls "
        + "into case08 tail @0x8008A274 -> jal mission_set_result(0x8004C318) "
        + "a0=0x100. Commits result code 0x100.",
        // 06
        "MISSION-202 VM case (a0=7): jal 0x8008A048 / 0x8008A080 (COM "
        + "broadcast sub-handlers), then return. Pure broadcast op.",
        // 07
        "MISSION-202 VM case (a0=8) SUCCESS-IF-99: jal 0x80052A2C(a0=99); if "
        + "ret==1 -> jal mission_set_result(0x8004C318) a0=0x100 @0x8008A274. "
        + "(= old cmd8 success-if-99; 0x80052A2C tests a global condition slot.)",
        // 08
        "MISSION-202 VM case (a0=9) OBJECTIVE-OBJECT METHOD: lw [0x8019F51C] "
        + "(objective object 0x801C4B40); lw 8(v0); jalr v0(a0=s1). Calls method "
        + "+0x8 of the per-mission objective object. (= old cmd9.)",
        // 09
        "MISSION-202 VM case (a0=10): jal 0x8008A048; loops the world-object "
        + "array @0x801D0B68 (s5=0x801D0B68, s3=0x8200 mask). Broadcast over "
        + "world objects. Semantics not yet confirmed.",
    };

    public void run() throws Exception {
        Listing lst = currentProgram.getListing();
        FunctionManager fm = currentProgram.getFunctionManager();

        // sanity: lower block present?
        if (currentProgram.getMemory().getBlock(toAddr(TABLE)) == null) {
            println("FAIL: no memory block at 0x8004C164 (run "
                + "AnnotateMission202Complete.java first to create ovl202_lower).");
            return;
        }

        // ---- 1) define the table as COUNT pointer entries ----
        PointerDataType ptr = new PointerDataType(currentProgram.getDataTypeManager());
        Address[] tgt = new Address[COUNT];
        for (int i = 0; i < COUNT; i++) {
            Address ent = toAddr(TABLE + (long)i * 4);
            lst.clearCodeUnits(ent, ent.add(3), false);
            Data d = lst.createData(ent, ptr);
            // read what the pointer resolves to and verify it matches TARGET[i]
            Object val = d.getValue();
            long got = (val instanceof Address) ? ((Address)val).getOffset()
                       : ((Scalar)d.getScalar(0)).getUnsignedValue();
            tgt[i] = toAddr(TARGET[i]);
            String ok = (got == TARGET[i]) ? "OK" : ("MISMATCH got 0x"
                       + Long.toHexString(got));
            println(String.format("entry %2d @0x%08x -> 0x%08x  %s",
                i, ent.getOffset(), TARGET[i], ok));
        }
        // label the table itself
        createLabel(toAddr(TABLE), "mission_vm_jumptable", true,
            SourceType.USER_DEFINED);
        lst.setComment(toAddr(TABLE), CodeUnit.PLATE_COMMENT,
            "MISSION-202: mission script-VM event jump table (10 entries). "
            + "Indexed by (a0-1) in mission_exit_commit @0x8008A0B0 "
            + "(sltiu v0,a0,10; lw v0,[0x8004C164+idx*4]; jr v0 @0x8008A104). "
            + "Each entry -> a case body in 0x8008A10C..0x8008A2A8 "
            + "(mission_vm_case00..09). NOTE: distinct from the secondary "
            + "spawn-group dispatch 0x8008B42C/0x8008B830 - do not conflate.");

        // ---- 2) label each case-body target + pre-comment ----
        for (int i = 0; i < COUNT; i++) {
            String nm = String.format("mission_vm_case%02d", i);
            createLabel(tgt[i], nm, true, SourceType.USER_DEFINED);
            // do not clobber the existing exit-complete pre-comment region;
            // place pre-comment at the true target address.
            String existing = lst.getComment(CodeUnit.PRE_COMMENT, tgt[i]);
            String body = NOTE[i];
            if (existing != null && !existing.isEmpty())
                body = body + "  [existing: " + existing + "]";
            lst.setComment(tgt[i], CodeUnit.PRE_COMMENT, body);
            println("labeled " + nm + " @ " + tgt[i]);
        }

        // ---- 3) build the JumpTable so the decompiler renders the switch ----
        Function disp = fm.getFunctionContaining(toAddr(JR_ADDR));
        println("dispatcher @0x8008A104 -> " + (disp == null ? "NULL"
            : disp.getName() + " entry " + disp.getEntryPoint()));
        try {
            java.util.ArrayList<Address> cases = new java.util.ArrayList<>();
            for (Address a : tgt) cases.add(a);
            JumpTable jt = new JumpTable(toAddr(JR_ADDR), cases, true);
            jt.writeOverride(disp != null ? disp : fm.getFunctionContaining(toAddr(0x8008A0B0L)));
            println("JumpTable override written at jr v0 @0x8008A104 ("
                + COUNT + " cases).");
        } catch (Exception ex) {
            println("JumpTable override FAILED: " + ex
                + " -- references/labels still applied; decompiler may need a "
                + "manual 'create jump table' but the data+labels are correct.");
        }

        println("DefineMissionVMTable DONE.");
    }
}
