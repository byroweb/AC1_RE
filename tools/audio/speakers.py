#!/usr/bin/env python3
"""
speakers.py — tag who is speaking: speaker embeddings + clustering + voice stats.

Stage 4 of the voice pipeline (docs/AUDIO_PIPELINE.md). For every clip in
<out>/clips.json:
  * a 192-d ECAPA-TDNN speaker embedding (SpeechBrain spkrec-ecapa-voxceleb,
    CPU; the model is fetched once into <out>/_models/),
  * median pitch (F0) from a normalised-autocorrelation tracker,
  * a "processed / robotic voice" heuristic: harmonicity (median autocorrelation
    peak of energetic frames) and spectral flatness. Vocoded or ring-modulated
    voices score low on harmonicity and high on flatness.
Embeddings are clustered with average-linkage agglomerative clustering on cosine
distance; clusters become voice_00, voice_01, ... (largest first). The owner
names them once (e.g. voice_02 = operator woman) via the review page or the
manifest's notes.

Writes <out>/speakers.json  {clips: {clip_id: {voice, f0_hz, harmonicity,
flatness, processed, duplicate_of}}, voices: [{voice, n_clips, total_s, f0_median_hz,
processed_frac, streams, examples}]} and <out>/speakers_emb.npz (so
--recluster can retune the threshold without re-embedding).

Usage:
  tools/audio/.venv/Scripts/python tools/audio/speakers.py [--threshold 0.72 | --n-voices N]
      [--recluster] [--min-clip-s 0.4]
"""
import argparse
import os
import sys

import numpy as np
import soundfile as sf

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
from split_utterances import resample, to_mono  # noqa: E402

SR = 16000


# ------------------------------------------------------------- pitch / timbre
def pitch_and_timbre(y, sr=SR, fmin=60.0, fmax=400.0, win_ms=40.0, hop_ms=20.0, energy_db=-35.0):
    """Median F0 (Hz, or None if unvoiced), harmonicity, spectral flatness of mono `y`."""
    win = int(sr * win_ms / 1000)
    hop = int(sr * hop_ms / 1000)
    if len(y) < win:
        return None, 0.0, 1.0
    n = 1 + (len(y) - win) // hop
    idx = np.arange(win)[None, :] + hop * np.arange(n)[:, None]
    frames = y[idx] * np.hanning(win)[None, :]
    rms_db = 20 * np.log10(np.sqrt((frames ** 2).mean(axis=1)) + 1e-9)
    energetic = rms_db > max(energy_db, rms_db.max() - 25)
    if not energetic.any():
        return None, 0.0, 1.0
    fr = frames[energetic] - frames[energetic].mean(axis=1, keepdims=True)
    nfft = 1 << (2 * win - 1).bit_length()
    spec = np.fft.rfft(fr, nfft)
    power = np.abs(spec) ** 2
    ac = np.fft.irfft(power, nfft)[:, :win]
    ac = ac / (ac[:, :1] + 1e-12)
    lo, hi = int(sr / fmax), int(sr / fmin)
    seg = ac[:, lo:hi + 1]
    peak_lag = seg.argmax(axis=1) + lo
    peak_val = seg.max(axis=1)
    voiced = peak_val > 0.5
    f0 = float(np.median(sr / peak_lag[voiced])) if voiced.any() else None
    harmonicity = float(np.median(peak_val))
    # spectral flatness over the speech band (geometric / arithmetic mean of power)
    band = power[:, int(100 * nfft / sr): int(4000 * nfft / sr)] + 1e-12
    flat = np.exp(np.log(band).mean(axis=1)) / band.mean(axis=1)
    flatness = float(np.median(flat))
    return (round(f0, 1) if f0 else None), round(harmonicity, 3), round(flatness, 4)


# Calibrated on the AC1 TALK clips: median harmonicity of the (radio-filtered) lines
# is ~0.51, 10th percentile ~0.34; flatness median ~0.09. Vocoded / ring-modulated
# voices and music-backed cutscene speech fall below / above these.
HARMONICITY_MIN = 0.40
FLATNESS_MAX = 0.20
DUPLICATE_DIST = 0.03   # cosine distance under which two clips are the same recording


def processed_flag(harmonicity, flatness):
    """Heuristic: processed/robotic if the voice is weakly periodic or noise-like."""
    return bool(harmonicity < HARMONICITY_MIN or flatness > FLATNESS_MAX)


def near_duplicates(embs, clips, max_dist=DUPLICATE_DIST):
    """clip_id -> clip_id of an earlier clip that is (almost) the same recording:
    embedding cosine distance < max_dist and duration within 10 %."""
    from scipy.spatial.distance import cdist
    D = cdist(embs, embs, "cosine")
    dup = {}
    for j in range(len(clips)):
        for i in range(j):
            if D[i, j] < max_dist and abs(clips[i]["duration_s"] - clips[j]["duration_s"])                     <= 0.1 * max(clips[i]["duration_s"], clips[j]["duration_s"]):
                dup[clips[j]["clip_id"]] = dup.get(clips[i]["clip_id"], clips[i]["clip_id"])
                break
    return dup


# ------------------------------------------------------------- embeddings
def load_encoder(models_dir):
    import torch  # noqa: F401  (import check)
    from speechbrain.inference.speaker import EncoderClassifier
    kw = {"source": "speechbrain/spkrec-ecapa-voxceleb",
          "savedir": os.path.join(models_dir, "spkrec-ecapa-voxceleb"),
          "run_opts": {"device": "cpu"}}
    try:
        from speechbrain.utils.fetching import LocalStrategy
        return EncoderClassifier.from_hparams(local_strategy=LocalStrategy.COPY, **kw)
    except TypeError:  # older speechbrain without local_strategy
        return EncoderClassifier.from_hparams(**kw)


def embed(enc, y):
    import torch
    with torch.no_grad():
        e = enc.encode_batch(torch.from_numpy(y)[None, :]).squeeze().cpu().numpy()
    return e / (np.linalg.norm(e) + 1e-9)


def cluster(embs, threshold=None, n_voices=None):
    from scipy.cluster.hierarchy import fcluster, linkage
    if len(embs) == 1:
        return np.zeros(1, dtype=int)
    Z = linkage(embs, method="average", metric="cosine")
    if n_voices:
        labels = fcluster(Z, t=n_voices, criterion="maxclust")
    else:
        labels = fcluster(Z, t=threshold, criterion="distance")
    # relabel by cluster size, largest first
    ids, counts = np.unique(labels, return_counts=True)
    order = {old: new for new, old in enumerate(ids[np.argsort(-counts, kind="stable")])}
    return np.array([order[l] for l in labels])


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--threshold", type=float, default=0.72,
                    help="cosine-distance cut for agglomerative clustering (lower = more voices); "
                         "0.7-0.75 gives ~15-18 voices on the AC1 lines")
    ap.add_argument("--n-voices", type=int, help="force this many clusters instead of --threshold")
    ap.add_argument("--min-clip-s", type=float, default=0.4,
                    help="clips shorter than this get an embedding but are clustered last")
    ap.add_argument("--recluster", action="store_true", help="reuse speakers_emb.npz, only recluster")
    args = ap.parse_args()

    out = args.out
    clips = A.load_json(os.path.join(out, "clips.json"))["clips"]
    emb_path = os.path.join(out, "speakers_emb.npz")
    ids = [c["clip_id"] for c in clips]

    if args.recluster and os.path.exists(emb_path):
        z = np.load(emb_path, allow_pickle=True)
        saved = {cid: (e, f) for cid, e, f in zip(z["ids"], z["embs"], z["feats"])}
        if set(saved) != set(ids):
            sys.exit("speakers_emb.npz does not match clips.json — rerun without --recluster")
        embs = np.stack([saved[c][0] for c in ids])
        feats = [tuple(saved[c][1]) for c in ids]
    else:
        enc = load_encoder(A.ensure_dir(os.path.join(out, "_models")))
        embs, feats = [], []
        for n, c in enumerate(clips, 1):
            x, rate = sf.read(os.path.join(out, c["wav"]), dtype="float32", always_2d=False)
            y = resample(to_mono(x), rate, SR)
            if len(y) < int(0.3 * SR):
                y = np.pad(y, (0, int(0.3 * SR) - len(y)))
            embs.append(embed(enc, y))
            feats.append(pitch_and_timbre(y))
            if n % 20 == 0 or n == len(clips):
                A.log(f"[speakers] embedded {n}/{len(clips)}")
        embs = np.stack(embs)
        np.savez(emb_path, ids=np.array(ids), embs=embs,
                 feats=np.array([[f[0] or 0.0, f[1], f[2]] for f in feats], dtype=float))

    labels = cluster(embs, args.threshold, args.n_voices)
    dups = near_duplicates(embs, clips)
    per_clip = {}
    voices = {}
    for c, lab, (f0, harm, flat) in zip(clips, labels, feats):
        f0 = float(f0) if f0 else None
        v = f"voice_{int(lab):02d}"
        proc = processed_flag(float(harm), float(flat))
        per_clip[c["clip_id"]] = {"voice": v, "f0_hz": f0, "harmonicity": round(float(harm), 3),
                                  "flatness": round(float(flat), 4), "processed": proc,
                                  "duplicate_of": dups.get(c["clip_id"], "")}
        d = voices.setdefault(v, {"voice": v, "n_clips": 0, "total_s": 0.0, "f0s": [],
                                  "processed": 0, "streams": set(), "clips": []})
        d["n_clips"] += 1
        d["total_s"] += c["duration_s"]
        if f0:
            d["f0s"].append(f0)
        d["processed"] += int(proc)
        d["streams"].add(c["stream"])
        d["clips"].append((c["duration_s"], c["clip_id"]))

    summary = []
    for v in sorted(voices, key=lambda k: (-voices[k]["n_clips"], k)):
        d = voices[v]
        d["clips"].sort(reverse=True)
        summary.append({
            "voice": v, "n_clips": d["n_clips"], "total_s": round(d["total_s"], 1),
            "f0_median_hz": round(float(np.median(d["f0s"])), 1) if d["f0s"] else None,
            "processed_frac": round(d["processed"] / d["n_clips"], 2),
            "streams": sorted(d["streams"]),
            "examples": [cid for _, cid in d["clips"][:3]],
            "label": "",  # owner fills in: e.g. "operator woman", "Hustler One", "robot"
        })
    A.save_json(os.path.join(out, "speakers.json"), {
        "params": {"threshold": args.threshold, "n_voices": args.n_voices,
                   "model": "speechbrain/spkrec-ecapa-voxceleb", "linkage": "average/cosine"},
        "clips": per_clip, "voices": summary})
    A.log(f"[speakers] {len(summary)} voices from {len(clips)} clips "
          f"(threshold {args.threshold if not args.n_voices else 'n=' + str(args.n_voices)}); "
          f"{len(dups)} near-duplicate recordings")
    for s in summary:
        A.log(f"  {s['voice']}: {s['n_clips']:3d} clips {s['total_s']:6.1f}s  "
              f"F0~{s['f0_median_hz']} Hz  processed {s['processed_frac']:.0%}  e.g. {s['examples'][0]}")
    A.log(f"[speakers] -> {os.path.join(out, 'speakers.json')}")


if __name__ == "__main__":
    main()
