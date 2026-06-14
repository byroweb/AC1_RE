// AnnotateMission202Complete.java
// Annotates the newly reverse-engineered "level complete" mechanism in the
// isolated Mission202 Ghidra project (FDAT entry 202 mission-runtime overlay,
// main dump base 0x80050000).
//
// Source of truth: DuckStation live-RE 2026-06-14, memory note
// project_ac1_objective_complete; OVERLAY_MAP.md.
//
// Two parts:
//  A) Three functions already inside the 0x80050000 dump:
//       mission_exit_commit      (entry containing 0x8008A1F4)
//       mission_phase_dispatch   (0x8008AB68)
//       mission_eventflags_tick  (entry containing the 0x8008A938 store, ~0x8008A8F8)
//  B) The result/end primitive FUN_8004C318 sits BELOW the 0x80050000 base, in
//     the overlay's lower portion. We import bytes 0x8004ADA0..0x8004FFFF from
//     the carved disc_map/overlays/ovl202_mission.bin (load_base 0x8004ADA0) as a
//     regular initialized memory block (the address space 0x8004ADA0..0x8004FFFF
//     is empty in this project, below the dump), disassemble it, then annotate
//     0x8004C318 = mission_set_result.
//
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressSet;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.SourceType;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import java.io.RandomAccessFile;

public class AnnotateMission202Complete extends GhidraScript {

    // carved lower-portion overlay image (base 0x8004ADA0)
    static final String LOWER_BIN =
        "/home/byron/Desktop/AC_1_USA_RE/disc_map/overlays/ovl202_mission.bin";
    static final long LOWER_BASE  = 0x8004ADA0L;
    static final long DUMP_BASE   = 0x80050000L;          // existing block start
    static final int  LOWER_LEN   = (int)(DUMP_BASE - LOWER_BASE); // 0x5260

    // {addr-inside-func (hex), name, plate comment}
    static final String[][] IN_RANGE = {
        {"0x8008a1f4", "mission_exit_commit",
         "MISSION-202: Crossing the level-exit border while objectives complete "
         + "(byte 0x801D0B50==3) -> fires the 'mission complete' COM sub-handlers "
         + "(jal 0x8008A048 / 0x8008A080) then commits the result via "
         + "FUN_8004C318(0x80) and arms the end-countdown. Runs each frame as the "
         + "objective object's update. [gate lhu 0x8019F528@0x8008A1EC; bnez "
         + "@0x8008A1F4; jal 0x8004C318@0x8008A20C with a0=0x80 in delay slot "
         + "@0x8008A210]"},
        {"0x8008ab68", "mission_phase_dispatch",
         "MISSION-202: Per-frame: calls [*[0x8019F51C]] - the objective object "
         + "(0x801C4B40) update method - via jalr at 0x8008AB80."},
        {"0x8008a938", "mission_eventflags_tick",
         "MISSION-202: Per-frame maintainer of the mission event-flags word "
         + "0x8019F524: lw; AND ~0x1000; sw @0x8008A938 (clears transient bit "
         + "0x1000). NOTE: this per-frame write makes a plain watchpoint on "
         + "0x8019F524 useless."},
    };

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing lst = currentProgram.getListing();

        // ---- Part A: in-range functions ----
        for (String[] e : IN_RANGE) {
            long v = Long.decode(e[0]) & 0xffffffffL;
            Address ad = toAddr(v);
            Function f = fm.getFunctionContaining(ad);
            if (f == null) {
                try { f = createFunction(ad, e[1]); } catch (Exception ex) { f = null; }
            }
            if (f == null) {
                println("ANNOTATE FAIL (no func) " + e[0]);
                lst.setComment(ad, CodeUnit.PLATE_COMMENT, e[1] + " -- " + e[2]);
                continue;
            }
            f.setName(e[1], SourceType.USER_DEFINED);
            lst.setComment(f.getEntryPoint(), CodeUnit.PLATE_COMMENT, e[2]);
            println("ANNOTATE OK (in-range) " + e[0] + " -> " + e[1]
                + " @ entry " + f.getEntryPoint()
                + " (body " + f.getBody().getNumAddresses() + " bytes)");
        }

        // ---- Part B: import lower block & annotate mission_set_result ----
        Memory mem = currentProgram.getMemory();
        Address lowerStart = toAddr(LOWER_BASE);
        MemoryBlock existing = mem.getBlock(lowerStart);
        if (existing != null) {
            println("LOWER BLOCK already present (" + existing.getName()
                + " @ " + existing.getStart() + "); skipping create.");
        } else {
            byte[] bytes = new byte[LOWER_LEN];
            RandomAccessFile raf = new RandomAccessFile(LOWER_BIN, "r");
            raf.readFully(bytes, 0, LOWER_LEN);   // bytes 0..0x5260 == 0x8004ADA0..0x8004FFFF
            raf.close();
            MemoryBlock blk = mem.createInitializedBlock(
                "ovl202_lower", lowerStart, LOWER_LEN, (byte)0, monitor, false);
            blk.setRead(true); blk.setWrite(false); blk.setExecute(true);
            mem.setBytes(lowerStart, bytes);
            println("CREATED lower block ovl202_lower @ " + blk.getStart()
                + ".." + blk.getEnd() + " (" + LOWER_LEN + " bytes) from " + LOWER_BIN);
            // disassemble the new block so the function/instructions exist
            DisassembleCommand dc = new DisassembleCommand(
                new AddressSet(blk.getStart(), blk.getEnd()), null, true);
            dc.applyTo(currentProgram, monitor);
            println("disassembled lower block.");
        }

        // annotate 0x8004C318 = mission_set_result
        Address resAd = toAddr(0x8004C318L);
        Function rf = fm.getFunctionContaining(resAd);
        if (rf == null || !rf.getEntryPoint().equals(resAd)) {
            try { rf = createFunction(resAd, "mission_set_result"); }
            catch (Exception ex) { rf = null; }
        }
        String resComment =
            "MISSION-202 (lower overlay 0x8004ADA0 block): Mission result/end "
            + "primitive. sw a0,[0x8019F524] @0x8004C324 stores the result code "
            + "(a0; 0x80 from the exit-complete path), then if [0x8019F528]==0 "
            + "sets the end-countdown 0x8019F528=100 (~debrief transition in ~100 "
            + "frames). Called from mission_exit_commit @0x8008A20C.";
        if (rf == null) {
            println("ANNOTATE FAIL mission_set_result (no func @0x8004C318)");
            lst.setComment(resAd, CodeUnit.PLATE_COMMENT,
                "mission_set_result -- " + resComment);
        } else {
            rf.setName("mission_set_result", SourceType.USER_DEFINED);
            lst.setComment(rf.getEntryPoint(), CodeUnit.PLATE_COMMENT, resComment);
            println("ANNOTATE OK mission_set_result @ " + rf.getEntryPoint()
                + " (body " + rf.getBody().getNumAddresses() + " bytes)");
        }

        println("AnnotateMission202Complete DONE.");
    }
}
