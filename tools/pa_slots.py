#!/usr/bin/env python3
"""
pa_slots.py — cross-file slot-structure analysis of the 72 PA##.T geometry packs.

PA files share a fixed "slot" skeleton: a given .T entry index holds the same KIND
of object in every PA file. Some slots are byte-identical everywhere (shared common
assets); others vary per stage (the real per-mission geometry). This tool builds the
slot × file matrix and classifies every slot.

Outputs:
  disc_map/pa_slots.csv   — slot, present_in, class, subs, faces, distinct_md5s, sample
  prints a summary table.

Classes:
  SHARED   — same md5 in every file that has the slot (one identical mesh reused).
  FIXED    — same (sub-count, face-count) in all files but bytes differ (a fixed
             role / template filled with per-stage data).
  VARIABLE — sub/face counts differ across files (genuinely stage-specific).

Usage: python3 tools/pa_slots.py
"""
import json, struct, hashlib, os, sys, re, csv, collections

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(HERE, "tools"))
import pa_obj as PA   # reuse the validated decoder


def pa_files():
    fm = json.load(open(os.path.join(HERE, "disc_map", "disc_files.json")))
    return [r for r in fm["files"] if re.search(r"/PA\d{2}\.T$", r.get("path", ""))]


def block_info(entry):
    """(byte_size, sub_count, face_count, md5) for a geometry block, else None."""
    if len(entry) < 12 or struct.unpack_from("<I", entry, 0)[0] != len(entry):
        return None
    _, subs = PA.parse_subobjects(entry)
    nf = 0
    for s in subs:
        prims, _, _ = PA.read_prims(entry, s["prim_off"], s["prim_cnt"])
        nf += len(prims)
    return (len(entry), len(subs), nf, hashlib.md5(entry).hexdigest()[:10])


def main():
    files = pa_files()
    names = [os.path.basename(r["path"]) for r in files]
    # slot -> {filename: info}
    matrix = collections.defaultdict(dict)
    for r in files:
        nm = os.path.basename(r["path"])
        for i, e in enumerate(PA.load_container(r["path"])):
            info = block_info(e)
            if info:
                matrix[nm][i] = info       # but keyed by slot below
    # rebuild keyed by slot
    by_slot = collections.defaultdict(dict)
    for r in files:
        nm = os.path.basename(r["path"])
        for i, e in enumerate(PA.load_container(r["path"])):
            info = block_info(e)
            if info:
                by_slot[i][nm] = info

    rows = []
    cls_count = collections.Counter()
    for slot in sorted(by_slot):
        cells = by_slot[slot]
        present = len(cells)
        subs = {v[1] for v in cells.values()}
        faces = {v[2] for v in cells.values()}
        md5s = {v[3] for v in cells.values()}
        if len(md5s) == 1:
            klass = "SHARED"
        elif len(subs) == 1 and len(faces) == 1:
            klass = "FIXED"
        else:
            klass = "VARIABLE"
        cls_count[klass] += 1
        sample = next(iter(cells.values()))
        rows.append(dict(slot=slot, present_in=present, klass=klass,
                         subs=sample[1], faces=sample[2],
                         distinct_md5s=len(md5s),
                         face_set=",".join(map(str, sorted(faces)[:6]))))

    out = os.path.join(HERE, "disc_map", "pa_slots.csv")
    with open(out, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=["slot", "present_in", "klass", "subs",
                                          "faces", "distinct_md5s", "face_set"])
        w.writeheader()
        w.writerows(rows)

    # ---- file categorisation by slot signature ----
    BIG = set(range(10, 25))
    sigs = {}
    for r in files:
        nm = os.path.basename(r["path"]).replace(".T", "")
        sigs[nm] = set(by_slot[s].keys() and s for s in by_slot if os.path.basename(r["path"]) in by_slot[s])
    # which big-mesh slots each file fills (stride-3 phase)
    cat = collections.defaultdict(list)
    for nm, s in sigs.items():
        big = sorted(s & BIG)
        has_big = bool(big)
        phase = (big[0] - 10) % 3 if big else "-"
        cat[("BIG" if has_big else "stage-only", phase)].append(nm)

    universal = [r for r in rows if r["present_in"] == len(files)]
    print(f"{len(files)} PA files;  {len(rows)} distinct geometry slots")
    print(f"slots present in ALL {len(files)} files: {len(universal)}")
    print(f"slot classes: {dict(cls_count)}")
    print(f"  SHARED   (identical mesh in every file): "
          f"{sum(1 for r in rows if r['klass']=='SHARED')}")
    print(f"  FIXED    (same role, per-stage data):    "
          f"{sum(1 for r in rows if r['klass']=='FIXED')}")
    print(f"  VARIABLE (stage-specific structure):     "
          f"{sum(1 for r in rows if r['klass']=='VARIABLE')}")
    print(f"\nfile categories (by big-mesh slots e10-e24 + stride-3 phase):")
    for k, v in sorted(cat.items(), key=lambda kv: (-len(kv[1]), str(kv[0]))):
        print(f"  {str(k):<20s} {len(v):>2d} files: {', '.join(sorted(v)[:10])}"
              f"{'…' if len(v) > 10 else ''}")
    print(f"\ncsv -> {out}")
    print("\nslot  present  class     subs faces  #md5  facecounts")
    for r in rows:
        if r["slot"] <= 25 or r["klass"] != "FIXED":
            print(f"  e{r['slot']:<3d} {r['present_in']:>3d}/{len(files)}  "
                  f"{r['klass']:<8s} {r['subs']:>3d} {r['faces']:>5d}   "
                  f"{r['distinct_md5s']:>3d}   {r['face_set']}")
    return rows


if __name__ == "__main__":
    main()
