#!/usr/bin/env python3
"""
reclaimable.py — disc space held by lines the localisation does not need.

Stage 9 of the voice pipeline (docs/AUDIO_PIPELINE.md §6). Reads the committed
manifest (disc_map/voice_manifest.tsv — so the owner's `kind` edits count) and
lists every clip whose kind is speech_ja, ambience or sfx with where it sits on
the disc and how much XA payload it holds, plus per-file and grand totals, into
disc_map/voice_reclaimable.tsv. With <out>/xa_line_refs.json (xa_refs.py) each
TALK clip also gets a static reference status.

What "reclaimable" means here — READ THIS BEFORE PLANNING AROUND THE NUMBERS:
  * The TALK files interleave 16 mono channels with sector stride 16, and the
    game addresses a line as (file, channel) and streams that channel from the
    file's first sector. A reclaimed clip is therefore a MONO 37.8 kHz XA SLOT
    of `audio_sectors` sectors in that channel (4032 samples = 0.1067 s per
    sector), re-fillable with tools/audio/reinject_xa.py — it is NOT contiguous
    free space on the disc, and the 15 other channels' sectors between its
    sectors stay in use.
  * xa_payload_bytes = audio_sectors * 2304 (the ADPCM sound data per Form 2
    sector; the sector's 2324 user bytes include 20 zero bytes); raw_bytes =
    audio_sectors * 2352 is what the sectors occupy on the image.
  * A whole channel is only free when every clip in it is reclaimable; the
    `channel_sectors` / `channel_free` columns (from <out>/channels.json when
    present) say how long the channel is and whether all of it can go. The
    silence between clips and the zero padding at the end of a channel are
    then free too, which is why channel_sectors > the clip sectors.
  * STR clips are speech mixed into the ending movies' stereo music track:
    nothing there can be reclaimed without remixing the track, so they are
    listed for completeness with reclaimable = no.
  * `referenced` is a static scan (see xa_refs.py): "referenced" = a mission
    script plays a sound sequence that plays this line; "unreferenced" = nothing
    found. Not proof — code can call the XA player directly.

Usage:
  tools/audio/.venv/Scripts/python tools/audio/reclaimable.py [--manifest TSV] [--out DIR]
      [--report disc_map/voice_reclaimable.tsv]
"""
import argparse
import csv
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
from classify import KINDS  # noqa: E402
from xa_refs import line_number, status_of  # noqa: E402

MANIFEST = os.path.join(A.ROOT, "disc_map", "voice_manifest.tsv")
REPORT = os.path.join(A.ROOT, "disc_map", "voice_reclaimable.tsv")
RECLAIM_KINDS = ("speech_ja", "ambience", "sfx")
XA_PAYLOAD = 2304      # ADPCM bytes per Form 2 audio sector (18 groups x 128)
SECTOR_RAW = 2352
COLUMNS = [
    "row", "clip_id", "kind", "source_file", "channel", "clip", "line", "xa_id",
    "sector_first", "sector_last", "audio_sector_first", "audio_sector_last", "audio_sectors",
    "stride", "duration_s", "xa_payload_bytes", "raw_bytes", "reclaimable",
    "channel_sectors", "channel_free", "referenced", "ref_detail", "language", "language_prob",
]


def read_manifest(path):
    with open(path, encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def channel_lengths(channels_json):
    """{(source_file, channel): n_audio_sectors} from channels.json, or {}."""
    if not channels_json or not os.path.exists(channels_json):
        return {}
    with open(channels_json, encoding="utf-8") as f:
        d = json.load(f)
    out = {}
    for c in d.get("channels", []):
        n = c.get("n_sectors")
        if n is None and c.get("sector_first") is not None:
            n = (c["sector_last"] - c["sector_first"]) // c.get("stride", 1) + 1
        if n is not None:
            out[(c.get("source_file"), c.get("channel"))] = n
    return out


def build_report(rows, refs=None, chan_len=None):
    """Report rows (clip rows + subtotal/total rows) from manifest rows."""
    refs = refs or {}
    chan_len = chan_len or {}
    sel = [r for r in rows if r.get("kind") in RECLAIM_KINDS]
    # a channel is free only when all its manifest clips are reclaimable
    per_chan = {}
    for r in rows:
        k = (r["source_file"], r["channel"])
        per_chan.setdefault(k, [0, 0])
        per_chan[k][0] += 1
        per_chan[k][1] += r.get("kind") in RECLAIM_KINDS
    out = []
    for r in sel:
        is_talk = line_number(r["source_file"], r["channel"]) is not None
        n = line_number(r["source_file"], r["channel"])
        a, b = int(r["audio_sector_first"]), int(r["audio_sector_last"])
        secs = b - a + 1
        if is_talk:
            st, why = status_of(refs.get(str(n)))
        else:
            st, why = "unchecked", "STR cutscene track, not an addressable XA line"
        tot, rec = per_chan[(r["source_file"], r["channel"])]
        clen = chan_len.get((r["source_file"], int(r["channel"])))
        out.append({
            "row": "clip", "clip_id": r["clip_id"], "kind": r["kind"], "source_file": r["source_file"],
            "channel": r["channel"], "clip": r["clip"], "line": n if n is not None else "",
            "xa_id": f"0x{0x30 + n:02X}" if n is not None else "",
            "sector_first": r["sector_first"], "sector_last": r["sector_last"],
            "audio_sector_first": a, "audio_sector_last": b, "audio_sectors": secs,
            "stride": r["stride"], "duration_s": r["duration_s"],
            "xa_payload_bytes": secs * XA_PAYLOAD if is_talk else 0,
            "raw_bytes": secs * SECTOR_RAW if is_talk else 0,
            "reclaimable": "yes" if is_talk else "no (stereo movie track)",
            "channel_sectors": clen if clen is not None else "",
            "channel_free": ("yes" if rec == tot else f"no ({tot - rec} speech clip(s) remain)") if is_talk else "",
            "referenced": st, "ref_detail": why,
            "language": r.get("language", ""), "language_prob": r.get("language_prob", ""),
        })
    # subtotals per file, then the grand total (TALK clips only count towards bytes)
    files = []
    for o in out:
        if o["source_file"] not in files:
            files.append(o["source_file"])
    totals = []
    for f in files:
        rs = [o for o in out if o["source_file"] == f]
        totals.append(_total_row("subtotal", f, rs, chan_len))
    totals.append(_total_row("total", "ALL", out, chan_len))
    return out + totals


def _total_row(kind, label, rs, chan_len):
    talk = [o for o in rs if o["reclaimable"] == "yes"]
    free_ch = {(o["source_file"], o["channel"]) for o in talk if o["channel_free"] == "yes"}
    ch_secs = sum(chan_len.get((f, int(c)), 0) for f, c in free_ch)
    row = {c: "" for c in COLUMNS}
    row.update({
        "row": kind, "clip_id": f"{label}: {len(rs)} clips, {len(talk)} reclaimable",
        "source_file": label, "audio_sectors": sum(o["audio_sectors"] for o in talk),
        "duration_s": round(sum(float(o["duration_s"]) for o in talk), 3),
        "xa_payload_bytes": sum(o["xa_payload_bytes"] for o in talk),
        "raw_bytes": sum(o["raw_bytes"] for o in talk),
        "reclaimable": f"{len(talk)} clips",
        "channel_sectors": ch_secs if chan_len else "",
        "channel_free": f"{len(free_ch)} whole channel(s)",
        "referenced": f"{sum(o['referenced'] == 'referenced' for o in talk)} referenced, "
                      f"{sum(o['referenced'] == 'unreferenced' for o in talk)} unreferenced, "
                      f"{sum(o['referenced'] == 'unchecked' for o in talk)} unchecked",
    })
    return row


def write_report(path, report):
    A.ensure_dir(os.path.dirname(path))
    with open(path, "w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n")
        w.writeheader()
        for r in report:
            w.writerow({k: r.get(k, "") for k in COLUMNS})


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--manifest", default=MANIFEST)
    ap.add_argument("--out", default=A.AUDIO_DIR, help="work dir with xa_line_refs.json / channels.json")
    ap.add_argument("--report", default=REPORT)
    args = ap.parse_args()

    rows = read_manifest(args.manifest)
    bad = sorted({r.get("kind", "") for r in rows} - set(KINDS) - {""})
    if bad:
        A.log(f"[reclaim] manifest has unknown kind values {bad} — they are ignored")
    refs_path = os.path.join(args.out, "xa_line_refs.json")
    refs = A.load_json(refs_path, default={}).get("lines", {})
    if not refs:
        A.log("[reclaim] no xa_line_refs.json — reference column will say unchecked (run xa_refs.py)")
    chan_len = channel_lengths(os.path.join(args.out, "channels.json"))
    report = build_report(rows, refs, chan_len)
    write_report(args.report, report)
    tot = report[-1]
    clips = [r for r in report if r["row"] == "clip"]
    by = {}
    for r in clips:
        by[r["kind"]] = by.get(r["kind"], 0) + 1
    A.log(f"[reclaim] {len(clips)} clips {by} -> {args.report}")
    A.log(f"[reclaim] TALK slots: {tot['audio_sectors']} audio sectors, {tot['duration_s']} s, "
          f"{tot['xa_payload_bytes']} B of XA payload ({tot['raw_bytes']} B raw); "
          f"{tot['channel_free']}; {tot['referenced']}")


if __name__ == "__main__":
    main()
