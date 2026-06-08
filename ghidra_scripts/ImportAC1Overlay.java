// ImportAC1Overlay.java — map the AC1 runtime overlay (dumped from DuckStation
// RAM) into the current Ghidra program so the name/menu code becomes
// decompilable + xref-able alongside the base exe.
//
// Java (NOT .py): Ghidra 11+ dropped Jython, so .py scripts need PyGhidra
// ("Python is not available" error). Java GhidraScripts always work.
//
// The menu/name code is a runtime overlay at 0x80050000 (above the base exe
// 0x80011000-0x80039fff) and is NOT in the SLUS_013.23 image. This splits the
// generic RAM block at 0x80050000, fills it with the dump, marks it executable,
// then disassembles + names the known entry points.
//
// NOTE: overlay_80050000.bin is a point-in-time RAM dump; it is STALE for code
// baked after the dump (re-dump live for current bytes). For one-off current
// functions, tools/overlay2c.sh is faster.
//
// Run from: Window -> Script Manager (category AC1_RE).
//@category AC1_RE
//@author RE session

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.listing.Function;
import ghidra.app.cmd.disassemble.DisassembleCommand;
import ghidra.app.cmd.function.CreateFunctionCmd;

import java.io.File;
import java.io.FileInputStream;
import java.util.LinkedHashMap;
import java.util.Map;

public class ImportAC1Overlay extends GhidraScript {

    private static final String BIN_PATH = "/home/byron/Desktop/AC_1_USA_RE/overlay_80050000.bin";
    private static final long   BASE     = 0x80050000L;
    private static final String BLOCK    = "overlay";

    // Known overlay entry points (verified live via DuckStation / overlay2c).
    private static final Map<Long, String> ENTRIES = new LinkedHashMap<>();
    static {
        ENTRIES.put(0x80059b50L, "farsi_gte_renderer_type6");
        ENTRIES.put(0x8005b648L, "box_init");
        ENTRIES.put(0x8005c2a8L, "box_init_caller");
        ENTRIES.put(0x8006599cL, "draw_string");
        ENTRIES.put(0x80065dbcL, "draw_kanji_RECLAIMED_shape_name");
        ENTRIES.put(0x800660c4L, "draw_char");
        ENTRIES.put(0x80082190L, "name_input_handler");
        ENTRIES.put(0x80083a28L, "cursor_update");
        ENTRIES.put(0x8009bce0L, "ui_update_loop");
        ENTRIES.put(0x8009c07cL, "render_dispatcher");
    }

    @Override
    public void run() throws Exception {
        Memory  mem  = currentProgram.getMemory();
        Address base = toAddr(BASE);

        File f = new File(BIN_PATH);
        if (!f.exists()) {
            throw new Exception("dump not found: " + BIN_PATH +
                " (DuckStation read_memory 0x80050000 -> file first)");
        }
        byte[] data = new byte[(int) f.length()];
        try (FileInputStream fis = new FileInputStream(f)) {
            int off = 0, n;
            while (off < data.length && (n = fis.read(data, off, data.length - off)) > 0) off += n;
        }
        println("[ac1] read " + data.length + " bytes from " + BIN_PATH);

        MemoryBlock blk = mem.getBlock(base);
        if (blk == null) throw new Exception("no block at 0x" + Long.toHexString(BASE));

        // Split the generic RAM block so [BASE .. BASE+len) is its own block.
        if (!blk.getStart().equals(base)) {
            mem.split(blk, base);
        }
        blk = mem.getBlock(base);
        Address endAddr = base.add(data.length);
        if (blk.getEnd().compareTo(endAddr) >= 0) {
            mem.split(blk, endAddr);
            blk = mem.getBlock(base);
        }
        if (!blk.isInitialized()) {
            blk = mem.convertToInitialized(blk, (byte) 0);
            println("[ac1] converted overlay block to initialized");
        }
        try { blk.setName(BLOCK); } catch (Exception e) { /* name may already be taken */ }
        mem.setBytes(base, data);
        blk.setRead(true); blk.setWrite(true); blk.setExecute(true);
        println("[ac1] overlay mapped [" + blk.getStart() + "-" + blk.getEnd() + "]");

        // Disassemble + name the known entry points.
        for (Map.Entry<Long, String> e : ENTRIES.entrySet()) {
            Address a = toAddr(e.getKey());
            new DisassembleCommand(a, null, true).applyTo(currentProgram, monitor);
            Function fn = getFunctionAt(a);
            if (fn == null) {
                new CreateFunctionCmd(a).applyTo(currentProgram);
                fn = getFunctionAt(a);
            }
            if (fn != null) {
                fn.setName(e.getValue(), SourceType.USER_DEFINED);
                println("[ac1] " + e.getValue() + " @ 0x" + Long.toHexString(e.getKey()));
            } else {
                println("[ac1] WARN could not create function at 0x" + Long.toHexString(e.getKey()));
            }
        }
        println("[ac1] done. Run Auto Analyze for full xrefs into the named base-exe SDK.");
    }
}
