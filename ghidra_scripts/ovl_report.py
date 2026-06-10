# ovl_report.py — headless post-script: report analyzed overlay + decompile entry
#@category AC1
from ghidra.app.decompiler import DecompInterface
from ghidra.util.task import ConsoleTaskMonitor

fm = currentProgram.getFunctionManager()
print("=== OVL REPORT ===")
print("program:", currentProgram.getName())
print("function_count:", fm.getFunctionCount())

# entry pointer is the word at base+4
mem = currentProgram.getMemory()
af = currentProgram.getAddressFactory().getDefaultAddressSpace()
base = af.getAddress(0x8004ada0)
entry_val = mem.getInt(af.getAddress(0x8004ada4)) & 0xffffffff
print("overlay +4 entry ptr: 0x%08x" % entry_val)

ent = af.getAddress(entry_val)
fn = fm.getFunctionContaining(ent)
print("entry function:", fn.getName() if fn else "(none)")

di = DecompInterface()
di.openProgram(currentProgram)
if fn:
    res = di.decompileFunction(fn, 60, ConsoleTaskMonitor())
    if res and res.getDecompiledFunction():
        print("=== DECOMPILE entry ===")
        print(res.getDecompiledFunction().getC()[:4000])
