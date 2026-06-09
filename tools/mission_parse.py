#!/usr/bin/env python3
"""mission_parse.py - dump an AC1 mission descriptor from FDAT.T.

Target: SLUS-01323 (v1.1).  Companion to docs/MISSION_SYSTEM.md and REFERENCE.md
"Mission runtime".  Pristine bin only.

A mission N is described by a PAIR of FDAT.T entries (file id 2):
  * entry  (2*N)    = "objective object" image (relocatable MIPS code + data,
                      loaded by the runtime to 0x801c4b40; starts with a method
                      vtable; CONFIRMED objective logic is per-mission code).
  * entry  (2*N+1)  = the mission CHUNK STREAM: a sequence of
                      [u32 length][payload(length bytes)] chunks consumed in a
                      fixed order by FUN_8004F508 (the scene loader).
                      Chunk 12 of that stream = the 256-record x 40-byte
                      OBJECT-INSTANCE / MT-SPAWN table fed to FUN_80073B74
                      (-> instance table 0x8019FAB8, 256 x 44).

Each 40-byte spawn record = 20 int16 halfwords:
  hw0,hw1,hw2 = X,Y,Z position             (CONFIRMED)
  hw3         = geometry block index        (-> instance +0x0A; -1 = none) CONFIRMED
  hw5         = rotation (PSX angle units)   HYPOTHESIS
  hw7         = object / MT type id          CONFIRMED (varies per mission)
  hw8..hw19   = per-type params (HP?, flags, link ids) HYPOTHESIS

Usage:
  tools/mission_parse.py --fdat disc_map/extracted/FDAT_T/entries  --mission 0
  tools/mission_parse.py --stream disc_map/extracted/FDAT_T/entries/001.bin
  tools/mission_parse.py --stream .../001.bin --spawns       # only chunk-12 table
"""
import argparse
import os
import struct
import sys

SPAWN_CHUNK = 12           # index of the instance/MT table in the chunk stream
SPAWN_REC_LEN = 40         # source record length (20 int16)


def walk_chunks(buf, limit=64):
    """Yield (idx, off, length, stop) for each [u32 len][payload] chunk."""
    off = 0
    idx = 0
    n = len(buf)
    while off + 4 <= n and idx < limit:
        ln = struct.unpack_from("<I", buf, off)[0]
        if ln == 0 or off + 4 + ln > n:
            yield (idx, off, ln, True)   # terminator / out of range
            return
        yield (idx, off, ln, False)
        off += ln + 4
        idx += 1


def dump_chunks(buf):
    print(f"chunk stream: {len(buf)} bytes")
    for idx, off, ln, stop in walk_chunks(buf):
        if stop:
            print(f"  chunk {idx:2d}  off 0x{off:05x}  len {ln} (STOP)")
            return
        tag = ""
        if idx == SPAWN_CHUNK and ln:
            tag = f"  <- SPAWN/INSTANCE table ({ln // SPAWN_REC_LEN} recs)"
        print(f"  chunk {idx:2d}  off 0x{off:05x}  len {ln} (0x{ln:x}){tag}")


def dump_spawns(buf, show_empty=False):
    spawn = next((c for c in walk_chunks(buf)
                  if c[0] == SPAWN_CHUNK and not c[3]), None)
    if spawn is None:
        print("no spawn chunk (index 12) found", file=sys.stderr)
        return
    _, off, ln, _ = spawn
    pay = buf[off + 4: off + 4 + ln]
    n = ln // SPAWN_REC_LEN
    print(f"spawn/instance table: {n} records of {SPAWN_REC_LEN} bytes")
    print(" idx  type  blk        X       Y       Z     rot   hw8  hw9  hw11  hw12  hw15")
    active = 0
    for i in range(n):
        hw = struct.unpack_from("<20h", pay, i * SPAWN_REC_LEN)
        empty = (hw[3] == -1 and hw[0] == 0 and hw[1] == 0
                 and hw[2] == 0 and hw[7] == 0)
        if empty and not show_empty:
            continue
        active += 1
        print(f" {i:3d}  {hw[7]:4d}  {hw[3]:3d}  {hw[0]:7d} {hw[1]:7d} {hw[2]:7d}  "
              f"{hw[5]:6d}  {hw[8]:4d} {hw[9]:4d} {hw[11]:5d} {hw[12]:5d} {hw[15]:5d}")
    print(f"({active} non-empty records)")


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat", help="dir of extracted FDAT entries (NNN.bin)")
    ap.add_argument("--mission", type=int, help="mission number (uses entry 2N+1)")
    ap.add_argument("--stream", help="path to a single chunk-stream entry (NNN.bin)")
    ap.add_argument("--spawns", action="store_true", help="dump only the spawn table")
    ap.add_argument("--all", action="store_true", help="include empty spawn slots")
    args = ap.parse_args()

    if args.stream:
        path = args.stream
    elif args.fdat is not None and args.mission is not None:
        path = os.path.join(args.fdat, f"{2 * args.mission + 1:03d}.bin")
    else:
        ap.error("give --stream PATH  or  --fdat DIR --mission N")

    with open(path, "rb") as f:
        buf = f.read()
    print(f"=== {path} ===")
    if args.spawns:
        dump_spawns(buf, show_empty=args.all)
    else:
        dump_chunks(buf)
        print()
        dump_spawns(buf, show_empty=args.all)


if __name__ == "__main__":
    main()
