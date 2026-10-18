#!/usr/bin/env python3
"""
classify.py — the non-speech gate: speech_en / speech_ja / ambience / sfx / unknown.

Stage 4 of the voice pipeline (docs/AUDIO_PIPELINE.md). Whisper will happily
write confident English over a Japanese line, a reactor hum or a klaxon, so
each clip is classified from three independent views before anything
downstream trusts its transcript:

  * Whisper's own doubt  — no_speech_prob, the detected language and its
    probability (transcribe.py now detects per clip), avg_logprob, and the
    stock hallucination phrases;
  * the signal            — 16 kHz mono features that do not need a model:
      voiced     fraction of active frames with a clear pitch (autocorrelation
                 peak > 0.5 in 60–400 Hz); speech 0.1–0.8, noise ~0
      flat       median spectral flatness, 100–4000 Hz; speech < 0.2, hiss > 0.3
      env_std    std of frame level (dB) over active frames — syllables make
                 speech bumpy (~6–9 dB), a steady bed is flat (< 3 dB)
      mod        share of envelope modulation energy at 2–10 Hz (syllable rate)
      crest      peak-to-RMS (dB): clicks and shots are spiky, beds are not
      cent       median spectral centroid (Hz)
  * the owner             — the manifest's `kind` column is the owner's: a row
                 whose kind differs from its kind_auto was changed by hand (TSV
                 edit or the review page's dropdown) and that kind wins.

Writes <out>/kinds.json: clip_id -> {kind, kind_auto, override, uncertain,
reasons, features}. `kind` is what the later stages use: speakers.py clusters
only speech (ambience/sfx go to a `nonspeech` pseudo-voice), match_text.py
skips non-speech, build_manifest.py blanks their transcript.

The thresholds below were set on the 133 AC1 clips (see docs); they are
plain constants, retune them if a different split or model moves the
distributions, and read `uncertain` as "listen to this one".

Usage:
  tools/audio/.venv/Scripts/python tools/audio/classify.py [--out DIR] [--manifest TSV]
      [--no-override] [--print]
"""
import argparse
import csv
import os
import sys

import numpy as np
import soundfile as sf

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
from split_utterances import resample, to_mono  # noqa: E402

SR = 16000
KINDS = ["speech_en", "speech_ja", "ambience", "sfx", "unknown"]
SPEECH = {"speech_en", "speech_ja"}
NONSPEECH = {"ambience", "sfx"}

# --- thresholds (calibrated on the AC1 clips; see docs/AUDIO_PIPELINE.md §3) ---
NO_SPEECH_HIGH = 0.60      # Whisper itself says "no speech"
NO_SPEECH_GREY = 0.30      # from here up Whisper is unsure
LANG_SURE = 0.80           # language probability that counts as a clear call
LANG_WEAK = 0.50           # below this the "language" is a coin toss
VOICED_SPEECH = 0.08       # below: no pitched frames worth the name
FLAT_NOISE = 0.25          # above: noise-like spectrum
ENV_STEADY_DB = 3.0        # below: a steady bed (no syllables); STR speech sits at 3.9+
MOD_SPEECH = 0.15          # below: no syllable-rate modulation (STR speech >= 0.19)
SFX_MAX_S = 1.5            # a non-speech clip this short is a one-shot effect
SFX_CREST_DB = 20.0        # or this spiky
HALLUCINATIONS = ("thank you for watching", "thanks for watching", "subtitles by", "bye-bye",
                  "please subscribe", "you", "thank you", "thank you so much", "bye")


# ------------------------------------------------------------------ features
def audio_features(y, sr=SR, win_ms=25.0, hop_ms=10.0, active_db=30.0, nfft=1024):
    """Signal-only features of mono float audio `y` (see module doc)."""
    y = np.asarray(y, dtype=np.float32)
    dur = len(y) / sr
    win = max(1, int(sr * win_ms / 1000))
    hop = max(1, int(sr * hop_ms / 1000))
    if len(y) < win:
        y = np.pad(y, (0, win - len(y)))
    n = 1 + (len(y) - win) // hop
    idx = np.arange(win)[None, :] + hop * np.arange(n)[:, None]
    fr = y[idx]
    e = (fr ** 2).mean(axis=1)
    db = 10 * np.log10(e + 1e-12)
    peak = float(db.max())
    active = db > peak - active_db
    n_act = int(active.sum())
    env_std = float(db[active].std()) if n_act > 2 else 0.0

    env = np.sqrt(e)
    env = env - env.mean()
    mod = 0.0
    if len(env) > 16:
        sp = np.abs(np.fft.rfft(env * np.hanning(len(env)))) ** 2
        f = np.fft.rfftfreq(len(env), d=hop / sr)
        tot = float(sp[(f >= 0.5) & (f <= 50)].sum()) + 1e-12
        mod = float(sp[(f >= 2) & (f <= 10)].sum() / tot)

    w = fr * np.hanning(win)
    P = np.abs(np.fft.rfft(w, nfft)) ** 2
    ac = np.fft.irfft(P, nfft)[:, :win]
    ac = ac / (ac[:, :1] + 1e-12)
    lo, hi = int(sr / 400), int(sr / 60)
    pk = ac[:, lo:hi + 1].max(axis=1)
    voiced = float((pk[active] > 0.5).mean()) if n_act else 0.0
    band = P[:, int(100 * nfft / sr):int(4000 * nfft / sr)] + 1e-12
    flat = np.exp(np.log(band).mean(axis=1)) / band.mean(axis=1)
    flat_med = float(np.median(flat[active])) if n_act else 1.0
    freqs = np.fft.rfftfreq(nfft, 1 / sr)
    cent = (P * freqs).sum(axis=1) / (P.sum(axis=1) + 1e-12)
    cent_med = float(np.median(cent[active])) if n_act else 0.0
    rms_db = float(10 * np.log10((y ** 2).mean() + 1e-12))
    crest = float(20 * np.log10(np.abs(y).max() + 1e-9) - rms_db)
    return {
        "dur": round(dur, 3), "rms_db": round(rms_db, 1), "crest": round(crest, 1),
        "env_std": round(env_std, 2), "mod": round(mod, 3), "voiced": round(voiced, 3),
        "flat": round(flat_med, 4), "cent": round(cent_med), "active": round(float(active.mean()), 2),
    }


# ---------------------------------------------------------------------- gate
def is_hallucination(text):
    t = (text or "").lower().strip(" .!?,")
    return not t or t in HALLUCINATIONS


def classify_clip(t, f):
    """(kind, uncertain, reasons) from a transcripts.json record `t` (language,
    language_prob, lang_probs, no_speech_prob, avg_logprob, text) and audio
    features `f` (audio_features()). Pure function — unit-tested."""
    nsp = float(t.get("no_speech_prob", 1.0))
    lang = t.get("language") or ""
    lprob = float(t.get("language_prob", 0.0) or 0.0)
    probs = t.get("lang_probs") or {}
    en_p = float(probs.get("en", lprob if lang == "en" else 0.0))
    ja_p = float(probs.get("ja", lprob if lang == "ja" else 0.0))
    text = (t.get("text") or "").strip()
    voiced, flat, env_std, mod = f["voiced"], f["flat"], f["env_std"], f["mod"]
    dur, crest = f["dur"], f["crest"]

    reasons = []
    # evidence against speech
    noise_like = voiced < VOICED_SPEECH and flat > FLAT_NOISE
    steady = env_std < ENV_STEADY_DB or mod < MOD_SPEECH
    whisper_doubts = nsp >= NO_SPEECH_HIGH
    lang_weak = lprob < LANG_WEAK
    empty = is_hallucination(text)
    against = 0
    if whisper_doubts:
        against += 2
        reasons.append(f"no_speech_prob {nsp:.2f}")
    elif nsp >= NO_SPEECH_GREY:
        against += 1
        reasons.append(f"no_speech_prob {nsp:.2f}")
    if noise_like:
        against += 2
        reasons.append(f"unvoiced noise-like signal (voiced {voiced:.2f}, flatness {flat:.2f})")
    elif voiced < VOICED_SPEECH:
        against += 1
        reasons.append(f"no pitched frames (voiced {voiced:.2f})")
    if steady:
        against += 1
        reasons.append(f"no syllable rhythm (env_std {env_std:.1f} dB, mod {mod:.2f})")
    if lang_weak:
        against += 1
        reasons.append(f"language {lang or '?'} only {lprob:.2f}")
    if empty:
        against += 2
        reasons.append("empty / stock hallucination text" if text else "no text")

    # evidence for speech
    for_speech = 0
    if lprob >= LANG_SURE and lang in ("en", "ja"):
        for_speech += 2
    elif lprob >= LANG_WEAK:
        for_speech += 1
    if nsp < 0.1:
        for_speech += 1
    if voiced >= 0.3 and not steady:
        for_speech += 1

    if against >= 3 and against > for_speech:
        if not (whisper_doubts or noise_like or steady or (empty and lang_weak)):
            # Whisper found nothing usable but the signal itself looks like a voice
            reasons.append("signal looks voiced — listen")
            return "unknown", True, "; ".join(reasons)
        kind = "sfx" if (dur < SFX_MAX_S or crest > SFX_CREST_DB) else "ambience"
        uncertain = for_speech >= 2 or (not whisper_doubts and not noise_like and not empty)
        return kind, uncertain, "; ".join(reasons)

    # speech (or at least not clearly non-speech)
    if lang == "ja" and lprob >= LANG_WEAK:
        kind = "speech_ja"
    elif lang == "en" and lprob >= LANG_WEAK:
        kind = "speech_en"
    elif ja_p >= LANG_WEAK:
        kind = "speech_ja"
    elif en_p >= LANG_WEAK:
        kind = "speech_en"
    else:
        reasons.append(f"language unclear ({lang or '?'} {lprob:.2f}, en {en_p:.2f}, ja {ja_p:.2f})")
        return "unknown", True, "; ".join(reasons)
    if LANG_WEAK <= lprob < LANG_SURE:
        reasons.append(f"language {lang} only {lprob:.2f}")
    uncertain = against >= 2 or lprob < LANG_SURE or empty
    if lang in ("en", "ja") and lprob >= LANG_SURE and abs(en_p - ja_p) < 0.3:
        uncertain = True
        reasons.append(f"en {en_p:.2f} vs ja {ja_p:.2f}")
    return kind, uncertain, "; ".join(reasons)


# ---------------------------------------------------------------------- main
def load_overrides(manifest):
    """{clip_id: kind} for every manifest row whose kind the owner changed, i.e.
    kind is a valid kind and differs from kind_auto (or kind_auto is missing)."""
    if not manifest or not os.path.exists(manifest):
        return {}
    out = {}
    with open(manifest, encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            kind = (r.get("kind") or "").strip()
            auto = (r.get("kind_auto") or "").strip()
            if kind in KINDS and kind != auto:
                out[r["clip_id"]] = kind
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--manifest", default=os.path.join(A.ROOT, "disc_map", "voice_manifest.tsv"),
                    help="existing manifest whose owner-set kind column is honoured")
    ap.add_argument("--no-override", action="store_true", help="ignore the manifest's kind column")
    ap.add_argument("--print", action="store_true", help="one line per clip with the features")
    args = ap.parse_args()

    out = args.out
    clips = A.load_json(os.path.join(out, "clips.json"))["clips"]
    trans = A.load_json(os.path.join(out, "transcripts.json"))
    overrides = {} if args.no_override else load_overrides(args.manifest)

    kinds = {}
    for c in clips:
        cid = c["clip_id"]
        t = trans.get(cid)
        if t is None:
            A.log(f"[classify] {cid}: no transcript — run transcribe.py first")
            continue
        x, rate = sf.read(os.path.join(out, c["wav"]), dtype="float32", always_2d=False)
        f = audio_features(resample(to_mono(x), rate, SR))
        kind, unsure, why = classify_clip(t, f)
        rec = {"kind": overrides.get(cid, kind), "kind_auto": kind, "override": overrides.get(cid, ""),
               "uncertain": bool(unsure), "reasons": why, "language": t.get("language", ""),
               "language_prob": t.get("language_prob", 0.0), "features": f}
        kinds[cid] = rec
        if args.print:
            A.log(f"{cid:<18} {rec['kind']:<9}{'?' if unsure else ' '} {t.get('language','?')}:"
                  f"{float(t.get('language_prob', 0)):.2f} nsp={float(t.get('no_speech_prob', 1)):.2f} "
                  f"voiced={f['voiced']:.2f} flat={f['flat']:.3f} env={f['env_std']:.1f} mod={f['mod']:.2f} "
                  f"crest={f['crest']:.0f} {why}")
    A.save_json(os.path.join(out, "kinds.json"), kinds)
    by = {}
    for r in kinds.values():
        by[r["kind"]] = by.get(r["kind"], 0) + 1
    unsure = [cid for cid, r in kinds.items() if r["uncertain"] and not r["override"]]
    A.log(f"[classify] {len(kinds)} clips: {dict(sorted(by.items()))}; "
          f"{len(overrides)} owner overrides; {len(unsure)} uncertain"
          + (f": {', '.join(unsure)}" if unsure else ""))
    A.log(f"[classify] -> {os.path.join(out, 'kinds.json')}")


if __name__ == "__main__":
    main()
