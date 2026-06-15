#!/usr/bin/env python3
"""spawn_edit.py - read/modify a chunk-12 spawn record inside a mission chunk
stream (FDAT entry 2N+1).

Target: SLUS-01323 (v1.1).  Companion to mission_parse.py (dumps the same table)
and docs/MISSION_SYSTEM.md.  Operates on the de-sectored stream bytes; pair with
tools/extract/t_repack.py (replace_entry) + tools/disc/reinject.py to land an
edit on disc.

Spawn record = 20 int16 halfwords (CONFIRMED layout, see mission_parse.py):
  hw0,hw1,hw2 = X,Y,Z   hw3 = geom block (-1=none)   hw5 = rotation   hw7 = type
  hw8..hw19   = per-type params (HP/AI/range; semantics in progress, Phase 1)

Field aliases: x=hw0 y=hw1 z=hw2 blk=hw3 rot=hw5 type=hw7 (else hwN).
"""
import struct, sys

SPAWN_CHUNK = 12
SPAWN_REC_LEN = 40   # 20 * int16
NREC = 256

ALIAS = {"x": 0, "y": 1, "z": 2, "blk": 3, "rot": 5, "type": 7}


def walk_chunks(buf, limit=64):
    off = idx = 0
    n = len(buf)
    while off + 4 <= n and idx < limit:
        ln = struct.unpack_from("<I", buf, off)[0]
        if ln == 0 or off + 4 + ln > n:
            return
        yield idx, off, ln
        off += ln + 4
        idx += 1


def spawn_payload_offset(buf):
    """Byte offset of the chunk-12 payload within the stream, or None."""
    for idx, off, ln in walk_chunks(buf):
        if idx == SPAWN_CHUNK and ln:
            return off + 4
    return None


def _field_index(field):
    if isinstance(field, int):
        return field
    if field in ALIAS:
        return ALIAS[field]
    if field.startswith("hw"):
        return int(field[2:])
    raise ValueError(f"unknown field: {field}")


def get_field(buf, rec, field):
    base = spawn_payload_offset(buf)
    if base is None:
        raise ValueError("no chunk-12 spawn table in stream")
    h = _field_index(field)
    return struct.unpack_from("<h", buf, base + rec * SPAWN_REC_LEN + h * 2)[0]


def set_field(buf, rec, field, value):
    """Return a new bytes with spawn[rec].field set to value (signed int16)."""
    base = spawn_payload_offset(buf)
    if base is None:
        raise ValueError("no chunk-12 spawn table in stream")
    if not (0 <= rec < NREC):
        raise ValueError(f"record {rec} out of range 0..{NREC-1}")
    h = _field_index(field)
    out = bytearray(buf)
    struct.pack_into("<h", out, base + rec * SPAWN_REC_LEN + h * 2, int(value))
    return bytes(out)


def main():
    import argparse
    ap = argparse.ArgumentParser(description="read/edit a chunk-12 spawn field")
    ap.add_argument("--stream", required=True, help="chunk-stream entry (NNN.bin)")
    ap.add_argument("--rec", type=int, required=True)
    ap.add_argument("--field", required=True, help="x|y|z|blk|rot|type|hwN")
    ap.add_argument("--set", type=int, help="new signed int16 value; omit to read")
    ap.add_argument("-o", "--out", help="output stream path (with --set)")
    args = ap.parse_args()
    buf = open(args.stream, "rb").read()
    if args.set is None:
        print(get_field(buf, args.rec, args.field))
        return
    old = get_field(buf, args.rec, args.field)
    new_buf = set_field(buf, args.rec, args.field, args.set)
    out = args.out or args.stream
    open(out, "wb").write(new_buf)
    print(f"rec {args.rec} {args.field}: {old} -> {args.set}  written {out}")


if __name__ == "__main__":
    main()
