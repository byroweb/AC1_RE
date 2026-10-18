#!/usr/bin/env python3
"""
transcribe.py — speech-to-text for every clip with faster-whisper on the CPU.

Stage 3 of the voice pipeline (docs/AUDIO_PIPELINE.md). Reads <out>/clips.json,
runs Whisper (default `large-v3`, int8, all CPU threads) on each clip and writes
<out>/transcripts.json:
  clip_id -> { text, language, avg_logprob, no_speech_prob, compression_ratio,
               words: [{word, start, end, probability}], model }

An initial_prompt seeds the decoder with AC1 vocabulary so proper nouns come out
as the game spells them (Raven, Ravens' Nest, Chrome, Murakumo Millennium,
Nine-Ball, Hustler One, Progtech, AC, MT, Isaac City ...). Edit AC1_PROMPT below
or pass --prompt-file to extend it.

Clips are 37800 Hz PCM; they are resampled to 16 kHz in numpy and handed to the
model as arrays (no ffmpeg needed). Already-transcribed clips are skipped unless
--force, so an interrupted run resumes.

Usage:
  tools/audio/.venv/Scripts/python tools/audio/transcribe.py [--model large-v3]
      [--compute-type int8] [--threads N] [--beam 5] [--only TALK|STR] [--limit N] [--force]
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


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--out", default=A.AUDIO_DIR)
    ap.add_argument("--model", default="large-v3",
                    help="faster-whisper model id/path (large-v3, distil-large-v3, medium.en ...)")
    ap.add_argument("--compute-type", default="int8")
    ap.add_argument("--threads", type=int, default=max(1, (os.cpu_count() or 4) - 2))
    ap.add_argument("--beam", type=int, default=5)
    ap.add_argument("--language", default="en")
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
        x, rate = sf.read(os.path.join(out, c["wav"]), dtype="float32", always_2d=False)
        y = resample(to_mono(x), rate, 16000)
        # whisper pads to 30 s anyway; very short clips benefit from a little tail silence
        if len(y) < 16000:
            y = np.pad(y, (0, 16000 - len(y)))
        segs, info = model.transcribe(
            y, language=args.language, beam_size=args.beam, best_of=args.beam,
            word_timestamps=True, initial_prompt=prompt, condition_on_previous_text=False,
            temperature=[0.0, 0.2, 0.4], vad_filter=False, without_timestamps=False)
        segs = list(segs)
        words = []
        text = " ".join(s.text.strip() for s in segs).strip()
        for s in segs:
            for w in (s.words or []):
                words.append({"word": w.word.strip(), "start": round(w.start, 3),
                              "end": round(w.end, 3), "probability": round(w.probability, 3)})
        dur = sum((s.end - s.start) for s in segs) or 1.0
        avg_lp = sum(s.avg_logprob * (s.end - s.start) for s in segs) / dur if segs else -10.0
        nsp = min((s.no_speech_prob for s in segs), default=1.0)
        cr = max((s.compression_ratio for s in segs), default=0.0)
        results[c["clip_id"]] = {
            "text": text, "language": info.language, "avg_logprob": round(avg_lp, 4),
            "no_speech_prob": round(nsp, 4), "compression_ratio": round(cr, 3),
            "words": words, "model": args.model,
        }
        done_s += c["duration_s"]
        el = time.time() - t0
        A.log(f"[transcribe] {n}/{len(todo)} {c['clip_id']:<20} lp={avg_lp:6.2f} nsp={nsp:.2f} "
              f"{text[:70]!r}  ({done_s/max(el,1e-6):.1f}x realtime)")
        if n % 10 == 0 or n == len(todo):
            A.save_json(tpath, results)
    A.save_json(tpath, results)
    A.log(f"[transcribe] {len(results)} transcripts -> {tpath}  ({time.time()-t0:.0f} s)")


if __name__ == "__main__":
    main()
