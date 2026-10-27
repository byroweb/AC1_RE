"""mkpsexe.py ELF IMAGE OUT: wrap a linked exe image in a generated PS-X EXE header.

IMAGE is `objcopy -O binary` of ELF. The header is built from the ELF, not copied:
  pc0     the entry symbol (--entry, default __SN_ENTRY_POINT)
  t_addr  the start of the image (the lowest allocated section)
  t_size  the image zero-padded to a 0x800 multiple
  s_addr  the initial stack (--stack, default 0x801FFFF0)
  gp0, d/b fields and s_size are 0, as in the retail SLUS_013.23 header.
The region marker at 0x4C is the licence string the boot ROM expects for the
North America region.

--data-end / --bss-end check the link: the start-up code zeroes [data_end, bss_end),
so main_DATA_END and main_BSS_START must land exactly at --data-end, and main_BSS_END
at --bss-end.
"""
import argparse, os, struct, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "match"))
from match_fn import Elf  # noqa: E402

REGION_NA = b"Sony Computer Entertainment Inc. for North America area"
SECTOR = 0x800


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("elf")
    ap.add_argument("image")
    ap.add_argument("out")
    ap.add_argument("--entry", default="__SN_ENTRY_POINT")
    ap.add_argument("--stack", type=lambda s: int(s, 0), default=0x801FFFF0)
    ap.add_argument("--data-end", type=lambda s: int(s, 0))
    ap.add_argument("--bss-end", type=lambda s: int(s, 0))
    args = ap.parse_args()

    elf = Elf(open(args.elf, "rb").read())
    syms = {s["name"]: s["value"] for s in elf.symbols if s["name"]}
    alloc = [s for s in elf.sections if s["flags"] & 2 and s["size"]]   # SHF_ALLOC
    t_addr = min(s["addr"] for s in alloc)
    if args.entry not in syms:
        sys.exit("mkpsexe: entry symbol %s not in %s" % (args.entry, args.elf))
    if args.data_end is not None:
        for name in ("main_DATA_END", "main_BSS_START"):
            if syms.get(name) != args.data_end:
                sys.exit("mkpsexe: %s = 0x%08X, expected 0x%08X"
                         % (name, syms.get(name, 0), args.data_end))
    if args.bss_end is not None and syms.get("main_BSS_END") != args.bss_end:
        sys.exit("mkpsexe: main_BSS_END = 0x%08X, expected 0x%08X" % (syms.get("main_BSS_END", 0), args.bss_end))

    image = open(args.image, "rb").read()
    if args.data_end is not None and t_addr + len(image) != args.data_end:
        sys.exit("mkpsexe: image ends at 0x%08X, expected 0x%08X" % (t_addr + len(image), args.data_end))
    image += bytes(-len(image) % SECTOR)

    hdr = bytearray(SECTOR)
    hdr[0:8] = b"PS-X EXE"
    struct.pack_into("<IIII", hdr, 0x10, syms[args.entry], 0, t_addr, len(image))
    struct.pack_into("<I", hdr, 0x30, args.stack)
    hdr[0x4C:0x4C + len(REGION_NA)] = REGION_NA
    open(args.out, "wb").write(bytes(hdr) + image)
    print("%s: pc0 0x%08X, t_addr 0x%08X, t_size 0x%X" % (args.out, syms[args.entry], t_addr, len(image)))


if __name__ == "__main__":
    main()
