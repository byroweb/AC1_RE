#!/usr/bin/env python3
"""
build_manifest.py — join the pipeline's outputs into the committed manifest and
the local review page.

Stage 7 of the voice pipeline (docs/AUDIO_PIPELINE.md). Reads clips.json,
transcripts.json, kinds.json, speakers.json and matches.json from <out> and writes:

  disc_map/voice_manifest.tsv   one row per clip (transcripts + metadata only —
                                no audio, safe to commit)
  <out>/review.html             local listening/verification page generated from
                                tools/audio/review.html; plays <out>/clips/*.wav,
                                lets you edit text / voice / kind / notes and
                                exports the edited TSV. Never publish it (it sits
                                next to game audio).

Owner edits survive re-runs: the columns verified_text, verified, voice_label and
notes are carried over from an existing manifest by clip_id, and so is a `kind`
the owner changed: when a row's kind differs from its kind_auto, that kind is
kept as an override (classify.py reads the same thing back, see load_overrides).
voice_label is carried per clip, so a cluster keeps its name even when the
voice ids are renumbered by a re-cluster (the new voice of each labelled clip
inherits the label by majority). Drop the exported TSV from the review page
over disc_map/voice_manifest.tsv to bring edits back in.

Text / kind columns:
  transcript      Whisper's transcript in the clip's own language (Japanese text
                  for a speech_ja clip); blank for ambience / sfx (the gate's
                  hidden guess is kept in auto_notes). There is no translation.
  language, language_prob   Whisper's detected language and its probability
  kind            speech_en / speech_ja / ambience / sfx / unknown — THE OWNER'S
                  COLUMN: edit it (TSV or review-page dropdown) and the value sticks
  kind_auto       what classify.py said; kind != kind_auto means "owner overrode"
confidence = exp(avg_logprob) * (1 - no_speech_prob)   (0..1, higher is better)
status     = empty   nothing transcribed / not speech (ambience, sfx)
             matched corpus string at similarity >= --match-threshold (85)
             check   needs a listen
             verified the owner ticked it
"""
import argparse
import csv
import json
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
from classify import KINDS, NONSPEECH, SPEECH  # noqa: E402

MANIFEST = os.path.join(A.ROOT, "disc_map", "voice_manifest.tsv")
TEMPLATE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "review.html")

COLUMNS = [
    "clip_id", "source_file", "stream", "channel", "clip", "sector_first", "sector_last",
    "audio_sector_first", "audio_sector_last", "stride", "start_sample", "end_sample", "rate",
    "duration_s", "voice", "voice_label", "f0_hz", "processed", "transcript",
    "language", "language_prob", "kind", "kind_auto",
    "confidence", "avg_logprob", "no_speech_prob", "matched_string_id", "matched_text",
    "similarity", "status", "auto_notes", "verified_text", "verified", "notes",
]
CARRY = ["verified_text", "verified", "kind", "voice_label", "notes"]
TRUE = ("1", "yes", "true", "x")
# Stock Whisper hallucinations on music / silence (seen on the music-only cutscene clips).
HALLUCINATIONS = ("thank you for watching", "thanks for watching", "subtitles by", "bye-bye",
                  "please subscribe", "you", "thank you.")


def load_existing(path):
    """(rows by clip_id, {voice: label}) from an existing manifest (empty if none)."""
    if not os.path.exists(path):
        return {}, {}
    rows = {}
    with open(path, encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            rows[r["clip_id"]] = r
    return rows, voice_labels_by_voice(rows)


def voice_labels_by_voice(rows):
    labels = {}
    for r in rows.values():
        if r.get("voice") and r.get("voice_label"):
            labels.setdefault(r["voice"], {}).setdefault(r["voice_label"], 0)
            labels[r["voice"]][r["voice_label"]] += 1
    return {v: max(d, key=d.get) for v, d in labels.items()}


def carried_voice_labels(old_rows, new_voice_of_clip):
    """{new voice id: label} — every labelled clip of the old manifest votes for its
    label under the voice it has NOW, so renumbered clusters keep their names."""
    votes = {}
    for cid, r in old_rows.items():
        lab = (r.get("voice_label") or "").strip()
        v = new_voice_of_clip.get(cid)
        if lab and v:
            votes.setdefault(v, {}).setdefault(lab, 0)
            votes[v][lab] += 1
    return {v: max(d, key=d.get) for v, d in votes.items()}


def owner_kind(old_row):
    """The kind the owner set in an existing manifest row, or "" if the row still
    shows the gate's own verdict (kind == kind_auto) or has no kind."""
    kind = (old_row.get("kind") or "").strip()
    auto = (old_row.get("kind_auto") or "").strip()
    if kind in KINDS and kind != auto:
        return kind
    return ""


def effective_kind(auto_kind, override):
    override = (override or "").strip()
    if override in KINDS:
        return override
    return auto_kind if auto_kind in KINDS else "unknown"


def transcript_text(t, kind):
    """The transcript column: Whisper's text in the clip's language, blank for non-speech."""
    return "" if kind in NONSPEECH else (t.get("text") or "").strip()


def build_rows(clips, trans, spk, matches, kinds, old_rows, match_threshold=85.0,
               voice_labels=None):
    """Manifest rows (dicts keyed by COLUMNS) for every clip."""
    voice_of = {cid: s.get("voice", "") for cid, s in spk.get("clips", {}).items()}
    labels = dict(voice_labels or {})
    for v, lab in carried_voice_labels(old_rows, voice_of).items():
        labels[v] = lab
    for v, lab in voice_labels_by_voice(old_rows).items():
        labels.setdefault(v, lab)

    rows = []
    for c in clips:
        cid = c["clip_id"]
        t = trans.get(cid) or {"text": "", "avg_logprob": -10.0, "no_speech_prob": 1.0,
                               "language": "", "language_prob": 0.0, "_missing": True}
        s = spk.get("clips", {}).get(cid, {})
        m = matches.get(cid, {})
        k = kinds.get(cid, {})
        old = old_rows.get(cid, {})
        kind_auto = k.get("kind_auto") or k.get("kind") or ""
        override = owner_kind(old)
        kind = effective_kind(kind_auto, override)
        text = transcript_text(t, kind)
        conf = math.exp(t["avg_logprob"]) * (1.0 - t["no_speech_prob"])
        sim = m.get("similarity", 0) or 0
        if kind in NONSPEECH or not text or t["no_speech_prob"] > 0.6:
            status = "empty"
        elif sim >= match_threshold:
            status = "matched"
        else:
            status = "check"
        notes = []
        if kind in NONSPEECH:
            notes.append(f"{kind}: {k.get('reasons', 'no speech')}")
            heard = (t.get("text") or "").strip()
            if heard:
                notes.append(f"gate hid Whisper's guess: {heard[:60]!r}")
        elif kind == "unknown":
            notes.append(f"unclear clip: {k.get('reasons', 'conflicting signals')}")
        elif kind == "speech_ja":
            notes.append("Japanese line (transcript is the Japanese; reclaimable, see voice_reclaimable.tsv)")
        elif kind == "speech_en" and t.get("language") not in ("en", "", None):
            notes.append(f"language {t.get('language')} but kind speech_en")
        if k.get("uncertain") and kind not in NONSPEECH:
            notes.append(f"classification uncertain: {k.get('reasons', '')}")
        if override and override != kind_auto:
            notes.append(f"kind set by owner (gate said {kind_auto or '?'})")
        if s.get("processed"):
            notes.append("processed/robotic voice?")
        if s.get("duplicate_of"):
            notes.append(f"same recording as {s['duplicate_of']}")
        if kind not in NONSPEECH and 0.3 < t["no_speech_prob"] <= 0.6:
            notes.append("maybe not speech")
        if c["kind"] == "STR":
            notes.append("cutscene track (music under speech)")
        if conf < 0.4 and text:
            notes.append("low confidence")
        if text.lower().strip(" .!") in [h.strip(" .") for h in HALLUCINATIONS]:
            notes.append("stock Whisper hallucination phrase — probably no speech")
        voice = s.get("voice", "")
        row = {
            "clip_id": cid, "source_file": c["source_file"], "stream": c["stream"],
            "channel": c["channel"], "clip": c["clip_index"],
            "sector_first": c["sector_first"], "sector_last": c["sector_last"],
            "audio_sector_first": c["audio_sector_first"], "audio_sector_last": c["audio_sector_last"],
            "stride": c["stride"], "start_sample": c["start_sample"], "end_sample": c["end_sample"],
            "rate": c["rate"], "duration_s": c["duration_s"],
            "voice": voice, "voice_label": labels.get(voice, ""),
            "f0_hz": s.get("f0_hz") if s.get("f0_hz") is not None else "",
            "processed": int(bool(s.get("processed"))) if s else "",
            "transcript": text, "language": t.get("language") or "",
            "language_prob": t.get("language_prob", ""),
            "kind": kind, "kind_auto": kind_auto if kind_auto in KINDS else "unknown",
            "confidence": round(conf, 3),
            "avg_logprob": t["avg_logprob"], "no_speech_prob": t["no_speech_prob"],
            "matched_string_id": m.get("best_id", ""), "matched_text": m.get("best_text", ""),
            "similarity": sim, "status": status,
            "verified_text": old.get("verified_text", ""), "verified": old.get("verified", ""),
            "auto_notes": "; ".join(notes), "notes": old.get("notes", ""),
        }
        if old.get("verified") in TRUE:
            row["status"] = "verified"
        rows.append(row)
    return rows


def write_manifest(path, rows):
    A.ensure_dir(os.path.dirname(path))
    with open(path, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n",
                           quoting=csv.QUOTE_MINIMAL)
        w.writeheader()
        for r in rows:
            w.writerow({k: ("" if r[k] is None else r[k]) for k in COLUMNS})


def counts(rows, key):
    by = {}
    for r in rows:
        by[r[key]] = by.get(r[key], 0) + 1
    return dict(sorted(by.items(), key=lambda kv: (-kv[1], kv[0])))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--manifest", default=MANIFEST)
    ap.add_argument("--match-threshold", type=float, default=85.0)
    ap.add_argument("--no-review", action="store_true")
    args = ap.parse_args()

    out = args.out
    clips = A.load_json(os.path.join(out, "clips.json"))["clips"]
    trans = A.load_json(os.path.join(out, "transcripts.json"))
    kinds = A.load_json(os.path.join(out, "kinds.json"), default={})
    spk = A.load_json(os.path.join(out, "speakers.json"), default={"clips": {}, "voices": []})
    matches = A.load_json(os.path.join(out, "matches.json"), default={})
    old_rows, _old_labels = load_existing(args.manifest)
    voice_labels = {v["voice"]: v.get("label", "") for v in spk.get("voices", []) if v.get("label")}
    if not kinds:
        A.log("[manifest] no kinds.json — run classify.py for the speech / ambience / sfx gate")

    rows = build_rows(clips, trans, spk, matches, kinds, old_rows, args.match_threshold, voice_labels)
    missing = sum(1 for c in clips if c["clip_id"] not in trans)
    write_manifest(args.manifest, rows)
    A.log(f"[manifest] {len(rows)} rows -> {args.manifest}")
    A.log(f"[manifest] status {counts(rows, 'status')}")
    A.log(f"[manifest] kind   {counts(rows, 'kind')}")
    A.log(f"[manifest] lang   {counts(rows, 'language')}"
          + (f"  ({missing} clips had no transcript)" if missing else ""))
    unsure = [r["clip_id"] for r in rows if ("uncertain" in r["auto_notes"] or r["kind"] == "unknown")
              and r["kind"] == r["kind_auto"]]
    if unsure:
        A.log(f"[manifest] {len(unsure)} clips with an uncertain classification: {', '.join(unsure)}")

    if not args.no_review:
        with open(TEMPLATE, encoding="utf-8") as f:
            html = f.read()
        wavs = {c["clip_id"]: c["wav"] for c in clips}
        for r in rows:
            r["wav"] = wavs[r["clip_id"]]
            r["candidates"] = matches.get(r["clip_id"], {}).get("candidates", [])
        payload = {"rows": rows, "voices": spk.get("voices", []), "columns": COLUMNS,
                   "kinds": KINDS, "speech_kinds": sorted(SPEECH),
                   "generated_from": os.path.relpath(args.manifest, A.ROOT).replace("\\", "/")}
        html = html.replace("__ROWS_JSON__", json.dumps(payload, ensure_ascii=False).replace("</", "<\\/"))
        rp = os.path.join(out, "review.html")
        with open(rp, "w", encoding="utf-8") as f:
            f.write(html)
        A.log(f"[manifest] review page -> {rp}  (local only; open in a browser)")


if __name__ == "__main__":
    main()
