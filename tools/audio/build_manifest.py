#!/usr/bin/env python3
"""
build_manifest.py — join the pipeline's outputs into the committed manifest and
the local review page.

Stage 6 of the voice pipeline (docs/AUDIO_PIPELINE.md). Reads clips.json,
transcripts.json, speakers.json and matches.json from <out> and writes:

  disc_map/voice_manifest.tsv   one row per clip (transcripts + metadata only —
                                no audio, safe to commit)
  <out>/review.html             local listening/verification page generated from
                                tools/audio/review.html; plays <out>/clips/*.wav,
                                lets you edit text / voice / notes and exports the
                                edited TSV. Never publish it (it sits next to game audio).

Owner edits survive re-runs: the columns verified_text, verified, voice_label
and notes are carried over from an existing manifest by clip_id (and
voice_label by voice id), so re-transcribing never wipes verification work.
Drop the exported TSV from the review page over disc_map/voice_manifest.tsv to
bring edits back in.

confidence = exp(avg_logprob) * (1 - no_speech_prob)   (0..1, higher is better)
status     = empty   nothing transcribed / Whisper thinks it is not speech
             matched corpus string at similarity >= --match-threshold (85)
             check   needs a listen
"""
import argparse
import csv
import json
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402

MANIFEST = os.path.join(A.ROOT, "disc_map", "voice_manifest.tsv")
TEMPLATE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "review.html")

COLUMNS = [
    "clip_id", "source_file", "stream", "channel", "clip", "sector_first", "sector_last",
    "audio_sector_first", "audio_sector_last", "stride", "start_sample", "end_sample", "rate",
    "duration_s", "voice", "voice_label", "f0_hz", "processed", "transcript", "confidence",
    "avg_logprob", "no_speech_prob", "matched_string_id", "matched_text", "similarity",
    "status", "auto_notes", "verified_text", "verified", "notes",
]
CARRY = ["verified_text", "verified", "voice_label", "notes"]
# Stock Whisper hallucinations on music / silence (seen on the music-only cutscene clips).
HALLUCINATIONS = ("thank you for watching", "thanks for watching", "subtitles by", "bye-bye",
                  "please subscribe", "you", "thank you.")


def load_existing(path):
    if not os.path.exists(path):
        return {}, {}
    rows = {}
    with open(path, encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            rows[r["clip_id"]] = r
    labels = {}
    for r in rows.values():
        if r.get("voice") and r.get("voice_label"):
            labels.setdefault(r["voice"], {}).setdefault(r["voice_label"], 0)
            labels[r["voice"]][r["voice_label"]] += 1
    labels = {v: max(d, key=d.get) for v, d in labels.items()}
    return rows, labels


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
    spk = A.load_json(os.path.join(out, "speakers.json"), default={"clips": {}, "voices": []})
    matches = A.load_json(os.path.join(out, "matches.json"), default={})
    old_rows, old_labels = load_existing(args.manifest)
    voice_labels = {v["voice"]: v.get("label", "") for v in spk.get("voices", [])}
    for v, lab in old_labels.items():
        voice_labels[v] = voice_labels.get(v) or lab

    rows = []
    missing = 0
    for c in clips:
        t = trans.get(c["clip_id"])
        if t is None:
            missing += 1
            t = {"text": "", "avg_logprob": -10.0, "no_speech_prob": 1.0}
        s = spk["clips"].get(c["clip_id"], {})
        m = matches.get(c["clip_id"], {})
        conf = math.exp(t["avg_logprob"]) * (1.0 - t["no_speech_prob"])
        text = t["text"].strip()
        sim = m.get("similarity", 0) or 0
        if not text or t["no_speech_prob"] > 0.6:
            status = "empty"
        elif sim >= args.match_threshold:
            status = "matched"
        else:
            status = "check"
        notes = []
        if s.get("processed"):
            notes.append("processed/robotic voice?")
        if s.get("duplicate_of"):
            notes.append(f"same recording as {s['duplicate_of']}")
        if 0.3 < t["no_speech_prob"] <= 0.6:
            notes.append("maybe not speech")
        if c["kind"] == "STR":
            notes.append("cutscene track (music under speech)")
        if conf < 0.4 and text:
            notes.append("low confidence")
        if text.lower().strip(" .!") in [h.strip(" .") for h in HALLUCINATIONS]:
            notes.append("stock Whisper hallucination phrase — probably no speech")
        old = old_rows.get(c["clip_id"], {})
        row = {
            "clip_id": c["clip_id"], "source_file": c["source_file"], "stream": c["stream"],
            "channel": c["channel"], "clip": c["clip_index"],
            "sector_first": c["sector_first"], "sector_last": c["sector_last"],
            "audio_sector_first": c["audio_sector_first"], "audio_sector_last": c["audio_sector_last"],
            "stride": c["stride"], "start_sample": c["start_sample"], "end_sample": c["end_sample"],
            "rate": c["rate"], "duration_s": c["duration_s"],
            "voice": s.get("voice", ""), "voice_label": voice_labels.get(s.get("voice", ""), ""),
            "f0_hz": s.get("f0_hz") if s.get("f0_hz") is not None else "",
            "processed": int(bool(s.get("processed"))) if s else "",
            "transcript": text, "confidence": round(conf, 3),
            "avg_logprob": t["avg_logprob"], "no_speech_prob": t["no_speech_prob"],
            "matched_string_id": m.get("best_id", ""), "matched_text": m.get("best_text", ""),
            "similarity": sim, "status": status,
            "verified_text": old.get("verified_text", ""), "verified": old.get("verified", ""),
            "auto_notes": "; ".join(notes), "notes": old.get("notes", ""),
        }
        if old.get("verified") in ("1", "yes", "true", "x"):
            row["status"] = "verified"
        rows.append(row)

    A.ensure_dir(os.path.dirname(args.manifest))
    with open(args.manifest, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n",
                           quoting=csv.QUOTE_MINIMAL)
        w.writeheader()
        for r in rows:
            w.writerow({k: ("" if r[k] is None else r[k]) for k in COLUMNS})
    by = {}
    for r in rows:
        by[r["status"]] = by.get(r["status"], 0) + 1
    A.log(f"[manifest] {len(rows)} rows -> {args.manifest}  status: {by}"
          + (f"  ({missing} clips had no transcript)" if missing else ""))

    if not args.no_review:
        with open(TEMPLATE, encoding="utf-8") as f:
            html = f.read()
        wavs = {c["clip_id"]: c["wav"] for c in clips}
        for r in rows:
            r["wav"] = wavs[r["clip_id"]]
            r["candidates"] = matches.get(r["clip_id"], {}).get("candidates", [])
        payload = {"rows": rows, "voices": spk.get("voices", []), "columns": COLUMNS,
                   "generated_from": os.path.relpath(args.manifest, A.ROOT).replace("\\", "/")}
        html = html.replace("__ROWS_JSON__", json.dumps(payload).replace("</", "<\\/"))
        rp = os.path.join(out, "review.html")
        with open(rp, "w", encoding="utf-8") as f:
            f.write(html)
        A.log(f"[manifest] review page -> {rp}  (local only; open in a browser)")


if __name__ == "__main__":
    main()
