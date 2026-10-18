#!/usr/bin/env python3
"""
split_utterances.py — cut each decoded voice stream into utterance clips.

Stage 2 of the voice pipeline (docs/AUDIO_PIPELINE.md). Reads <out>/channels.json
from extract_voice.py, splits every stream into clips and writes
<out>/clips/<stream>_cNNN.wav plus <out>/clips.json.

Two splitters, picked per stream by --method (default `auto`):
  energy  numpy RMS gate with hysteresis (deterministic, tested in audio_test.py).
          Right for the TALK channels: dry voice over digital silence.
  vad     Silero VAD (ships with faster-whisper) on a 16 kHz downmix. Right for
          the STR cutscene tracks, where speech sits on top of music and an
          energy gate would never close.
  auto    TALK -> energy, STR -> vad.

Every clip keeps the bookkeeping needed to find it on the disc again:
start/end sample in the stream, the stream's XA sector arithmetic (ac1audio.XAStream)
and the inclusive disc-sector range those samples were decoded from, so a dubbed
line can later be re-encoded and written over exactly those sectors.

Usage:
  tools/audio/.venv/Scripts/python tools/audio/split_utterances.py [--out DIR]
      [--method auto|energy|vad] [--threshold-db -40] [--min-speech-ms 150]
      [--min-gap-ms 350] [--pad-ms 100] [--force]
"""
import argparse
import os
import sys

import numpy as np
import soundfile as sf

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402

EPS = 1e-9


def to_mono(x):
    x = np.asarray(x, dtype=np.float32)
    return x if x.ndim == 1 else x.mean(axis=1)


def frame_db(x, rate, win_ms=20.0, hop_ms=10.0):
    """RMS level in dBFS per analysis frame (window win_ms, hop hop_ms)."""
    win = max(1, int(rate * win_ms / 1000))
    hop = max(1, int(rate * hop_ms / 1000))
    if len(x) < win:
        x = np.pad(x, (0, win - len(x)))
    n = 1 + (len(x) - win) // hop
    idx = np.arange(win)[None, :] + hop * np.arange(n)[:, None]
    rms = np.sqrt((x[idx] ** 2).mean(axis=1))
    return 20 * np.log10(rms + EPS), hop


def split_by_energy(x, rate, threshold_db=-40.0, hysteresis_db=8.0, min_speech_ms=150,
                    min_gap_ms=350, pad_ms=100, win_ms=20.0, hop_ms=10.0, floor_margin_db=15.0,
                    max_raise_db=10.0):
    """Return [(start_sample, end_sample), ...] of non-silent regions of mono `x`.

    Gate opens when a frame exceeds `on` = threshold_db, raised to noise_floor +
    floor_margin_db (by at most max_raise_db) when the stream has a hiss floor,
    and closes after `min_gap_ms` of frames below `on - hysteresis_db`. Regions
    shorter than `min_speech_ms` are dropped, then each is padded by `pad_ms`
    (clamped to the signal, overlapping pads merged).
    """
    x = to_mono(x)
    if len(x) == 0:
        return []
    db, hop = frame_db(x, rate, win_ms, hop_ms)
    # Noise floor = 10th percentile frame level (digital silence clamps to -90 dBFS).
    # A hissy stream raises the gate by up to `max_raise_db`; a stream that is
    # nearly all speech (floor near speech level) must NOT push the gate above it.
    floor = max(-90.0, float(np.percentile(db, 10)))
    on = min(max(threshold_db, floor + floor_margin_db), threshold_db + max_raise_db)
    off = on - hysteresis_db
    gap_frames = max(1, int(round(min_gap_ms / hop_ms)))

    segs = []
    state = False
    start = 0
    below = 0
    for i, v in enumerate(db):
        if not state:
            if v >= on:
                state, start, below = True, i, 0
        else:
            if v >= off:
                below = 0
            else:
                below += 1
                if below >= gap_frames:
                    segs.append((start, i - below + 1))
                    state = False
    if state:
        segs.append((start, len(db)))

    win = int(rate * win_ms / 1000)
    pad = int(rate * pad_ms / 1000)
    min_len = int(rate * min_speech_ms / 1000)
    out = []
    for a, b in segs:
        s = a * hop
        e = min(len(x), (b - 1) * hop + win)
        if e - s < min_len:
            continue
        s = max(0, s - pad)
        e = min(len(x), e + pad)
        if out and s <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], e))
        else:
            out.append((s, e))
    return out


def resample(x, rate, target):
    if rate == target:
        return x.astype(np.float32)
    from math import gcd
    from scipy.signal import resample_poly
    g = gcd(rate, target)
    return resample_poly(x, target // g, rate // g).astype(np.float32)


def split_by_vad(x, rate, min_speech_ms=250, min_gap_ms=500, pad_ms=200, max_speech_s=20.0,
                 threshold=0.5):
    """Silero VAD (via faster_whisper.vad) on a 16 kHz downmix; spans scaled back."""
    from faster_whisper.vad import VadOptions, get_speech_timestamps
    x = to_mono(x)
    y = resample(x, rate, 16000)
    opts = VadOptions(threshold=threshold, min_speech_duration_ms=min_speech_ms,
                      min_silence_duration_ms=min_gap_ms, speech_pad_ms=pad_ms,
                      max_speech_duration_s=max_speech_s)
    spans = get_speech_timestamps(y, opts, sampling_rate=16000)
    out = []
    for sp in spans:
        s = int(round(sp["start"] * rate / 16000))
        e = min(len(x), int(round(sp["end"] * rate / 16000)))
        if e > s:
            out.append((s, e))
    return out


def peak_dbfs(seg):
    return float(20 * np.log10(np.abs(seg).max() + EPS)) if len(seg) else -180.0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--method", choices=["auto", "energy", "vad"], default="auto")
    ap.add_argument("--threshold-db", type=float, default=-40.0)
    ap.add_argument("--min-speech-ms", type=int, default=150)
    ap.add_argument("--min-gap-ms", type=int, default=350)
    ap.add_argument("--pad-ms", type=int, default=100)
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()

    out = args.out
    chans = A.load_json(os.path.join(out, "channels.json"))["channels"]
    clip_dir = A.ensure_dir(os.path.join(out, "clips"))
    clips = []
    per_kind = {}
    for ch in chans:
        st = A.XAStream.from_dict(ch)
        wav = os.path.join(out, ch["wav"])
        x, rate = sf.read(wav, dtype="float32", always_2d=False)
        mono = to_mono(x)
        method = args.method
        if method == "auto":
            method = "energy" if ch["kind"] == "TALK" else "vad"
        if method == "energy":
            spans = split_by_energy(mono, rate, args.threshold_db, min_speech_ms=args.min_speech_ms,
                                    min_gap_ms=args.min_gap_ms, pad_ms=args.pad_ms)
        else:
            spans = split_by_vad(mono, rate)
        for i, (s, e) in enumerate(spans):
            cid = f"{ch['name']}_c{i:03d}"
            cwav = os.path.join(clip_dir, cid + ".wav")
            if args.force or not os.path.exists(cwav):
                sf.write(cwav, mono[s:e], rate, subtype="PCM_16")
            sec_a, sec_b = st.sector_range(s, e)
            clips.append({
                "clip_id": cid,
                "stream": ch["name"],
                "kind": ch["kind"],
                "source_file": ch["source_file"],
                "stream_id": ch["id"],
                "channel": ch["channel"],
                "clip_index": i,
                "rate": rate,
                "start_sample": int(s),
                "end_sample": int(e),
                "duration_s": round((e - s) / rate, 3),
                "sector_first": sec_a,
                "sector_last": sec_b,
                "audio_sector_first": st.audio_sector_index(s),
                "audio_sector_last": st.audio_sector_index(e - 1),
                "stride": st.stride,
                "method": method,
                "peak_dbfs": round(peak_dbfs(mono[s:e]), 1),
                "wav": os.path.relpath(cwav, out).replace("\\", "/"),
            })
        k = per_kind.setdefault(ch["kind"], [0, 0.0, 0])
        k[0] += len(spans)
        k[1] += sum((e - s) for s, e in spans) / rate
        k[2] += 1
        A.log(f"[split] {ch['name']:<14} {method:<6} {len(spans):3d} clips  "
              f"{sum((e - s) for s, e in spans) / rate:6.1f}s speech of {len(mono) / rate:6.1f}s")

    A.save_json(os.path.join(out, "clips.json"), {"params": vars(args), "clips": clips})
    tot = sum(c["duration_s"] for c in clips)
    A.log(f"[split] {len(clips)} clips, {tot:.0f} s ({tot/60:.1f} min) of speech")
    for k, (n, secs, streams) in sorted(per_kind.items()):
        A.log(f"        {k}: {n} clips from {streams} streams, {secs:.0f} s")
    A.log(f"[split] -> {os.path.join(out, 'clips.json')}")


if __name__ == "__main__":
    main()
