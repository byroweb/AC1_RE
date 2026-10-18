#!/usr/bin/env python3
"""
match_text.py — line up each transcript with the game's own text.

Stage 6 of the voice pipeline (docs/AUDIO_PIPELINE.md). Two corpora:

  * the SOUND-SEQUENCE SUBTITLES (default on when disc_map/extracted/FDAT_T/
    entries/200.bin exists, or an <out>/xa_line_refs.json from xa_refs.py):
    every radio line's on-screen English subtitle, one candidate per sequence,
    id `SEQ:<snd>:X<n>` (sequence <snd> plays TALK line n = 16*file + channel)
    or `SEQ:<snd>` for a sequence without an X opcode (music cues, text-only
    radio). The leading speaker tag ("Radio:", "COM :") is split off into a
    `speaker` field. These are the lines MIS.T never had.
  * the mission-text corpus of .T containers already extracted by
    tools/extract/extract_t.py --dump-entries (default: disc_map/extracted/MIS_T
    — GG/MS/MIS.T split into draw_string messages exactly like
    tools/extract/scan_text.py does, and additionally into sentences). Ids follow
    disc_map/text_inventory.csv: <container>:<entry>:+0x<byte_off> (a "#sN"
    suffix marks the N-th sentence of that message). Add more with --corpus /
    --all-extracted.

Every transcript is scored against the union with rapidfuzz (free match ->
best_*). A TALK clip's own line number is known from its file and channel
(xa_refs.line_number), so the clip is ALSO scored against the subtitle of that
very line (own_line_*): that is the direct hit — the script line this slot is
supposed to say — and build_manifest.py prefers it for the status. A low
own-line score with a good transcript means the slot's audio does not say what
the subtitle says (TALK01 ch9: Japanese take under "A...a friend! We're saved!").

Clips the gate (classify.py / kinds.json) marked ambience or sfx, and clips in a
language other than English (the corpora are English), get an empty match; the
own-line subtitle is still recorded for them (similarity 0).

Writes <out>/matches.json: clip_id -> {best_id, best_text, similarity, candidates,
own_line, own_line_id, own_line_text, own_line_speaker, own_line_similarity}.

Usage:
  tools/audio/.venv/Scripts/python tools/audio/match_text.py
      [--corpus disc_map/extracted/FDAT_T/toc.json ...]  [--all-extracted]
      [--fdat-entries DIR]  [--no-sequences]
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
import xa_refs as X  # noqa: E402
from classify import NONSPEECH  # noqa: E402

DEFAULT_CORPUS = os.path.join(A.ROOT, "disc_map", "extracted", "MIS_T", "toc.json")
MARKUP = re.compile(rb"[@%]\d")
SPEAKER_TAG = re.compile(r"^\s*(Radio|COM)\s*:\s*", re.I)
SENT_SPLIT = re.compile(r"(?<=[.!?])\s+(?=[A-Z\"'(])")
UNKNOWN_GLYPH = re.compile(r"<[0-9a-f]{2}>")
STRONG = 85.0   # "matches its own line" in the summary (build_manifest's default threshold)
WEAK = 60.0     # below this the audio probably does not say the subtitle


def message_text(raw):
    """Bytes of one draw_string message -> plain readable text."""
    c = ST.clean(raw)
    c = MARKUP.sub(b"", c)
    c = c.replace(b"\r", b" ").replace(b"\n", b" ").replace(b"\t", b" ")
    s = c.decode("latin-1", errors="replace")
    s = "".join(ch if 0x20 <= ord(ch) < 0x7f else " " for ch in s)
    return re.sub(r"\s+", " ", s).strip()


def english_text(t):
    """The transcript if the clip is English, else "" (the corpus is English and
    there is no translation step)."""
    return t.get("text", "") if (t.get("language") or "en") == "en" else ""


ONES = "zero one two three four five six seven eight nine ten eleven twelve thirteen fourteen " \
       "fifteen sixteen seventeen eighteen nineteen".split()
TENS = "_ _ twenty thirty forty fifty sixty seventy eighty ninety".split()


def number_words(m):
    """'30' -> 'thirty', '17' -> 'seventeen' (0..99; larger numbers stay digits)."""
    n = int(m.group(0))
    if n < 20:
        return ONES[n]
    if n < 100:
        return TENS[n // 10] + ("" if n % 10 == 0 else " " + ONES[n % 10])
    return m.group(0)


def normalize(s):
    """Lower-case words only; digits become words (the subtitles' timer digits
    "30 seconds" against Whisper's "thirty seconds"); unknown font codes dropped."""
    s = UNKNOWN_GLYPH.sub(" ", s)
    s = s.lower().replace("’", "'")
    s = re.sub(r"[^a-z0-9' ]+", " ", s)
    s = re.sub(r"\b\d+\b", number_words, s)
    return re.sub(r"\s+", " ", s).strip()


def sentences(sid, text, runs=False):
    """[(sid#sN, sentence)] when the text has more than one 3+ word sentence;
    with runs=True also every run of consecutive sentences (sid#sI-J), for a
    clip that covers part of a multi-sentence line."""
    sents = [t.strip() for t in SENT_SPLIT.split(text) if len(t.split()) >= 3]
    if len(sents) < 2:
        return []
    out = [(f"{sid}#s{i}", t) for i, t in enumerate(sents)]
    if runs:
        for i in range(len(sents)):
            for j in range(i + 1, len(sents)):
                if j - i + 1 < len(sents):
                    out.append((f"{sid}#s{i}-{j}", " ".join(sents[i:j + 1])))
    return out


# ------------------------------------------------------------ .T containers
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
                cands += sentences(sid, text)
    return cands


# -------------------------------------------------------- sound sequences
def split_speaker(text):
    """'Radio: Is that you?' -> ('Radio', 'Is that you?'); 'COM : Combat system
    engaged' -> ('COM', ...); no tag -> ('', text)."""
    text = (text or "").strip()
    m = SPEAKER_TAG.match(text)
    if not m:
        return "", text
    return m.group(1).strip(), text[m.end():].strip()


def sequence_candidates(refs):
    """One candidate per sequence that carries a subtitle, from an xa_refs.scan()
    result (or a loaded xa_line_refs.json):
    [{id, text, speaker, line, snd}] in sequence order. `line` is the TALK line
    the sequence's X opcode plays (None for SEQ:<snd> ids)."""
    out = []
    seqs = refs.get("sequences", {})
    for snd in sorted(seqs, key=int):
        rec = seqs[snd]
        speaker, text = split_speaker(rec.get("text", ""))
        if len(text) < 2:
            continue
        xa = rec.get("xa_lines") or []
        line = xa[0] if xa else None
        sid = f"SEQ:{snd}:X{line}" if line is not None else f"SEQ:{snd}"
        out.append({"id": sid, "text": text, "speaker": speaker, "line": line, "snd": int(snd)})
    return out


def load_sequence_corpus(fdat_entries=None, refs_path=None):
    """Sequence candidates from the blob in <fdat_entries>/200.bin when it exists,
    else from an existing xa_line_refs.json; [] when neither is there."""
    fdat_entries = fdat_entries or X.FDAT_ENTRIES
    seq_path = os.path.join(fdat_entries, f"{X.SEQ_ENTRY:03d}.bin")
    if os.path.exists(seq_path):
        return sequence_candidates(X.scan(fdat_entries))
    if refs_path and os.path.exists(refs_path):
        return sequence_candidates(A.load_json(refs_path))
    return []


def own_line_of(clip):
    """TALK line number of a clips.json record (None for STR tracks)."""
    return X.line_number(clip.get("source_file", ""), clip.get("channel", 0))


def score_own_line(q, n, seq_cands, scorer=None):
    """Best hit of the normalised transcript `q` against the subtitle(s) of TALK
    line n — the whole subtitle, each of its sentences and each run of consecutive
    sentences (a channel split into several clips gives one clip per sentence or
    two), over every sequence that plays the line: {id, text, speaker, similarity}; None when no sequence
    plays line n (the Japanese slots). An empty `q` scores 0 but still names
    the line's subtitle, so the manifest can show what the slot should say."""
    if n is None:
        return None
    own = [c for c in seq_cands if c["line"] == n]
    if not own:
        return None
    if scorer is None:
        from rapidfuzz import fuzz
        scorer = fuzz.token_sort_ratio
    best = None
    for c in own:
        for sid, text in [(c["id"], c["text"])] + sentences(c["id"], c["text"], runs=True):
            s = float(scorer(q, normalize(text))) if q else 0.0
            if best is None or s > best["similarity"]:
                best = {"id": sid, "text": text, "speaker": c["speaker"], "similarity": round(s, 1)}
    return best


def empty_match():
    return {"best_id": "", "best_text": "", "similarity": 0, "candidates": [],
            "own_line": None, "own_line_id": "", "own_line_text": "", "own_line_speaker": "",
            "own_line_similarity": 0}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--corpus", action="append", default=[],
                    help="extra toc.json from extract_t.py (repeatable)")
    ap.add_argument("--all-extracted", action="store_true",
                    help="use every disc_map/extracted/*/toc.json that has dumped entries")
    ap.add_argument("--bin", help="disc image, only needed when a corpus has no entries/ dump")
    ap.add_argument("--fdat-entries", default=X.FDAT_ENTRIES,
                    help="dumped FDAT.T entries (200.bin = the sound-sequence blob with the subtitles)")
    ap.add_argument("--no-sequences", action="store_true", help="leave the subtitle corpus out")
    ap.add_argument("--top", type=int, default=3)
    args = ap.parse_args()

    tocs = [DEFAULT_CORPUS] if os.path.exists(DEFAULT_CORPUS) else []
    tocs += args.corpus
    if args.all_extracted:
        tocs += [p for p in glob.glob(os.path.join(A.ROOT, "disc_map", "extracted", "*", "toc.json"))
                 if os.path.isdir(os.path.join(os.path.dirname(p), "entries"))]
    tocs = list(dict.fromkeys(os.path.abspath(t) for t in tocs if os.path.exists(t)))
    seq_cands = [] if args.no_sequences else load_sequence_corpus(
        args.fdat_entries, os.path.join(args.out, "xa_line_refs.json"))
    if not tocs and not seq_cands:
        sys.exit("no corpus: run `python3 tools/extract/extract_t.py --file GG/MS/MIS.T --dump-entries` "
                 "and/or `--file GG/COM/FDAT.T --dump-entries` first")
    bin_path = args.bin or (A.disc_bin() if any(
        not os.path.isdir(os.path.join(os.path.dirname(t), "entries")) for t in tocs) else None)
    cands = load_corpus(tocs, bin_path) if tocs else []
    A.log(f"[match] corpus: {len(cands)} candidate strings from {len(tocs)} container(s)")
    seq_strings = []
    for c in seq_cands:
        seq_strings.append((c["id"], c["text"]))
        seq_strings += sentences(c["id"], c["text"])
    speakers = {c["id"]: c["speaker"] for c in seq_cands}
    A.log(f"[match] subtitles: {len(seq_cands)} sequences with text "
          f"({sum(1 for c in seq_cands if c['line'] is not None)} play a TALK line), "
          f"{len(seq_strings)} candidate strings" if seq_cands else "[match] subtitles: none (no 200.bin / xa_line_refs.json)")
    cands += seq_strings

    from rapidfuzz import fuzz, process
    choices = {sid: normalize(t) for sid, t in cands}
    texts = dict(cands)
    tpath = os.path.join(args.out, "transcripts.json")
    trans = A.load_json(tpath)
    clips = {c["clip_id"]: c for c in A.load_json(os.path.join(args.out, "clips.json"), default={"clips": []})["clips"]}
    kinds = A.load_json(os.path.join(args.out, "kinds.json"), default={})
    matches = {}
    skipped = 0
    for cid, t in trans.items():
        if kinds.get(cid, {}).get("kind") in NONSPEECH:
            skipped += 1
            q = ""
        else:
            q = normalize(english_text(t))
        if len(q) < 3:
            q = ""
        m = empty_match()
        n = own_line_of(clips.get(cid, {}))
        m["own_line"] = n
        own = score_own_line(q, n, seq_cands, fuzz.token_sort_ratio)
        if own:
            m.update(own_line_id=own["id"], own_line_text=own["text"], own_line_speaker=own["speaker"],
                     own_line_similarity=own["similarity"])
        if q:
            hits = process.extract(q, choices, scorer=fuzz.token_sort_ratio, limit=args.top)
            cl = []
            for _, score, sid in hits:
                c = {"id": sid, "text": texts[sid], "similarity": round(score, 1)}
                base = sid.split("#")[0]
                if base in speakers:
                    c["speaker"] = speakers[base]
                cl.append(c)
            if cl:
                m.update(best_id=cl[0]["id"], best_text=cl[0]["text"], similarity=cl[0]["similarity"], candidates=cl)
        matches[cid] = m
    A.save_json(os.path.join(args.out, "matches.json"), matches)
    sims = sorted((m["similarity"] for m in matches.values()), reverse=True)
    strong = sum(1 for s in sims if s >= STRONG)
    A.log(f"[match] {len(matches)} transcripts scored; {strong} at >={STRONG:.0f}, "
          f"median {sims[len(sims)//2] if sims else 0}; {skipped} non-speech clips skipped")
    own = {cid: m for cid, m in matches.items() if m["own_line_id"]}
    if own:
        hi = sum(1 for m in own.values() if m["own_line_similarity"] >= STRONG)
        low = sorted((m["own_line_similarity"], cid) for cid, m in own.items() if m["own_line_similarity"] < WEAK)
        no_sub = sorted(cid for cid, m in matches.items() if m["own_line"] is not None and not m["own_line_id"])
        A.log(f"[match] own line: {len(own)} TALK clips have their line's subtitle, {hi} match it at >={STRONG:.0f}, "
              f"{len(low)} below {WEAK:.0f}; {len(no_sub)} TALK clips sit in a line no sequence plays")
        for s, cid in low:
            m = matches[cid]
            A.log(f"  {cid:<20} {s:5.1f}  subtitle {m['own_line_text'][:60]!r}")
    A.log(f"[match] -> {os.path.join(args.out, 'matches.json')}")


if __name__ == "__main__":
    main()
