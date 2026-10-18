#!/usr/bin/env python3
"""
transcribe.py — speech-to-text for every clip with faster-whisper on the CPU.

Stage 3 of the voice pipeline (docs/AUDIO_PIPELINE.md). Reads <out>/clips.json,
runs Whisper (default `large-v3`, int8, all CPU threads) on each clip and writes
<out>/transcripts.json:
  clip_id -> { text, language, language_prob, lang_probs: {en, ja, ...top 5},
               avg_logprob, no_speech_prob, compression_ratio,
               words: [{word, start, end, probability}], model, schema }

Per clip, two steps:
  1. language detection with no prompt (`WhisperModel.detect_language`): the
     detected code and its probability are recorded, plus the top-5 language
     probabilities so the non-speech gate (classify.py) can see how undecided
     the model was. Forcing English (the old behaviour) hallucinated confident
     English out of Japanese lines and ambience beds.
  2. transcription in the detected language. Only English clips get the AC1
     vocabulary initial_prompt (Raven, Ravens' Nest, Chrome, Murakumo
     Millennium, Nine-Ball, Hustler One, Progtech, AC, MT, Isaac City ...);
     a Japanese clip is decoded as Japanese with no prompt, so `text` is the
     Japanese line. Edit AC1_PROMPT below or pass --prompt-file to extend it.
  There is deliberately no translation pass: the Japanese lines are located so
  their disc space can be reclaimed (classify.py, reclaimable.py), not read.

Pass --language en to force the old single-language behaviour. Clips are
37800 Hz PCM; they are resampled to 16 kHz in numpy and handed to the model as
arrays (no ffmpeg needed). Already-transcribed clips are skipped unless --force,
so an interrupted run resumes; records written by the pre-language-detection
version of this script (schema 1) are redone automatically.

Usage:
  tools/audio/.venv/Scripts/python tools/audio/transcribe.py [--model large-v3]
      [--compute-type int8] [--threads N] [--beam 5] [--language auto|en|ja]
      [--only TALK|STR] [--limit N] [--force]
First run downloads the model (~3 GB for large-v3) into the Hugging Face cache.
"""
import argparse
import os
import sys
import time

import numpy as np
import soundfile as sf

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
from split_utterances import resample, to_mono  # noqa: E402

AC1_PROMPT = (
    "Armored Core radio chatter. Raven, Ravens' Nest, Chrome, Murakumo Millennium, "
    "Nine-Ball, Hustler One, Progtech, AC, MT, MTs, Isaac City, Imminent Storm, "
    "Struggle, Dark Soul, Justice, Guard, Plus, Great Destruction, mission, "
    "target, operator, enemy forces, mission complete, mission failed."
)
SR = 16000
TOP_LANGS = 5
SCHEMA = 2   # per-clip record version; older records are redone on the next run


def load_clip_16k(path):
    x, rate = sf.read(path, dtype="float32", always_2d=False)
    y = resample(to_mono(x), rate, SR)
    # whisper pads to 30 s anyway; very short clips benefit from a little tail silence
    if len(y) < SR:
        y = np.pad(y, (0, SR - len(y)))
    return y


def decode(model, y, language, prompt, beam):
    """One Whisper decode -> (text, words, avg_logprob, no_speech_prob, compression_ratio)."""
    segs, _info = model.transcribe(
        y, language=language, beam_size=beam, best_of=beam,
        word_timestamps=True, initial_prompt=prompt, condition_on_previous_text=False,
        temperature=[0.0, 0.2, 0.4], vad_filter=False, without_timestamps=False)
    segs = list(segs)
    text = " ".join(s.text.strip() for s in segs).strip()
    words = [{"word": w.word.strip(), "start": round(w.start, 3), "end": round(w.end, 3),
              "probability": round(w.probability, 3)}
             for s in segs for w in (s.words or [])]
    dur = sum((s.end - s.start) for s in segs) or 1.0
    avg_lp = sum(s.avg_logprob * (s.end - s.start) for s in segs) / dur if segs else -10.0
    nsp = min((s.no_speech_prob for s in segs), default=1.0)
    cr = max((s.compression_ratio for s in segs), default=0.0)
    return text, words, round(avg_lp, 4), round(nsp, 4), round(cr, 3)


def top_language_probs(all_probs, n=TOP_LANGS):
    """{code: prob} of the n most likely languages, always including en and ja."""
    probs = dict(all_probs)
    top = dict(sorted(probs.items(), key=lambda kv: -kv[1])[:n])
    for k in ("en", "ja"):
        top.setdefault(k, probs.get(k, 0.0))
    return {k: round(float(v), 4) for k, v in top.items()}


def transcribe_clip(model, y, prompt, beam, force_language="auto"):
    """Detect the language and transcribe in it. Returns the transcripts.json
    record (minus `model`)."""
    lang, lprob, all_probs = model.detect_language(y)
    top = top_language_probs(all_probs)
    if force_language and force_language != "auto":
        lang, lprob = force_language, dict(all_probs).get(force_language, 0.0)
    is_en = lang == "en"
    text, words, avg_lp, nsp, cr = decode(model, y, lang, prompt if is_en else None, beam)
    return {
        "text": text, "language": lang, "language_prob": round(float(lprob), 4),
        "lang_probs": top,
        "avg_logprob": avg_lp, "no_speech_prob": nsp, "compression_ratio": cr,
        "words": words, "schema": SCHEMA,
    }


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--model", default="large-v3",
                    help="faster-whisper model id/path (large-v3, distil-large-v3, medium.en ...)")
    ap.add_argument("--compute-type", default="int8")
    ap.add_argument("--threads", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    ap.add_argument("--beam", type=int, default=5)
    ap.add_argument("--language", default="auto",
                    help="auto = detect per clip (default); a code such as en or ja forces it")
    ap.add_argument("--prompt-file", help="text file appended to the built-in AC1 vocabulary prompt")
    ap.add_argument("--only", choices=["TALK", "STR"])
    ap.add_argument("--limit", type=int, help="stop after N clips (smoke test)")
    ap.add_argument("--force", action="store_true")
    args = ap.parse_args()

    out = args.out
    clips = A.load_json(os.path.join(out, "clips.json"))["clips"]
    if args.only:
        clips = [c for c in clips if c["kind"] == args.only]
    tpath = os.path.join(out, "transcripts.json")
    results = A.load_json(tpath, default={}) if not args.force else {}
    stale = [k for k, v in results.items() if v.get("schema", 1) < SCHEMA]
    if stale:
        A.log(f"[transcribe] {len(stale)} records from the old single-language run will be redone")
        for k in stale:
            del results[k]
    todo = [c for c in clips if c["clip_id"] not in results]
    if args.limit:
        todo = todo[:args.limit]
    A.log(f"[transcribe] {len(todo)} clips to do ({len(clips) - len(todo)} already done)")
    if not todo:
        return

    prompt = AC1_PROMPT
    if args.prompt_file:
        with open(args.prompt_file, encoding="utf-8") as f:
            prompt = prompt + " " + f.read().strip()

    from faster_whisper import WhisperModel
    A.log(f"[transcribe] loading {args.model} ({args.compute_type}, {args.threads} threads)")
    model = WhisperModel(args.model, device="cpu", compute_type=args.compute_type,
                         cpu_threads=args.threads)

    t0 = time.time()
    done_s = 0.0
    for n, c in enumerate(todo, 1):
        y = load_clip_16k(os.path.join(out, c["wav"]))
        rec = transcribe_clip(model, y, prompt, args.beam, args.language)
        rec["model"] = args.model
        results[c["clip_id"]] = rec
        done_s += c["duration_s"]
        el = time.time() - t0
        A.log(f"[transcribe] {n}/{len(todo)} {c['clip_id']:<20} {rec['language']}:{rec['language_prob']:.2f} "
              f"lp={rec['avg_logprob']:6.2f} nsp={rec['no_speech_prob']:.2f} {rec['text'][:80]!r}  "
              f"({done_s/max(el,1e-6):.1f}x realtime)")
        if n % 10 == 0 or n == len(todo):
            A.save_json(tpath, results)
    A.save_json(tpath, results)
    langs = {}
    for v in results.values():
        langs[v["language"]] = langs.get(v["language"], 0) + 1
    A.log(f"[transcribe] {len(results)} transcripts -> {tpath}  languages {langs}  ({time.time()-t0:.0f} s)")


if __name__ == "__main__":
    main()
