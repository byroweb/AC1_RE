#!/usr/bin/env python3
"""
run_all.py — the whole voice pipeline in one command.

  extract_voice -> split_utterances -> transcribe -> classify -> speakers -> match_text
      -> build_manifest -> xa_refs -> reclaimable

Each stage is a script in this directory and resumes from what is already on
disk (decoded WAVs, finished transcripts), so re-running after an interruption
or a parameter tweak only redoes what changed. Pass --from/--to to run a slice,
--force to redo everything, and anything after `--` goes to transcribe.py
(e.g. `-- --model distil-large-v3`).

Run from Git Bash at the repo root with the venv interpreter (see
tools/audio/setup_audio.sh / docs/AUDIO_PIPELINE.md):
  tools/audio/.venv/Scripts/python tools/audio/run_all.py
  tools/audio/.venv/Scripts/python tools/audio/run_all.py --from speakers
  tools/audio/.venv/Scripts/python tools/audio/run_all.py --skip-demoplay -- --model distil-large-v3
  tools/audio/.venv/Scripts/python tools/audio/run_all.py --from match \
      --out /path/to/AUDIO --corpus /path/to/MIS_T/toc.json --fdat-entries /path/to/FDAT_T/entries
"""
import argparse
import os
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
STAGES = ["extract", "split", "transcribe", "classify", "speakers", "match", "manifest", "refs", "reclaim"]
SCRIPTS = {
    "extract": "extract_voice.py", "split": "split_utterances.py", "transcribe": "transcribe.py",
    "classify": "classify.py", "speakers": "speakers.py", "match": "match_text.py", "manifest": "build_manifest.py",
    "refs": "xa_refs.py", "reclaim": "reclaimable.py",
}


def main():
    argv = sys.argv[1:]
    extra = []
    if "--" in argv:
        i = argv.index("--")
        argv, extra = argv[:i], argv[i + 1:]
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--from", dest="start", choices=STAGES, default="extract")
    ap.add_argument("--to", dest="stop", choices=STAGES, default="reclaim")
    ap.add_argument("--out", help="work dir (default disc_map/extracted/AUDIO or $AC1_AUDIO_DIR)")
    ap.add_argument("--force", action="store_true", help="redo decoded wavs, clips and transcripts")
    ap.add_argument("--skip-demoplay", action="store_true", help="extract: skip GG/STR/DEMOPLAY.STR")
    ap.add_argument("--threshold", type=float, help="speakers: clustering cosine-distance cut")
    ap.add_argument("--n-voices", type=int, help="speakers: force N voice clusters")
    ap.add_argument("--corpus", action="append", default=[], help="match: extra toc.json (repeatable)")
    ap.add_argument("--fdat-entries", help="match + refs: dumped FDAT.T entries dir (200.bin = sound sequences)")
    args = ap.parse_args(argv)

    sel = STAGES[STAGES.index(args.start):STAGES.index(args.stop) + 1]
    t0 = time.time()
    for st in sel:
        cmd = [sys.executable, os.path.join(HERE, SCRIPTS[st])]
        if args.out:
            cmd += ["--out", args.out]
        if args.force and st in ("extract", "split", "transcribe"):
            cmd.append("--force")
        if st == "extract" and args.skip_demoplay:
            cmd.append("--skip-demoplay")
        if st == "speakers":
            if args.threshold is not None:
                cmd += ["--threshold", str(args.threshold)]
            if args.n_voices:
                cmd += ["--n-voices", str(args.n_voices)]
        if st == "transcribe":
            cmd += extra
        if st == "match":
            for c in args.corpus:
                cmd += ["--corpus", c]
        if st in ("match", "refs") and args.fdat_entries:
            cmd += ["--fdat-entries", args.fdat_entries]
        print(f"\n=== [{st}] {' '.join(os.path.basename(c) if i < 2 else c for i, c in enumerate(cmd))}",
              file=sys.stderr, flush=True)
        ts = time.time()
        r = subprocess.run(cmd)
        if r.returncode:
            sys.exit(f"stage {st} failed (exit {r.returncode})")
        print(f"=== [{st}] done in {time.time() - ts:.0f} s", file=sys.stderr, flush=True)
    print(f"\nall stages done in {(time.time() - t0) / 60:.1f} min", file=sys.stderr)


if __name__ == "__main__":
    main()
