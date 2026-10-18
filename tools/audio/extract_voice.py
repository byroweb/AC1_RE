#!/usr/bin/env python3
"""
extract_voice.py — decode AC1's spoken-voice XA streams from your disc to WAV.

Stage 1 of the voice pipeline (docs/AUDIO_PIPELINE.md). Uses jPSXdec (the same
tool the repo's disc map comes from):
  1. build a jPSXdec index of the .bin (skipped if <out>/ac1.idx exists),
  2. pick the voice streams out of it:
       GG/BGM/TALK00..05.XA[0..15]   96 mono 37800 Hz XA channels, stride 16
                                     (in-mission radio chatter)
       GG/STR/*.STR[0.0]             the XA audio track of each cutscene video
                                     (stereo, stride 8; music + speech mixed)
     BGM*.XA / CPU*.XA are music and are skipped,
  3. decode each to 16-bit PCM WAV at the native rate (jPSXdec `-af wav`),
  4. check the decoded sample count against the sector arithmetic in
     ac1audio.XAStream and write <out>/channels.json describing every stream
     (id, sectors, stride, channel, rate, wav path, sample counts).

Everything lands under disc_map/extracted/AUDIO/ (gitignored). Nothing derived
from the disc is written into the repo.

Usage (Git Bash, from the repo root; JAVA_HOME -> JDK 21):
  tools/audio/.venv/Scripts/python tools/audio/extract_voice.py
  tools/audio/.venv/Scripts/python tools/audio/extract_voice.py --only TALK --force
Options: --bin, --jar, --out, --only TALK|STR, --skip-demoplay, --force
"""
import argparse
import os
import shutil
import sys
import tempfile
import wave

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402


def wav_info(path):
    with wave.open(path, "rb") as w:
        return {"channels": w.getnchannels(), "rate": w.getframerate(),
                "samples": w.getnframes(), "sampwidth": w.getsampwidth()}


def decode_item(java, jar, idx, item_id, dest_wav, tmp_root):
    """Run jPSXdec on one index item; it writes <tmp>/<path of item>.wav, which
    we move to dest_wav."""
    tmp = tempfile.mkdtemp(prefix="jpsx_", dir=tmp_root)
    try:
        # cwd=tmp so jPSXdec's save.log/debug00.log die with the temp dir
        A.run([java, "-jar", jar, "-x", os.path.abspath(idx), "-i", item_id, "-af", "wav", "-dir", tmp],
              cwd=tmp, stdout=subprocess_devnull(), stderr=subprocess_devnull())
        wavs = []
        for dp, _, fns in os.walk(tmp):
            wavs += [os.path.join(dp, f) for f in fns if f.lower().endswith(".wav")]
        if len(wavs) != 1:
            sys.exit(f"{item_id}: expected one wav from jPSXdec, got {wavs}")
        shutil.move(wavs[0], dest_wav)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def subprocess_devnull():
    import subprocess
    return subprocess.DEVNULL


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--bin", help="disc image (default: filemap source_bin / $AC1_BIN)")
    ap.add_argument("--jar", help="jpsxdec.jar (default: $JPSXDEC_JAR / ~/dev/jpsxdec/*/jpsxdec.jar)")
    ap.add_argument("--out", default=A.AUDIO_DIR, help=f"work dir (default {A.AUDIO_DIR})")
    ap.add_argument("--idx", help="jPSXdec index to use/create (default <out>/ac1.idx)")
    ap.add_argument("--only", choices=["TALK", "STR"], help="restrict to one source kind")
    ap.add_argument("--skip-demoplay", action="store_true",
                    help="skip GG/STR/DEMOPLAY.STR (attract-mode gameplay, music only)")
    ap.add_argument("--force", action="store_true", help="re-decode even if the wav exists")
    args = ap.parse_args()

    bin_path = A.disc_bin(args.bin)
    java = A.find_java()
    jar = A.find_jpsxdec(args.jar)
    out = A.ensure_dir(args.out)
    idx = args.idx or os.path.join(out, "ac1.idx")
    if not os.path.exists(idx):
        A.log(f"[extract] indexing {bin_path} -> {idx}")
        A.build_index(bin_path, idx, jar, java)
    else:
        A.log(f"[extract] using existing index {idx}")

    items = A.parse_idx(idx)
    talk = [it for it in items if A.is_talk_item(it)]
    strs = [it for it in items if A.is_str_audio_item(it)]
    if args.skip_demoplay:
        strs = [it for it in strs if "DEMOPLAY" not in it["id"]]
    sel = []
    if args.only != "STR":
        sel += talk
    if args.only != "TALK":
        sel += strs
    if not sel:
        sys.exit("no voice streams found in the index — is this the SLUS-01323 v1.1 disc?")
    A.log(f"[extract] {len(talk)} TALK channels, {len(strs)} STR audio tracks in index; decoding {len(sel)}")

    ch_dir = A.ensure_dir(os.path.join(out, "channels"))
    tmp_root = A.ensure_dir(os.path.join(out, "_tmp"))
    channels = []
    bad = 0
    total_s = 0.0
    for n, it in enumerate(sel, 1):
        st = A.XAStream.from_idx_item(it)
        name = A.stream_short_name(it["id"])
        wav = os.path.join(ch_dir, name + ".wav")
        if args.force or not os.path.exists(wav):
            A.log(f"[extract] ({n}/{len(sel)}) {it['id']} -> {os.path.relpath(wav, out)}")
            decode_item(java, jar, idx, it["id"], wav, tmp_root)
        info = wav_info(wav)
        ok = info["samples"] == st.total_samples and info["rate"] == st.rate \
            and info["channels"] == (2 if st.stereo else 1)
        if not ok:
            bad += 1
            A.log(f"  !! {name}: wav {info} but sector math says {st.total_samples} samples "
                  f"@ {st.rate} Hz x{2 if st.stereo else 1} — sample->sector mapping unreliable")
        total_s += info["samples"] / info["rate"]
        rec = st.to_dict()
        rec.update({
            "name": name,
            "kind": A.source_kind(it["id"]),
            "source_file": it["id"].split("[")[0],
            "wav": os.path.relpath(wav, out).replace("\\", "/"),
            "wav_samples": info["samples"],
            "wav_channels": info["channels"],
            "sample_count_matches": ok,
        })
        channels.append(rec)

    shutil.rmtree(tmp_root, ignore_errors=True)
    A.save_json(os.path.join(out, "channels.json"), {
        "disc": os.path.basename(bin_path),
        "index": os.path.relpath(idx, out).replace("\\", "/"),
        "jpsxdec": jar,
        "channels": channels,
    })
    by_kind = {}
    for c in channels:
        k = by_kind.setdefault(c["kind"], [0, 0.0])
        k[0] += 1
        k[1] += c["wav_samples"] / c["rate"]
    A.log(f"[extract] wrote {len(channels)} streams, {total_s/60:.1f} min of audio total; "
          f"{bad} sample-count mismatches")
    for k, (cnt, secs) in sorted(by_kind.items()):
        A.log(f"           {k}: {cnt} streams, {secs/60:.1f} min")
    A.log(f"[extract] -> {os.path.join(out, 'channels.json')}")


if __name__ == "__main__":
    main()
