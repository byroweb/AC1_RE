#!/usr/bin/env python3
"""
match_text.py — line up each transcript with the extracted mission-text corpus.

Stage 5 of the voice pipeline (docs/AUDIO_PIPELINE.md). Builds a candidate list
from the .T containers already extracted by tools/extract/extract_t.py
--dump-entries (default: disc_map/extracted/MIS_T — the whole mission text
corpus of GG/MS/MIS.T, split into draw_string messages exactly like
tools/extract/scan_text.py does, and additionally into sentences), then scores
every transcript against it with rapidfuzz.

String ids follow disc_map/text_inventory.csv: <container>:<entry>:+0x<byte_off>
(a "#sN" suffix marks the N-th sentence of that message). A good match means the
owner only has to accept the catalogued string; a poor one flags the clip for
real listening. Note: AC1's in-mission radio lines mostly have no on-screen
text, so low scores on TALK clips are expected — the match is there to catch
the lines that ARE in the corpus (mission names, briefing phrases, cutscene text).

Writes <out>/matches.json: clip_id -> {best_id, best_text, similarity, candidates}.

Usage:
  tools/audio/.venv/Scripts/python tools/audio/match_text.py
      [--corpus disc_map/extracted/FDAT_T/toc.json ...]  [--all-extracted]
"""
import argparse
import glob
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "extract"))
import ac1audio as A  # noqa: E402
import scan_text as ST  # noqa: E402  (tools/extract/scan_text.py: split_messages / clean)

DEFAULT_CORPUS = os.path.join(A.ROOT, "disc_map", "extracted", "MIS_T", "toc.json")
MARKUP = re.compile(rb"[@%]\d")


def message_text(raw):
    """Bytes of one draw_string message -> plain readable text."""
    c = ST.clean(raw)
    c = MARKUP.sub(b"", c)
    c = c.replace(b"\r", b" ").replace(b"\n", b" ").replace(b"\t", b" ")
    s = c.decode("latin-1", errors="replace")
    s = "".join(ch if 0x20 <= ord(ch) < 0x7f else " " for ch in s)
    return re.sub(r"\s+", " ", s).strip()


def normalize(s):
    s = s.lower().replace("’", "'")
    s = re.sub(r"[^a-z0-9' ]+", " ", s)
    return re.sub(r"\s+", " ", s).strip()


SENT_SPLIT = re.compile(r"(?<=[.!?])\s+(?=[A-Z\"'(])")


def load_corpus(toc_paths, bin_path=None):
    """[(string_id, text)] from every non-TIM entry of each extracted container."""
    cands = []
    for toc_path in toc_paths:
        toc = json.load(open(toc_path))
        name = toc["name"]
        ent_dir = os.path.join(os.path.dirname(toc_path), "entries")
        for e in toc["entries"]:
            if e["sector_count"] <= 0:
                continue
            p = os.path.join(ent_dir, f"{e['index']:03d}.bin")
            if os.path.exists(p):
                payload = open(p, "rb").read()
            elif bin_path:
                payload = ST.read_entry(bin_path, toc["disc_sector_first"], e)
            else:
                continue
            if payload[:4] == b"\x10\x00\x00\x00":
                continue  # TIM image
            for boff, raw, term in ST.split_messages(payload):
                text = message_text(raw)
                if len(text) < 4 or len(set(text.lower())) < 4:
                    continue
                sid = f"{name}:{e['index']}:+0x{boff:04x}"
                cands.append((sid, text))
                sents = [t.strip() for t in SENT_SPLIT.split(text) if len(t.split()) >= 3]
                if len(sents) > 1:
                    cands += [(f"{sid}#s{i}", t) for i, t in enumerate(sents)]
    return cands


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--corpus", action="append", default=[],
                    help="extra toc.json from extract_t.py (repeatable)")
    ap.add_argument("--all-extracted", action="store_true",
                    help="use every disc_map/extracted/*/toc.json that has dumped entries")
    ap.add_argument("--bin", help="disc image, only needed when a corpus has no entries/ dump")
    ap.add_argument("--top", type=int, default=3)
    args = ap.parse_args()

    tocs = [DEFAULT_CORPUS] if os.path.exists(DEFAULT_CORPUS) else []
    tocs += args.corpus
    if args.all_extracted:
        tocs += [p for p in glob.glob(os.path.join(A.ROOT, "disc_map", "extracted", "*", "toc.json"))
                 if os.path.isdir(os.path.join(os.path.dirname(p), "entries"))]
    tocs = list(dict.fromkeys(os.path.abspath(t) for t in tocs if os.path.exists(t)))
    if not tocs:
        sys.exit("no corpus: run `python3 tools/extract/extract_t.py --file GG/MS/MIS.T --dump-entries` first")
    bin_path = args.bin or (A.disc_bin() if any(
        not os.path.isdir(os.path.join(os.path.dirname(t), "entries")) for t in tocs) else None)
    cands = load_corpus(tocs, bin_path)
    A.log(f"[match] corpus: {len(cands)} candidate strings from {len(tocs)} container(s)")

    from rapidfuzz import fuzz, process
    choices = {sid: normalize(t) for sid, t in cands}
    texts = dict(cands)
    tpath = os.path.join(args.out, "transcripts.json")
    trans = A.load_json(tpath)
    matches = {}
    for cid, t in trans.items():
        q = normalize(t.get("text", ""))
        if len(q) < 3:
            matches[cid] = {"best_id": "", "best_text": "", "similarity": 0, "candidates": []}
            continue
        hits = process.extract(q, choices, scorer=fuzz.token_sort_ratio, limit=args.top)
        cl = [{"id": sid, "text": texts[sid], "similarity": round(score, 1)} for _, score, sid in hits]
        best = cl[0] if cl else {"id": "", "text": "", "similarity": 0}
        matches[cid] = {"best_id": best["id"], "best_text": best["text"],
                        "similarity": best["similarity"], "candidates": cl}
    A.save_json(os.path.join(args.out, "matches.json"), matches)
    sims = sorted((m["similarity"] for m in matches.values()), reverse=True)
    strong = sum(1 for s in sims if s >= 85)
    A.log(f"[match] {len(matches)} transcripts scored; {strong} at >=85, "
          f"median {sims[len(sims)//2] if sims else 0}")
    A.log(f"[match] -> {os.path.join(args.out, 'matches.json')}")


if __name__ == "__main__":
    main()
