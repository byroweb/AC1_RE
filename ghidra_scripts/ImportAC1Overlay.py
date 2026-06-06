# Import the AC1 runtime overlay (dumped from DuckStation RAM) into Ghidra.
#
# The menu/name code is a runtime overlay above ~0x8004ADA0 and is NOT in the
# base SLUS_013.23 exe. This script maps the dumped bytes in as a new memory
# block at their runtime base, then disassembles the known entry points so the
# decompiler (and the ghidra MCP) can analyze them.
#
# Usage: open the AC1 program in CodeBrowser, then run this in the Script
# Manager (refresh if it doesn't appear; it lives in ghidra_scripts/).
#
# @category AC1
# @menupath Tools.AC1.Import Overlay

from java.io import File, FileInputStream
from ghidra.app.cmd.disassemble import DisassembleCommand

BIN_PATH = "/home/byron/Desktop/AC_1_USA_RE/overlay_80050000.bin"
BASE     = 0x80050000
BLOCK    = "overlay"

# Known overlay entry points (verified live via DuckStation).
ENTRIES = {
    0x8006599c: "draw_string",
    0x800822d8: "name_mutation_loop",   # rebuilds name buffer each keypress
    0x80082180: "name_keypress_handler",  # caller / menu handler
}

mem = currentProgram.getMemory()

# 1. Create the initialized RWX block (idempotent).
if mem.getBlock(BLOCK) is not None:
    print("[ac1] block '%s' already present; skipping create." % BLOCK)
else:
    f = File(BIN_PATH)
    if not f.exists():
        raise Exception("dump not found: %s (run DuckStation read_memory first)" % BIN_PATH)
    fis = FileInputStream(f)
    try:
        blk = mem.createInitializedBlock(
            BLOCK, toAddr(BASE), fis, f.length(), monitor, False)
        blk.setRead(True); blk.setWrite(True); blk.setExecute(True)
        print("[ac1] created block '%s' @ 0x%08x len 0x%x" % (BLOCK, BASE, f.length()))
    finally:
        fis.close()

# 2. Disassemble + create functions at the known entry points.
for addr, name in sorted(ENTRIES.items()):
    a = toAddr(addr)
    DisassembleCommand(a, None, True).applyTo(currentProgram, monitor)
    fn = getFunctionAt(a)
    if fn is None:
        fn = createFunction(a, name)
    if fn is not None:
        fn.setName(name, ghidra.program.model.symbol.SourceType.USER_DEFINED)
        print("[ac1] %s @ 0x%08x" % (name, addr))
    else:
        print("[ac1] WARN could not create function at 0x%08x" % addr)

print("[ac1] done. Run Auto Analyze (or analyze the new block) for full xrefs.")
