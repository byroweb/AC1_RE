// AnnotateMission202.java — name + comment the confirmed mission-runtime
// (FDAT entry 202) functions after the raw overlay is imported & analyzed at
// base 0x80050000. Forces a function to exist at each address if auto-analysis
// missed the boundary, renames it, and sets a one-line plate comment.
// Source of truth: overlays/README_mission_overlay.md +
// memory project_ac1_dynamic_spawn / project_ac1_mission_runtime.
//@category AC1
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.SourceType;

public class AnnotateMission202 extends GhidraScript {
    // {hex addr, name, one-line comment}
    static final String[][] FUNCS = {
        {"0x8008b42c", "mission_script_vm_dispatch",
            "MISSION-202: script-VM central dispatch loop; opcode stream ~0x8014FCD8, each handler ends j here"},
        {"0x8008b830", "mission_op_spawn_group",
            "MISSION-202: 'spawn group N' opcode; s1=0x8019FAB8+N*44; if flag!=1 -> call spawn 0x80078CFC"},
        {"0x80078cfc", "mission_spawn_routine",
            "MISSION-202: spawn routine (a0=spawn record): alloc slot, load resources, init record, set flag"},
        {"0x80078a2c", "mission_enemy_slot_alloc",
            "MISSION-202: enemy-slot allocator; returns AC-array record, base 0x801A26B8 stride 0x170"},
        {"0x800788c8", "mission_resource_loader",
            "MISSION-202: resource loader; copies+relocates geometry/collision via FUN_80050FC4"},
        {"0x80078b14", "mission_spawn_binder",
            "MISSION-202: binder; sb 1->[spawnrec+0] (spawned flag), sb slot_idx->[spawnrec+1]"},
        {"0x80078c00", "mission_spawn_record_init",
            "MISSION-202: record initializer (~addr); type ptr 0x8019F590, AI/collision defaults, back-links to spawn point"},
    };

    public void run() throws Exception {
        FunctionManager fm = currentProgram.getFunctionManager();
        Listing lst = currentProgram.getListing();
        int ok = 0;
        for (String[] e : FUNCS) {
            long v = Long.decode(e[0]) & 0xffffffffL;
            Address ad = toAddr(v);
            Function f = fm.getFunctionContaining(ad);
            if (f == null || !f.getEntryPoint().equals(ad)) {
                // force a function at exactly this address
                try { f = createFunction(ad, e[1]); } catch (Exception ex) { f = null; }
            }
            if (f == null) {
                println("ANNOTATE FAIL (no func) " + e[0]);
                // still drop a plate comment at the address so it's documented
                lst.setComment(ad, CodeUnit.PLATE_COMMENT, e[1] + " -- " + e[2]);
                continue;
            }
            f.setName(e[1], SourceType.USER_DEFINED);
            lst.setComment(f.getEntryPoint(), CodeUnit.PLATE_COMMENT, e[2]);
            println("ANNOTATE OK " + e[0] + " -> " + e[1]
                + " @ " + f.getEntryPoint() + " (body " + f.getBody().getNumAddresses() + " bytes)");
            ok++;
        }
        // global program note
        lst.setComment(toAddr(0x80050000L), CodeUnit.PLATE_COMMENT,
            "AC1 MISSION-RUNTIME OVERLAY (FDAT entry 202 / 0xCA). Loads to 0x80050000-0x800DFFFF "
            + "ONLY while a mission is running. This region is SWAPPABLE: the garage/UI overlay 201 "
            + "and link/arena overlays 203/204 occupy the SAME addresses at other times. "
            + "Addresses >=0x80050000 here are MISSION-202 ONLY. The resident main exe (0x80011000-0x80039FFF) "
            + "is unaffected. Dump: mission_overlay_FDAT202_80050000.bin.");
        println("ANNOTATE DONE ok=" + ok + "/" + FUNCS.length);
    }
}
