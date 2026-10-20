#!/usr/bin/env python3
"""mission_geom.py - byte-exact parse / edit / round-trip of a mission's PER-MISSION
GEOMETRY (the chunk-0 geometry blocks of FDAT.T entry 2N+1).

Target: SLUS-01323 (v1.1). Companion to tools/mission/mission_parse.py (chunk walk),
tools/pa/pa_encode.py (PA geometry block encoder) and disc_map/trace/assemble_levels.py
(which decodes chunk-0 geometry + chunk-7 placement to OBJ). Pristine bin only.

WHAT WAS DETERMINED (evidence in scratch/re/mission_geom_notes.md)
------------------------------------------------------------------
1. The mission chunk-0 geometry blocks are the SAME block format as PA##.T geometry
   blocks. pa_encode.parse()/encode() round-trips every chunk-0 block byte-exact
   (verified 10/10, 34/34, 14/14 on missions 0/1/2). They are delimited by a leading
   u32 SIZE PREFIX (block[+0] == len(block)); chunk 0 is NOT a count/offset table.

   Chunk-0 PAYLOAD layout (payload = bytes after the chunk's own [u32 len] header):
       [u32 0x80]                 constant header word (payload byte 0)
       [block0][block1]...[blockN] size-prefixed PA geometry blocks (start payload+4)
       [tail]                     run of 0x00000004 words (4-byte block stubs) padding
                                  chunk 0 out to its declared length; geom_blocks()
                                  STOPs here (size 4 < 12).

   We model chunk 0 as: header span (payload[0:4]) + N GeomBlock segments + tail span.
   The header and tail are kept as raw bytes, so splice is byte-exact.

2. Chunk 11 ("per-mission geom records") is a NESTED sub-stream, not a flat 44-byte
   record array. Header = u32 count; then `count` variable-length sub-records, each
   beginning with a u32 sublen (the per-record geometry-model blob the runtime handler
   FUN_800739AC copies, allocating sublen-36 bytes; see docs/MISSION_STAGES.md). It is
   the SOURCE of instance-table records [1..N] @0x8019F564+(k-1)*44 (parked
   enemies / scenario props). Because the sub-record stride is not a clean fixed
   width on every mission (some missions have count>1 with trailing empty/zero
   sub-records), chunk 11 is round-tripped here as a single RAW span (byte-exact)
   and only CHARACTERIZED (count + first sublens) for the dump. The authored
   geometry-edit path is chunk 0.

READ-SIDE HELPERS (no PA decode; what tools/ac1mod/core/level.py reads):
  geom_blocks(stream)        -> chunk-0 blocks as raw bytes
  placement_records(stream)  -> chunk-7 section table (52-byte records)
  placements(stream, nblk)   -> [(block, translation, light)]; world =
                                block_local * SCALE + translation

CLI (mirrors tools/mission/mission_parse.py / pa_encode.py):
  tools/mission/mission_geom.py --fdat disc_map/extracted/FDAT_T/entries --mission 0
  tools/mission/mission_geom.py --fdat .../entries --mission 0 --translate 0 0 100 0 0
                                                  # BLOCK SUB DX DY DZ : demo edit
  tools/mission/mission_geom.py --selftest        # gate: every mission stream
"""
import argparse
import collections
import os
import struct
import sys

HERE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
_PA = os.path.join(HERE, "tools", "pa")
if _PA not in sys.path:            # an importer (e.g. AC1mod) may already have it
    sys.path.insert(0, _PA)
import pa_encode  # noqa: E402  (byte-exact PA geometry block encoder)
_MISSION = os.path.join(HERE, "tools", "mission")
if _MISSION not in sys.path:
    sys.path.insert(0, _MISSION)
from mission_parse import chunk_span  # noqa: E402,F401  (the one FDAT 2N+1 chunk walker)

GEOM_CHUNK = 0     # chunk-0 = local geometry blocks
PLACE_CHUNK = 7    # chunk-7 = section placement table
RECS_CHUNK = 11    # chunk-11 = per-mission geometry records (nested sub-stream)
PLACE_REC_LEN = 52

# Chunk-0 blocks are authored at 4x the placement scale (the engine renders them
# through a down-scaling GTE matrix): world = block_local * SCALE + translation.
# CONFIRMED universal: every chunk-7 section bbox is exactly 0.25x its block's bbox,
# and the 2500-unit cell translations only tile without overlap at this scale.
# Spawn X,Y,Z (chunk 12) are already WORLD units (not x SCALE): measured on missions
# 0-49, 571 spawns stand within 100 units above a floor face unscaled, 122 at x0.25.
SCALE = 0.25


# =====================================================================================
# Chunk-0 block run (raw, no decode)
# =====================================================================================
def _block_spans(pay):
    """[(start, end)] of the size-prefixed PA blocks in a chunk-0 payload, walked
    from payload+4 until a size < 12 (the 0x00000004 tail stubs) or an overrun."""
    spans, off = [], 4
    while off + 4 <= len(pay):
        sz = struct.unpack_from("<I", pay, off)[0]
        if sz < 12 or off + sz > len(pay):
            break                                 # tail (0x04 stubs) -> stop
        spans.append((off, off + sz))
        off += sz
    return spans


def geom_blocks(stream):
    """The chunk-0 geometry blocks of a mission stream as raw bytes (each a whole
    size-prefixed PA block, decodable by tools/pa/pa_obj.py); [] if no chunk 0."""
    span = chunk_span(stream, GEOM_CHUNK)
    if span is None:
        return []
    off, ln = span
    pay = stream[off:off + ln]
    return [pay[a:b] for a, b in _block_spans(pay)]


# =====================================================================================
# Chunk-7 section placement table
# =====================================================================================
# 52-byte records, terminated by s16 -1 at +0x06 (solved live 2026-06-12; field map
# in disc_map/trace/assemble_levels.py):
#   +0x00 s16[3] world bbox min   +0x06 s16 index (PVS/cell; -1 ends the table)
#   +0x08 s16[3] world bbox max   +0x10 s16[3] placement translation
#   +0x18 s16[3] copy of bbox min +0x1e s16 chunk-0 block index (blocks are reused)
#   +0x26 s16 zone/portal index   +0x28 u8 lighting-record index   +0x2a u16 flags
# No per-section rotation: chunk-0 blocks are pre-rotated on disc.
Placement = collections.namedtuple(
    "Placement", "slot bbox_min index bbox_max trans block zone light flags")


def placement_records(stream):
    """Every chunk-7 record up to the -1 terminator, as Placement tuples (the block
    index is not range-checked).  [] if the stream has no chunk 7 (missions that
    load a shared scene instead)."""
    span = chunk_span(stream, PLACE_CHUNK)
    if span is None:
        return []
    toff, tlen = span
    out = []
    for i in range(tlen // PLACE_REC_LEN):
        o = toff + i * PLACE_REC_LEN
        index = struct.unpack_from("<h", stream, o + 6)[0]
        if index == -1:
            break
        out.append(Placement(
            i, struct.unpack_from("<3h", stream, o), index,
            struct.unpack_from("<3h", stream, o + 0x08),
            struct.unpack_from("<3h", stream, o + 0x10),
            struct.unpack_from("<h", stream, o + 0x1e)[0],
            struct.unpack_from("<h", stream, o + 0x26)[0],
            stream[o + 0x28],
            struct.unpack_from("<H", stream, o + 0x2a)[0]))
    return out


def placements(stream, nblocks):
    """[(block, (tx, ty, tz), light)] for each placed section whose block index is
    in 0..nblocks-1 (world = block_local * SCALE + (tx, ty, tz))."""
    return [(p.block, p.trans, p.light) for p in placement_records(stream)
            if 0 <= p.block < nblocks]


# =====================================================================================
# Chunk-0 geometry model
# =====================================================================================
class MissionGeom:
    """The chunk-0 geometry of one mission stream, modelled as:
        header (raw 4 bytes, payload[0:4], the constant 0x80 word)
      + ordered list of pa_encode.GeomBlock (each a size-prefixed PA block)
      + tail (raw bytes after the last block to the end of chunk 0's payload).
    Re-serialization concatenates header + each block's to_bytes() + tail; the chunk
    [u32 len] header and every byte outside chunk 0 are preserved by splice_into()."""

    def __init__(self, stream):
        self.stream = bytes(stream)
        span = chunk_span(self.stream, GEOM_CHUNK)
        if span is None:
            raise ValueError("no chunk 0 (geometry) in stream")
        self.c0_off, self.c0_len = span          # payload offset/len within stream
        pay = self.stream[self.c0_off: self.c0_off + self.c0_len]
        self.header = pay[0:4]                    # constant 0x80 word
        # [(start,end)] of the size-prefixed blocks, within payload
        self.block_spans = _block_spans(pay)
        self.blocks = [pa_encode.parse(pay[a:b])  # [pa_encode.GeomBlock]
                       for a, b in self.block_spans]
        off = self.block_spans[-1][1] if self.block_spans else 4
        self.tail_off = off                       # within payload
        self.tail = pay[off:]                      # raw run of 0x00000004 stubs

    # ---- serialize ---------------------------------------------------------------
    def geom_bytes(self):
        """Re-emit the chunk-0 PAYLOAD (header + blocks + tail)."""
        out = bytearray(self.header)
        for b in self.blocks:
            out += b.to_bytes()
        out += self.tail
        if len(out) != self.c0_len:
            raise ValueError(f"chunk-0 re-encode len {len(out)} != original {self.c0_len}")
        return bytes(out)

    def splice_into(self):
        """Return the FULL mission stream with the re-encoded chunk-0 payload spliced
        back in place (every other byte preserved verbatim)."""
        out = bytearray(self.stream)
        out[self.c0_off: self.c0_off + self.c0_len] = self.geom_bytes()
        return bytes(out)

    # ---- edit helper -------------------------------------------------------------
    def translate(self, block_index, sub_index, dx, dy, dz):
        if not (0 <= block_index < len(self.blocks)):
            raise ValueError(f"block {block_index} out of range (have {len(self.blocks)})")
        return self.blocks[block_index].translate_sub(sub_index, dx, dy, dz)

    # ---- characterization --------------------------------------------------------
    def summary(self):
        live = sum(len([s for s in b.subs if not s.terminated]) for b in self.blocks)
        return (f"chunk0 payload @0x{self.c0_off:x} len {self.c0_len} (0x{self.c0_len:x}); "
                f"header={self.header.hex()}; {len(self.blocks)} geom blocks "
                f"({live} live sub-objs); tail {len(self.tail)}B")


def chunk11_info(stream):
    """Characterize chunk 11 (per-mission geom records) without decoding it fully.
    Returns (off, len, count, sublens, raw) or None."""
    span = chunk_span(stream, RECS_CHUNK)
    if span is None:
        return None
    off, ln = span
    pay = stream[off:off + ln]
    if ln < 4:
        return (off, ln, None, [], pay)
    count = struct.unpack_from("<I", pay, 0)[0]
    sublens = []
    o = 4
    for _ in range(min(count, 32)):
        if o + 4 > len(pay):
            break
        sl = struct.unpack_from("<I", pay, o)[0]
        sublens.append(sl)
        if sl < 4:                 # 0/short sublen -> stop characterizing
            break
        o += sl
    return (off, ln, count, sublens, pay)


def parse(stream):
    return MissionGeom(stream)


# =====================================================================================
# Self-test (the gate): every mission stream (FDAT entry 2N+1 present)
# =====================================================================================
def _stream_paths(fdat_dir):
    out = []
    for f in sorted(os.listdir(fdat_dir)):
        if not f.endswith(".bin"):
            continue
        idx = int(f[:3])
        if idx % 2 == 1:           # odd = chunk stream (2N+1)
            out.append((idx, os.path.join(fdat_dir, f)))
    return out


def _looks_like_geom_stream(buf):
    """Has a chunk 0 whose payload[+4] is a size-prefixed block."""
    span = chunk_span(buf, GEOM_CHUNK)
    if span is None:
        return False
    off, ln = span
    if ln < 16:
        return False
    sz = struct.unpack_from("<I", buf, off + 4)[0]
    return 12 <= sz <= ln


def selftest(fdat_dir):
    streams = _stream_paths(fdat_dir)
    print(f"=== mission geometry round-trip self-test ({fdat_dir}) ===")
    print(f"{'entry':>5} {'mission':>7} {'blocks':>6} {'sub-ok':>6}  result")
    tot = ok = 0
    blk_tot = blk_ok = 0
    failures = []
    for idx, path in streams:
        with open(path, "rb") as f:
            buf = f.read()
        if len(buf) < 64 or not _looks_like_geom_stream(buf):
            continue
        mission = (idx - 1) // 2
        tot += 1
        try:
            mg = parse(buf)
            # 1. chunk-0 payload re-encodes byte-exact
            geom_ok = mg.geom_bytes() == buf[mg.c0_off: mg.c0_off + mg.c0_len]
            # 2. per-block byte-exact (and count them)
            nblk = len(mg.blocks)
            nok = 0
            for b in mg.blocks:
                blk_tot += 1
                if b.to_bytes() == b.orig:
                    nok += 1
                    blk_ok += 1
            # 3. splicing back yields the ORIGINAL whole stream
            splice_ok = mg.splice_into() == buf
            entry_ok = geom_ok and splice_ok and (nok == nblk)
        except Exception as ex:                       # noqa: BLE001
            entry_ok = False
            nblk = nok = 0
            failures.append((idx, f"EXC {type(ex).__name__}: {ex}"))
        if entry_ok:
            ok += 1
            flag = "OK"
        else:
            if not any(fp[0] == idx for fp in failures):
                failures.append((idx, "mismatch"))
            flag = "*** FAIL"
        print(f"{idx:>5} {mission:>7} {nblk:>6} {nok:>6}  {flag}")
    print(f"\n  -> {ok}/{tot} mission streams round-trip byte-exact "
          f"(chunk-0 geom + splice)")
    print(f"  -> {blk_ok}/{blk_tot} individual chunk-0 geometry blocks byte-exact")
    if failures:
        print(f"\n  {len(failures)} failing stream(s) (first 20):")
        for idx, why in failures[:20]:
            print(f"    entry {idx:03d}: {why}")
    return ok == tot, ok, tot


# =====================================================================================
# Dump / edit demo (single mission)
# =====================================================================================
def dump(path):
    with open(path, "rb") as f:
        buf = f.read()
    print(f"=== {path} ===")
    mg = parse(buf)
    print("  " + mg.summary())
    for bi, b in enumerate(mg.blocks):
        live = [s for s in b.subs if not s.terminated]
        verts = sum(s.vtx_cnt for s in live)
        prims = sum(len(s.prims) for s in live)
        print(f"    block{bi:2d}: len={b.length:6d}  {b.sub_count} sub-objs "
              f"({len(live)} live)  {verts} verts  {prims} prims")
    info = chunk11_info(buf)
    if info:
        off, ln, count, sublens, _ = info
        print(f"  chunk11 (per-mission geom records) @0x{off:x} len {ln}: "
              f"count={count}  first sublens={sublens}")
        print("    (round-tripped as a raw span; see scratch/re/mission_geom_notes.md)")


def edit_demo(path, block_index, sub_index, dx, dy, dz):
    with open(path, "rb") as f:
        buf = f.read()
    print(f"=== {path} : edit demo ===")
    mg = parse(buf)
    print("  " + mg.summary())
    # baseline round-trip
    base_ok = mg.splice_into() == buf
    print(f"  baseline splice round-trip: {'BYTE-EXACT' if base_ok else 'MISMATCH'}")
    # capture original verts of the target sub-object
    orig = parse(buf)
    s0 = next(s for s in orig.blocks[block_index].subs if s.index == sub_index)
    print(f"\n  EDIT: translate block{block_index} sub{sub_index} by ({dx},{dy},{dz})")
    s = mg.translate(block_index, sub_index, dx, dy, dz)
    edited_stream = mg.splice_into()
    print(f"    edited stream len={len(edited_stream)} (== {len(buf)}: "
          f"{len(edited_stream) == len(buf)})")
    # re-parse edited stream, confirm the vertices moved exactly as intended
    mg2 = parse(edited_stream)
    s2 = next(x for x in mg2.blocks[block_index].subs if x.index == sub_index)
    clamp = pa_encode._clamp_s16
    moved_ok = all(
        v2.x == clamp(v0.x + dx) and v2.y == clamp(v0.y + dy) and v2.z == clamp(v0.z + dz)
        for v0, v2 in zip(s0.verts, s2.verts))
    diff = _diff_regions(buf, edited_stream)
    bs = mg.block_spans[block_index]
    print(f"    re-parsed edited stream: vertices match expected: {moved_ok}")
    print(f"    changed byte region(s) in stream: {diff}")
    print(f"    (expected inside block{block_index}'s chunk-0 span "
          f"0x{mg.c0_off + bs[0]:x}..0x{mg.c0_off + bs[1]:x}, "
          f"sub{sub_index} vtx pool len {len(s.vtx_raw)}B)")


def _diff_regions(a, b):
    regions = []
    n = min(len(a), len(b))
    i = 0
    while i < n:
        if a[i] != b[i]:
            j = i
            while j < n and a[j] != b[j]:
                j += 1
            regions.append((f"0x{i:x}", f"0x{j:x}"))
            i = j
        else:
            i += 1
    return regions


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--fdat", help="dir of extracted FDAT entries (NNN.bin)")
    ap.add_argument("--mission", type=int, help="mission number (uses entry 2N+1)")
    ap.add_argument("--stream", help="path to a single chunk-stream entry (NNN.bin)")
    ap.add_argument("--translate", nargs=5, type=int,
                    metavar=("BLOCK", "SUB", "DX", "DY", "DZ"),
                    help="edit demo: translate chunk-0 BLOCK's sub-object SUB by (DX,DY,DZ)")
    ap.add_argument("--selftest", action="store_true",
                    help="round-trip EVERY mission stream's chunk-0 geometry (the gate)")
    args = ap.parse_args()

    if args.selftest:
        fdat = args.fdat or os.path.join(
            HERE, "disc_map", "extracted", "FDAT_T", "entries")
        ok, _, _ = selftest(fdat)
        sys.exit(0 if ok else 1)

    if args.stream:
        path = args.stream
    elif args.fdat is not None and args.mission is not None:
        path = os.path.join(args.fdat, f"{2 * args.mission + 1:03d}.bin")
    else:
        ap.error("give --selftest, or --stream PATH, or --fdat DIR --mission N")

    if args.translate:
        bi, si, dx, dy, dz = args.translate
        edit_demo(path, bi, si, dx, dy, dz)
    else:
        dump(path)


if __name__ == "__main__":
    main()
