#!/usr/bin/env python3
"""
scan_text.py — find draw_string-compatible text inside extracted .T entries.

Consumes a toc.json produced by extract_t.py (+ the source .bin) and scans every
non-empty, non-TIM entry for translatable strings. AC1 menu/mission text is
rendered by draw_string (docs/REFERENCE.md §4): a byte walker over (mostly) ASCII /
Shift-JIS with inline escapes and a '>' (0x3E) terminator. Mission briefings in
MIS.T additionally use '/' (0x2F) as the final end-of-message marker and literal
'@d' / '%d' colour markup.

For each entry we emit, per detected message:
  container, entry, byte_off, char_len, encoding, terminator, renderable, sample

Plus a human-readable per-container dump (<name>_text.txt) for translators.

Usage:
  python3 tools/scan_text.py --toc disc_map/extracted/MIS_T/toc.json
  python3 tools/scan_text.py --toc .../toc.json --csv disc_map/text_inventory.csv --append
  python3 tools/scan_text.py --toc .../toc.json --self-test   # FDAT-201 gate helper
"""
import json, struct, re, os, sys, argparse, csv

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FILEMAP = os.path.join(HERE, "disc_map", "disc_files.json")
RAW, OFF, DATA = 2352, 24, 2048

# SJIS double-byte lead ranges (the JP leftover paths)
def is_sjis_lead(b): return (0x81 <= b <= 0x9f) or (0xe0 <= b <= 0xef)


def read_entry(bin_path, disc_first, e):
    out = bytearray()
    s0 = disc_first + e["sector_start"]
    with open(bin_path, "rb") as f:
        for s in range(s0, s0 + e["sector_count"]):
            f.seek(s * RAW + OFF)
            out += f.read(DATA)
    return bytes(out)


# Bytes allowed *inside* a printable text run (CR/LF/TAB are inline formatting).
_RUN_INNER = set(range(0x20, 0x7f)) | {0x0a, 0x0d, 0x09}


def split_messages(payload, min_run=4):
    """
    Extract messages as maximal runs of printable-ASCII (+ CR/LF/TAB) bytes, then
    split each run into page-messages on the '>' (0x3E) terminator and the MIS '/'
    end-marker. This is run-anchored: surrounding binary (code/geometry) can never
    leak into a message, which keeps the FDAT code+string overlay clean.

    Returns (byte_off, raw_bytes, terminator) tuples.
    """
    msgs = []
    n = len(payload)
    i = 0
    while i < n:
        if payload[i] not in _RUN_INNER:
            i += 1
            continue
        j = i
        while j < n and payload[j] in _RUN_INNER:
            j += 1
        run = payload[i:j]
        if len(run) >= min_run:
            # sub-split the run on terminators, keeping byte offsets accurate
            seg_start = 0
            k = 0
            while k < len(run):
                b = run[k]
                if b == 0x3e:  # '>'
                    msgs.append((i + seg_start, run[seg_start:k], ">"))
                    seg_start = k + 1
                elif b == 0x2f:  # '/'  — MIS end-marker (only at run end / before space)
                    nxt = run[k + 1] if k + 1 < len(run) else 0x00
                    if nxt in (0x00, 0x20) or k + 1 == len(run):
                        msgs.append((i + seg_start, run[seg_start:k], "/"))
                        seg_start = k + 1
                k += 1
            if seg_start < len(run):
                msgs.append((i + seg_start, run[seg_start:], ""))
        i = j
    return msgs


def clean(raw):
    """Strip leading/trailing padding (0x00/0xff) and trailing whitespace bytes."""
    raw = raw.strip(b"\x00\xff")
    return raw


def classify_encoding(raw):
    has_sjis = False
    i = 0
    while i < len(raw):
        b = raw[i]
        if is_sjis_lead(b) and i + 1 < len(raw):
            has_sjis = True
            i += 2
            continue
        i += 1
    hi = any(b >= 0x80 for b in raw)
    if has_sjis:
        return "sjis" if not all(b < 0x80 or is_sjis_lead(b) for b in raw) else "sjis"
    return "ascii" if not hi else "latin/raw"


def renderable(raw):
    """
    draw_string handles: printable ASCII, the documented inline escapes
    (\\r,\\n,;,{,},^,~,@,%), and SJIS double-byte. A message is 'renderable' if
    every byte is ASCII-printable, a known control/escape, or part of an SJIS pair.
    Returns (bool, note).
    """
    ok = True
    note = ""
    i = 0
    KNOWN_CTRL = {0x0a, 0x0d, 0x09, 0x01, 0x02, 0x03}  # LF/CR/TAB + colour ctrl seen in MIS
    while i < len(raw):
        b = raw[i]
        if 0x20 <= b < 0x7f:
            i += 1
        elif b in KNOWN_CTRL:
            i += 1
        elif is_sjis_lead(b) and i + 1 < len(raw):
            note = "has-sjis"
            i += 2
        elif b in (0x00, 0xff):
            i += 1
        else:
            ok = False
            note = f"byte 0x{b:02x}"
            break
    return ok, note


def display(raw):
    s = []
    for b in raw:
        if b in (0x0a, 0x0d):
            s.append("\\n")
        elif 0x20 <= b < 0x7f:
            s.append(chr(b))
        elif b in (0x00, 0xff):
            pass
        else:
            s.append(f"<{b:02x}>")
    return "".join(s)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--toc", required=True)
    ap.add_argument("--bin", help="override source .bin (default from filemap)")
    ap.add_argument("--csv", default=os.path.join(HERE, "disc_map", "text_inventory.csv"))
    ap.add_argument("--append", action="store_true", help="append to CSV instead of overwrite")
    ap.add_argument("--txt", help="readable dump path (default disc_map/<name>_text.txt)")
    ap.add_argument("--min-len", type=int, default=3)
    ap.add_argument("--terminated-only", action="store_true",
                    help="keep only '>' / '/' terminated messages (cuts table-fragment noise)")
    ap.add_argument("--self-test", action="store_true",
                    help="print whether known menu strings are found (ground-truth gate)")
    args = ap.parse_args()

    toc = json.load(open(args.toc))
    name = toc["name"]
    bin_path = args.bin or json.load(open(FILEMAP))["source_bin"]
    disc_first = toc["disc_sector_first"]
    ents = [e for e in toc["entries"] if e["sector_count"] > 0]

    rows = []
    txt_path = args.txt or os.path.join(HERE, "disc_map", f"{name}_text.txt")
    with open(txt_path, "w") as tf:
        tf.write(f"# {name} — extracted draw_string text (scan_text.py)\n\n")
        for e in ents:
            payload = read_entry(bin_path, disc_first, e)
            if payload[:4] == b"\x10\x00\x00\x00":
                continue  # TIM
            msgs = split_messages(payload)
            wrote_hdr = False
            for (boff, raw, term) in msgs:
                c = clean(raw)
                printable = sum(1 for b in c if 0x20 <= b < 0x7f)
                if printable < args.min_len:
                    continue
                if len(set(b for b in c if 0x20 <= b < 0x7f)) < 3:
                    continue  # filler run
                if args.terminated_only and term not in (">", "/"):
                    continue
                enc = classify_encoding(c)
                ren, note = renderable(c)
                disp = display(c)
                rows.append({
                    "container": name,
                    "entry": e["index"],
                    "byte_off": boff,
                    "char_len": printable,
                    "encoding": enc,
                    "terminator": term,
                    "renderable": "yes" if ren else "no",
                    "note": note,
                    "sample": disp[:80],
                })
                if not wrote_hdr:
                    tf.write(f"\n=== entry {e['index']} (sectors {e['sector_start']}+"
                             f"{e['sector_count']}, disc LBA {disc_first + e['sector_start']}) ===\n")
                    wrote_hdr = True
                tf.write(f"  [+0x{boff:04x} term={term!r} {enc}] {disp}\n")

    # write CSV
    mode = "a" if args.append and os.path.exists(args.csv) else "w"
    write_header = not (mode == "a")
    os.makedirs(os.path.dirname(args.csv), exist_ok=True)
    with open(args.csv, mode, newline="") as cf:
        w = csv.DictWriter(cf, fieldnames=["container", "entry", "byte_off", "char_len",
                                           "encoding", "terminator", "renderable", "note", "sample"])
        if write_header:
            w.writeheader()
        for r in rows:
            w.writerow(r)

    n_ren = sum(1 for r in rows if r["renderable"] == "yes")
    n_sjis = sum(1 for r in rows if "sjis" in r["encoding"])
    total_chars = sum(r["char_len"] for r in rows)
    print(f"[{name}] messages: {len(rows)}  renderable: {n_ren}  sjis: {n_sjis}  "
          f"chars: {total_chars:,}")
    print(f"  readable dump -> {txt_path}")
    print(f"  csv ({'appended' if mode=='a' else 'wrote'}) -> {args.csv}")

    if args.self_test:
        joined = " ".join(r["sample"] for r in rows)
        needles = ["NEW GAME", "CONTINUE", "GARAGE", "MISSION", "RANKING", "MAIL", "SHOP", "SYSTEM"]
        print("  SELF-TEST known-string presence:")
        for nd in needles:
            print(f"    {'FOUND ' if nd in joined else 'absent'} {nd!r}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
