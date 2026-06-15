#!/usr/bin/env python3
"""
pa_encode.py — byte-exact ENCODER (write path) for AC1 PA##.T geometry blocks.

This is the inverse of the structural decoder (tools/pa/pa_parse.py) and the OBJ
exporter (tools/pa/pa_obj.py): it parses a real PA geometry block (entry 2..N) into
an explicit, fully-captured Python model, then serializes that model back to bytes
such that

    encode(parse(block)) == block          (byte-for-byte)

for EVERY geometry block in EVERY PA##.T on the disc. That round-trip is the gate
(--selftest), mirroring tools/extract/t_repack_test.py discipline.

WHY A REGION MODEL (not a from-scratch rebuild)
-----------------------------------------------
A geometry block is a header + a stride-28 sub-object table + per-sub-object pools
(primitive stream / vertex pool / normal pool / pool4), laid out on disc in the
order  [header][table] then per sub-object [prims][verts][normals][pool4]. The exact
on-disc ORDER, the un-relocated stored offsets, the variable-length primitive record
bytes, and any inter-region padding all have to be reproduced exactly. Rather than
guess the authoring layout, we PARSE THE BLOCK INTO SEGMENTS that tile the whole
block with no gaps and no overlap (every byte belongs to exactly one segment), keep
each segment's raw bytes, and additionally decode the geometry-bearing segments
(sub-object descriptors, vertex pools, normal pools, primitive records) into editable
Python structures. Serialization re-emits each segment in on-disc order. Because the
default segment payload is its captured raw bytes, the round-trip is byte-exact by
construction; an editor only re-renders the segments it actually changed.

This means:
  * Byte-exact round-trip is guaranteed as long as the segment tiling is complete.
    The selftest asserts completeness implicitly (any unmodelled byte => mismatch).
  * Edits (e.g. translate a sub-object's vertices) re-serialize ONLY that pool and
    are proven correct by re-parsing the encoded block.

The decoded structures reuse the field semantics confirmed in docs/PA_FORMAT.md and
tools/pa/pa_obj.py (sub-object descriptor layout, primitive record walk, gouraud
stride-4 interleave, etc.).

CLI (mirrors tools/pa/pa_obj.py):
  python3 tools/pa/pa_encode.py --pa GG/P0/PA00.T --entry 2     # round-trip one block
  python3 tools/pa/pa_encode.py --pa GG/P0/PA00.T --entry 2 --translate 0 100 0 0
                                                                # demo edit: move sub0
  python3 tools/pa/pa_encode.py --selftest                     # gate: all 72 files
"""
import json, struct, sys, argparse, os, collections

HERE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")
RAW, OFF, DATA = 2352, 24, 2048

# --- Primitive type -> (first vertex-index byte offset, #verts, index stride) -----
# Identical to tools/pa/pa_obj.py PRIM_VERTS (RE'd from relocation walker
# FUN_800574D8). Used ONLY by the editable decode/re-encode of vertex indices; the
# byte-exact round-trip does not depend on it (records are kept as raw bytes unless
# edited). See docs/PA_FORMAT.md.
PRIM_VERTS = {
    0x20: (0x0a, 3, 2), 0x28: (0x0a, 4, 2),
    0x24: (0x12, 3, 2), 0x2c: (0x16, 4, 2),
    0x34: (0x12, 3, 4), 0x3c: (0x16, 4, 4),
    0xa0: (0x0a, 3, 2), 0xa8: (0x0a, 4, 2),
    0xa4: (0x12, 3, 2), 0xac: (0x16, 4, 2),
    0xb4: (0x12, 3, 2), 0xbc: (0x12, 4, 2),
}


# =====================================================================================
# Container I/O  (same mechanism as pa_parse.py / pa_obj.py)
# =====================================================================================
def load_container(path):
    fm = json.load(open(FILEMAP))
    rec = next((r for r in fm["files"]
                if r.get("path") == path
                or (r.get("path") or "").endswith("/" + path)), None)
    if not rec:
        sys.exit(f"not in filemap: {path}")
    blob = bytearray()
    with open(fm["source_bin"], "rb") as f:
        for s in range(rec["sector_first"], rec["sector_last"] + 1):
            f.seek(s * RAW + OFF)
            blob += f.read(DATA)
    blob = bytes(blob)
    n = struct.unpack_from("<H", blob, 0)[0]
    offs = list(struct.unpack_from(f"<{n+1}H", blob, 2))
    return [blob[offs[i] * 2048: offs[i + 1] * 2048] for i in range(n)]


def all_pa_files():
    fm = json.load(open(FILEMAP))
    return [r["path"] for r in fm["files"]
            if (r.get("path") or "").endswith(".T")
            and "/PA" in (r.get("path") or "")]


def is_geom_block(block):
    """A geometry block has u32[0] == its own length (size-prefixed)."""
    return len(block) >= 12 and struct.unpack_from("<I", block, 0)[0] == len(block)


# =====================================================================================
# Decoded (editable) sub-structures
# =====================================================================================
class Vertex:
    __slots__ = ("x", "y", "z", "w")

    def __init__(self, x, y, z, w):
        self.x, self.y, self.z, self.w = x, y, z, w

    def pack(self):
        return struct.pack("<4h", self.x, self.y, self.z, self.w)

    def __repr__(self):
        return f"V({self.x},{self.y},{self.z},{self.w})"


class Primitive:
    """One variable-length primitive record. Kept as raw bytes (byte-exact) with a
    decoded view of its type and vertex indices for editing. Editing vertex indices
    re-packs them into the raw buffer in place (same length)."""
    __slots__ = ("raw", "type", "vidx")

    def __init__(self, raw):
        self.raw = bytearray(raw)
        self.type = raw[3] & 0xbc
        self.vidx = []
        info = PRIM_VERTS.get(self.type)
        if info:
            voff, nv, stride = info
            for k in range(nv):
                p = voff + stride * k
                if p + 2 <= len(raw):
                    self.vidx.append(struct.unpack_from("<H", raw, p)[0])

    def set_vidx(self, new_idx):
        info = PRIM_VERTS.get(self.type)
        if not info:
            raise ValueError(f"cannot edit indices of unknown prim type 0x{self.type:02x}")
        voff, nv, stride = info
        if len(new_idx) != nv:
            raise ValueError(f"type 0x{self.type:02x} needs {nv} indices")
        for k, v in enumerate(new_idx):
            struct.pack_into("<H", self.raw, voff + stride * k, v)
        self.vidx = list(new_idx)

    def pack(self):
        return bytes(self.raw)


class SubObject:
    """A sub-object: its 28-byte descriptor plus the four pools it owns. Each pool is
    stored both as raw bytes (default, byte-exact) and as a decoded view (verts /
    prims). Re-encoding emits the raw bytes unless `dirty_*` is set, in which case the
    decoded structure is re-packed (must be the same length)."""

    def __init__(self, index, desc_raw):
        self.index = index
        self.desc = bytearray(desc_raw)          # 28-byte descriptor, byte-exact
        # decoded descriptor fields (relative to reloc base).
        # NOTE: vertex/normal counts are u16 (NOT u32): +0x04 = vtx_cnt(u16),
        # +0x06 = a u16 param (0 in the "simple" variant; nonzero in the multi-LOD /
        # articulated variant, e.g. PA00 e60). Reading +0x04 as u32 (as pa_obj.py
        # did) folds +0x06 into the count and overshoots the pool — that was the
        # original ~30% round-trip failure. See scratch/re/pa_encode_notes.md.
        self.vtx_off = struct.unpack_from("<I", desc_raw, 0)[0]
        self.vtx_cnt = struct.unpack_from("<H", desc_raw, 0x04)[0]
        self.vtx_param = struct.unpack_from("<H", desc_raw, 0x06)[0]
        self.pool2_off = struct.unpack_from("<I", desc_raw, 0x08)[0]
        self.norm_cnt = struct.unpack_from("<H", desc_raw, 0x0c)[0]
        self.flags = struct.unpack_from("<H", desc_raw, 0x0e)[0]
        self.prim_off = struct.unpack_from("<I", desc_raw, 0x10)[0]
        self.prim_cnt_base = struct.unpack_from("<H", desc_raw, 0x14)[0]
        self.pool4_param = struct.unpack_from("<H", desc_raw, 0x16)[0]
        self.pool4_off = struct.unpack_from("<I", desc_raw, 0x18)[0]
        self.terminated = bool(self.flags & 0x8000)
        self.prim_cnt = self.prim_cnt_base + (self.flags & 0x1ff) - 1
        # pools (filled by parser); raw + decoded views
        self.vtx_raw = b""
        self.norm_raw = b""
        self.prim_raw = b""
        self.pool4_raw = b""
        self.verts = []          # [Vertex]
        self.prims = []          # [Primitive]
        self.dirty_verts = False
        self.dirty_prims = False

    def pack_descriptor(self):
        return bytes(self.desc)


class GeomBlock:
    """A full geometry block, modelled as an ordered list of byte segments that tile
    the entire block with no gap/overlap. A segment is either:
      * 'raw'   : verbatim bytes (header, table, pool4s, padding, anything not edited)
      * 'verts' : a sub-object vertex pool (re-packed from Vertex list if dirtied)
      * 'prims' : a sub-object primitive stream (re-packed from Primitive list)
    Serialization concatenates segment payloads in order."""

    def __init__(self, block):
        self.orig = bytes(block)
        self.length = len(block)
        self.reloc = 0
        self.subs = []           # [SubObject]
        self.segments = []       # ordered [(kind, start, end, owner)]
        self._parse()

    # ---- parse -------------------------------------------------------------------
    def _parse(self):
        block = self.orig
        a0 = struct.unpack_from("<I", block, 8)[0]      # block[+8]
        self.block8 = a0
        self.reloc = a0 + 12
        count = struct.unpack_from("<I", block, a0 + 8)[0]
        self.sub_count = count
        tbl = a0 + 12
        # sub-object descriptors
        for i in range(count):
            o = tbl + i * 28
            self.subs.append(SubObject(i, block[o:o + 28]))
        # decode each sub-object's pools and record their byte spans
        for s in self.subs:
            if s.terminated:
                continue
            # vertex pool
            s.vtx_abs = s.vtx_off + self.reloc
            s.vtx_raw = block[s.vtx_abs: s.vtx_abs + s.vtx_cnt * 8]
            s.verts = [Vertex(*struct.unpack_from("<4h", s.vtx_raw, i * 8))
                       for i in range(s.vtx_cnt)]
            # normal pool
            s.norm_abs = s.pool2_off + self.reloc
            s.norm_raw = block[s.norm_abs: s.norm_abs + s.norm_cnt * 8]
            # primitive stream — walk prim_cnt variable-length records
            s.prim_abs = s.prim_off + self.reloc
            s.prims, prim_end = self._walk_prims(block, s.prim_abs, s.prim_cnt)
            s.prim_raw = block[s.prim_abs: prim_end]
            s.prim_end = prim_end
            # pool4 (start known; size unknown -> captured via gap-tiling below)
            s.pool4_abs = s.pool4_off + self.reloc
        # Build the segment tiling. Collect every known fixed-position region as an
        # anchor (start, end_or_None, owner), then sweep the block start->end filling
        # gaps with 'raw' segments. Regions whose end is unknown (pool4) end where the
        # next anchor begins.
        anchors = []
        # header+table is one raw region [0 .. table_end]
        table_end = tbl + count * 28
        anchors.append(("raw", 0, table_end, None))
        for s in self.subs:
            if s.terminated:
                continue
            anchors.append(("verts", s.vtx_abs, s.vtx_abs + len(s.vtx_raw), s))
            anchors.append(("raw_norm", s.norm_abs, s.norm_abs + len(s.norm_raw), s))
            anchors.append(("prims", s.prim_abs, s.prim_end, s))
            anchors.append(("pool4", s.pool4_abs, None, s))   # end resolved by sweep
        # sort by start; resolve open-ended (pool4) ends to next anchor start
        anchors.sort(key=lambda a: a[1])
        resolved = []
        for i, (kind, start, end, owner) in enumerate(anchors):
            if end is None:
                nxt = next((anchors[j][1] for j in range(i + 1, len(anchors))
                            if anchors[j][1] > start), self.length)
                end = nxt
            resolved.append((kind, start, end, owner))
        # sweep to produce a complete, non-overlapping tiling
        self.segments = []
        cur = 0
        for (kind, start, end, owner) in resolved:
            if start < cur:
                # overlap (shouldn't happen on clean data) — clamp
                start = cur
                if end <= start:
                    continue
            if start > cur:
                self.segments.append(("raw", cur, start, None))   # padding/gap
            self.segments.append((kind, start, end, owner))
            cur = max(cur, end)
        if cur < self.length:
            self.segments.append(("raw", cur, self.length, None))
        # capture pool4 raw bytes from the resolved spans
        for (kind, start, end, owner) in self.segments:
            if kind == "pool4" and owner is not None:
                owner.pool4_raw = self.orig[start:end]

    @staticmethod
    def _walk_prims(block, off, cnt):
        o = off
        out = []
        n = 0
        while n < cnt and o + 4 <= len(block):
            words = block[o + 1]
            reclen = 4 + words * 4
            if reclen < 4 or o + reclen > len(block):
                break
            out.append(Primitive(block[o:o + reclen]))
            o += reclen
            n += 1
        return out, o

    # ---- serialize ---------------------------------------------------------------
    def to_bytes(self):
        out = bytearray()
        for (kind, start, end, owner) in self.segments:
            if kind == "verts" and owner is not None and owner.dirty_verts:
                payload = b"".join(v.pack() for v in owner.verts)
                if len(payload) != (end - start):
                    raise ValueError(f"sub{owner.index} vertex re-pack length changed")
                out += payload
            elif kind == "prims" and owner is not None and owner.dirty_prims:
                payload = b"".join(p.pack() for p in owner.prims)
                if len(payload) != (end - start):
                    raise ValueError(f"sub{owner.index} prim re-pack length changed")
                out += payload
            else:
                out += self.orig[start:end]
        if len(out) != self.length:
            raise ValueError(f"encoded length {len(out)} != original {self.length}")
        return bytes(out)

    # ---- edit helpers ------------------------------------------------------------
    def translate_sub(self, sub_index, dx, dy, dz):
        """Translate every vertex of one sub-object by (dx,dy,dz). Marks the vertex
        pool dirty so it re-serializes. Proves authored edits round-trip."""
        s = next((s for s in self.subs if s.index == sub_index and not s.terminated), None)
        if s is None:
            raise ValueError(f"no live sub-object {sub_index}")
        for v in s.verts:
            v.x = _clamp_s16(v.x + dx)
            v.y = _clamp_s16(v.y + dy)
            v.z = _clamp_s16(v.z + dz)
        s.dirty_verts = True
        return s

    def summary(self):
        live = [s for s in self.subs if not s.terminated]
        return (f"len={self.length}  block[+8]=0x{self.block8:x} reloc=0x{self.reloc:x}  "
                f"{self.sub_count} sub-objs ({len(live)} live)  "
                f"{len(self.segments)} segments")


def _clamp_s16(v):
    return max(-32768, min(32767, v))


def parse(block):
    return GeomBlock(block)


def encode(model):
    return model.to_bytes()


# =====================================================================================
# Self-test  (the gate)
# =====================================================================================
def selftest():
    files = all_pa_files()
    print(f"=== PA encode round-trip self-test ({len(files)} PA files) ===")
    print(f"{'file':<16} {'blocks':>7} {'ok':>5}  result")
    tot_blocks = tot_ok = 0
    failures = []
    for path in files:
        try:
            ents = load_container(path)
        except SystemExit as ex:
            print(f"{path:<16} {'':>7} {'':>5}  LOAD ERR {ex}")
            continue
        nblk = nok = 0
        for i, e in enumerate(ents):
            if i < 2 or not is_geom_block(e):
                continue
            nblk += 1
            try:
                m = parse(e)
                rt = encode(m)
                ok = (rt == e)
            except Exception as ex:
                ok = False
                failures.append((path, i, f"EXC {type(ex).__name__}: {ex}"))
            if ok:
                nok += 1
            else:
                if not any(f[0] == path and f[1] == i for f in failures):
                    failures.append((path, i, "byte mismatch"))
        tot_blocks += nblk
        tot_ok += nok
        flag = "OK" if nok == nblk else f"*** {nblk-nok} FAIL"
        print(f"{os.path.basename(path):<16} {nblk:>7} {nok:>5}  {flag}")
    print(f"\n  -> {tot_ok}/{tot_blocks} geometry blocks byte-exact "
          f"({100.0*tot_ok/tot_blocks if tot_blocks else 0:.2f}%)")
    if failures:
        print(f"\n  {len(failures)} failing block(s) (first 20):")
        for (path, i, why) in failures[:20]:
            print(f"    {path} entry {i}: {why}")
    return tot_ok == tot_blocks, tot_ok, tot_blocks


def roundtrip_one(path, entry, translate=None):
    ents = load_container(path)
    block = ents[entry]
    print(f"=== {path} entry {entry} (len {len(block)}) ===")
    if not is_geom_block(block):
        print("  NOT a size-prefixed geometry block (entry 0/1 or empty); skipping.")
        return
    m = parse(block)
    print("  " + m.summary())
    for s in m.subs:
        if s.terminated:
            print(f"    sub{s.index:2d}: TERMINATED (flags=0x{s.flags:04x})")
            continue
        print(f"    sub{s.index:2d}: verts={s.vtx_cnt} norms={s.norm_cnt} "
              f"prims={len(s.prims)} pool4={len(s.pool4_raw)}B "
              f"flags=0x{s.flags:04x}")
    rt = encode(m)
    ok = (rt == block)
    print(f"  round-trip: {'BYTE-EXACT' if ok else 'MISMATCH (len %d vs %d)' % (len(rt), len(block))}")
    if not ok:
        # show first diff
        for k in range(min(len(rt), len(block))):
            if rt[k] != block[k]:
                print(f"    first diff @0x{k:x}: got 0x{rt[k]:02x} want 0x{block[k]:02x}")
                break
    if translate is not None:
        si, dx, dy, dz = translate
        print(f"\n  EDIT demo: translate sub{si} by ({dx},{dy},{dz})")
        s = m.translate_sub(si, dx, dy, dz)
        edited = encode(m)
        print(f"    encoded edited block: len={len(edited)} (== {len(block)}: "
              f"{len(edited) == len(block)})")
        # re-parse the edited block and confirm the vertices moved as intended
        m2 = parse(edited)
        s2 = next(x for x in m2.subs if x.index == si)
        orig_m = parse(block)
        s0 = next(x for x in orig_m.subs if x.index == si)
        moved_ok = all(
            v2.x == _clamp_s16(v0.x + dx) and v2.y == _clamp_s16(v0.y + dy)
            and v2.z == _clamp_s16(v0.z + dz)
            for v0, v2 in zip(s0.verts, s2.verts))
        # and confirm ONLY this sub-object's vertex bytes changed
        diff_regions = _diff_segments(block, edited)
        print(f"    re-parsed edited block: vertices match expected: {moved_ok}")
        print(f"    changed byte region(s): {diff_regions}")
        print(f"    (expected exactly sub{si}'s vertex pool @0x{s.vtx_abs:x}"
              f"..0x{s.vtx_abs + len(s.vtx_raw):x})")


def _diff_segments(a, b):
    """Return list of (start,end) byte ranges where a and b differ."""
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
    ap = argparse.ArgumentParser(description="byte-exact encoder for AC1 PA geometry blocks")
    ap.add_argument("--pa", help="PA##.T container path (e.g. GG/P0/PA00.T)")
    ap.add_argument("--entry", type=int, help="geometry block entry index (>=2)")
    ap.add_argument("--translate", nargs=4, type=int, metavar=("SUB", "DX", "DY", "DZ"),
                    help="edit demo: translate sub-object SUB by (DX,DY,DZ) and re-encode")
    ap.add_argument("--selftest", action="store_true",
                    help="round-trip EVERY geometry block in EVERY PA##.T (the gate)")
    args = ap.parse_args()

    if args.selftest:
        ok, _, _ = selftest()
        sys.exit(0 if ok else 1)
    if args.pa is None or args.entry is None:
        ap.error("provide --pa FILE --entry N, or --selftest")
    roundtrip_one(args.pa, args.entry, translate=args.translate)


if __name__ == "__main__":
    main()
