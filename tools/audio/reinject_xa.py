#!/usr/bin/env python3
"""
reinject_xa.py - write a replacement take (WAV) into one TALK channel of a COPY
of the disc, driven by a row of disc_map/voice_manifest.tsv.

The dub half of the voice pipeline (docs/AUDIO_PIPELINE.md §6). Given a clip id
the manifest tells us the stream, its XA channel, the interleave stride and the
clip's audio-sector budget (audio_sector_first..audio_sector_last, i.e. disc
sectors sector_first + stride*k). This script:

  1. loads the WAV, downmixes to mono, resamples to 37800 Hz if needed, applies
     an optional gain, and refuses a take longer than the budget
     (budget = (n_sectors * 4032) / 37800 s; --trim cuts it instead);
  2. pads the take with silence to the WHOLE budget (so the tail of the old line
     never survives), seeds the ADPCM history by decoding the channel's sectors
     that precede the clip (the decoder's two-sample history runs across
     sectors), and encodes with tools/audio/xa_adpcm.py (4-bit XA-ADPCM);
  3. rewrites exactly the budget's sectors of --image as Mode 2 Form 2: the
     original sync/header/subheader are kept byte for byte (so the stream's
     EOF/EOR bits on its last sector survive), 2304 ADPCM bytes + 20 zero bytes
     are written and the 4-byte EDC is recomputed. The other 15 channels'
     sectors in between are never touched;
  4. reads the sectors back, checks the EDC and decodes them, and reports the
     SNR of what the game will hear against the padded take. --verify-jpsxdec
     additionally decodes the whole stream from the patched image AND from the
     pristine one with jPSXdec: the sample count must be unchanged, everything
     before the clip bit-identical, the decode after the clip must re-converge
     within one sector (a few-ms ADPCM history transient is inherent), and the
     whole stream must equal what xa_adpcm predicts for the sectors on disc
     (jPSXdec's double-precision decode is modelled).

--image is the file that gets written. It must NOT be your pristine disc (the
script refuses $AC1_BIN / the filemap's source_bin); if it does not exist yet
it is copied from --copy-from (default: the pristine image) first, so several
lines can be dubbed into the same copy by repeating the command.

Usage (Git Bash, repo root; numpy needed, scipy only for resampling):
  tools/audio/.venv/Scripts/python tools/audio/reinject_xa.py \
      --clip TALK05_ch00_c000 --wav take.wav --image /tmp/ac1_dub.bin
  ... --dry-run                 # encode + report, write nothing
  ... --trim                    # cut a long take to the budget instead of refusing
  ... --verify-jpsxdec          # decode the patched stream with jPSXdec and compare
Options: --manifest, --copy-from, --gain-db, --xa-out (dump the raw sectors written),
         --jar/--idx (for --verify-jpsxdec).
"""
import argparse
import csv
import os
import shutil
import subprocess
import sys
import tempfile
import wave

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
import xa_adpcm as X  # noqa: E402

MANIFEST = os.path.join(A.ROOT, "disc_map", "voice_manifest.tsv")


# ------------------------------------------------------------------ manifest
def load_row(manifest, clip_id):
    with open(manifest, encoding="utf-8", newline="") as f:
        for r in csv.DictReader(f, delimiter="\t"):
            if r["clip_id"] == clip_id:
                return r
    sys.exit(f"clip {clip_id!r} not in {manifest}")


class Target:
    """Where a manifest row's audio lives on the disc."""

    def __init__(self, row):
        self.clip_id = row["clip_id"]
        self.source_file = row["source_file"]
        self.stream = row["stream"]
        self.channel = int(row["channel"])
        self.stride = int(row["stride"])
        self.sector_first = int(row["sector_first"])
        self.sector_last = int(row["sector_last"])
        self.audio_first = int(row["audio_sector_first"])
        self.audio_last = int(row["audio_sector_last"])
        self.rate = int(row.get("rate") or 37800)
        if ".STR[" in self.source_file or "_str" in self.stream:
            sys.exit(f"{self.clip_id}: STR audio tracks are stereo music+speech mixes; "
                     "this tool only dubs the mono TALK channels (see AUDIO_PIPELINE.md §6)")
        self.n_sectors = self.audio_last - self.audio_first + 1
        if self.n_sectors < 1:
            sys.exit(f"{self.clip_id}: empty sector budget")
        if self.sector_first + self.stride * (self.n_sectors - 1) != self.sector_last:
            sys.exit(f"{self.clip_id}: sector_first/last {self.sector_first}-{self.sector_last} "
                     f"do not agree with audio sectors {self.audio_first}-{self.audio_last} x stride {self.stride}")
        # first sector of the whole stream (audio sector 0)
        self.stream_first = self.sector_first - self.stride * self.audio_first

    @property
    def budget_samples(self):
        return self.n_sectors * X.SAMPLES_PER_SECTOR

    @property
    def budget_s(self):
        return self.budget_samples / self.rate

    def disc_sector(self, k):
        """Disc sector of audio sector k of the STREAM."""
        return self.stream_first + self.stride * k

    def target_sectors(self):
        return [self.disc_sector(k) for k in range(self.audio_first, self.audio_last + 1)]


# ------------------------------------------------------------------ audio in
def read_wav(path):
    """-> (float64 samples [-1,1] shape (n, ch), rate). soundfile if present, else wave (PCM 16/24/32)."""
    try:
        import soundfile as sf
        x, rate = sf.read(path, dtype="float64", always_2d=True)
        return x, rate
    except ImportError:
        pass
    with wave.open(path, "rb") as w:
        ch, sw, rate, n = w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()
        raw = w.readframes(n)
    if sw == 2:
        x = np.frombuffer(raw, np.int16).astype(np.float64) / 32768.0
    elif sw == 1:
        x = (np.frombuffer(raw, np.uint8).astype(np.float64) - 128.0) / 128.0
    elif sw in (3, 4):
        b = np.frombuffer(raw, np.uint8).reshape(-1, sw)
        v = np.zeros(len(b), np.int64)
        for i in range(sw):
            v |= b[:, i].astype(np.int64) << (8 * i)
        v = (v << (64 - 8 * sw)) >> (64 - 8 * sw)  # sign-extend
        x = v.astype(np.float64) / float(1 << (8 * sw - 1))
    else:
        sys.exit(f"{path}: unsupported sample width {sw}")
    return x.reshape(-1, ch), rate


def prepare_take(path, target_rate, gain_db=0.0):
    """Mono float -> int16 at target_rate. Returns (int16 array, notes list)."""
    x, rate = read_wav(path)
    notes = []
    if x.shape[1] > 1:
        notes.append(f"downmixed {x.shape[1]} channels to mono")
        x = x.mean(axis=1)
    else:
        x = x[:, 0]
    if rate != target_rate:
        try:
            from scipy.signal import resample_poly
        except ImportError:
            sys.exit(f"{path} is {rate} Hz but the channel is {target_rate} Hz; resampling needs scipy "
                     "(tools/audio/.venv has it) - or resample the take yourself")
        from math import gcd
        g = gcd(int(rate), int(target_rate))
        x = resample_poly(x, target_rate // g, rate // g)
        notes.append(f"resampled {rate} -> {target_rate} Hz")
    if gain_db:
        x = x * (10.0 ** (gain_db / 20.0))
        notes.append(f"gain {gain_db:+.1f} dB")
    peak = float(np.max(np.abs(x))) if len(x) else 0.0
    if peak > 1.0:
        notes.append(f"peak {20*np.log10(peak):+.1f} dBFS clipped to full scale")
    y = np.clip(np.round(x * 32767.0), -32768, 32767).astype(np.int16)
    return y, notes


# ------------------------------------------------------------------ disc io
def read_sector(f, sector):
    f.seek(sector * X.SECTOR_RAW)
    s = f.read(X.SECTOR_RAW)
    if len(s) != X.SECTOR_RAW:
        sys.exit(f"sector {sector} is past the end of the image")
    return s


def check_target_sector(sec, raw, t, k):
    """The sector we are about to overwrite must be this channel's Form 2 audio."""
    if raw[0:12] != b"\x00" + b"\xff" * 10 + b"\x00":
        sys.exit(f"sector {sec}: no sync pattern - is this a 2352-byte raw image?")
    if raw[15] != 2:
        sys.exit(f"sector {sec}: mode {raw[15]}, expected 2")
    fno, ch, sm, coding = X.subheader(raw)
    if ch != t.channel:
        sys.exit(f"sector {sec}: XA channel {ch} but the manifest says channel {t.channel} "
                 f"- manifest / image mismatch")
    if not (sm & X.SM_FORM2) or not (sm & X.SM_AUDIO):
        sys.exit(f"sector {sec}: submode {sm:#04x} is not Form 2 audio")
    if coding != 0x00:
        sys.exit(f"sector {sec}: coding {coding:#04x} is not mono/37800 Hz/4-bit (0x00); "
                 "the encoder only produces that")
    stored = int.from_bytes(raw[X.EDC_OFF:X.EDC_OFF + 4], "little")
    if stored != X.form2_edc(raw):
        A.log(f"  ! sector {sec}: original EDC {stored:08x} does not verify - image already patched or damaged?")
    if (sm & (X.SM_EOF | X.SM_EOR)) and k != t.audio_last:
        A.log(f"  ! sector {sec}: EOF/EOR set on audio sector {k} which is not the last of the budget")


def seed_history(f, t):
    """Decode the stream's audio sectors before the clip to get the decoder's
    (p1, p2) history at the clip's first sample. (0, 0) at the stream start."""
    p1 = p2 = 0
    for k in range(t.audio_first):
        raw = read_sector(f, t.disc_sector(k))
        _, p1, p2 = X.decode_sector_data(X.sector_sound_area(raw), p1, p2)
    return p1, p2


def pristine_paths():
    ps = []
    for p in (os.environ.get("AC1_BIN"),):
        if p:
            ps.append(p)
    if os.path.exists(A.FILEMAP):
        import json
        with open(A.FILEMAP) as fh:
            sb = json.load(fh).get("source_bin")
        if sb:
            ps.append(sb)
    return [os.path.normcase(os.path.abspath(p)) for p in ps]


def same_file(a, b):
    return os.path.normcase(os.path.abspath(a)) == os.path.normcase(os.path.abspath(b))


# ------------------------------------------------------------------ verify
def jpsxdec_decode_stream(image, item, idx, jar=None):
    """Decode one XA item of `image` with jPSXdec -> int16 array. `idx` is an
    index whose sector layout matches (the pristine disc's index does, since the
    patch moves nothing); None builds a fresh one (~6 s)."""
    java = A.find_java()
    jar = A.find_jpsxdec(jar)
    tmp = tempfile.mkdtemp(prefix="xa_verify_")
    try:
        if idx and os.path.exists(idx):
            base = [java, "-jar", jar, "-x", os.path.abspath(idx), "-f", os.path.abspath(image)]
        else:
            idx = os.path.join(tmp, "patched.idx")
            A.log("[verify] indexing with jPSXdec (~6 s)")
            A.run([java, "-jar", jar, "-f", os.path.abspath(image), "-x", idx], cwd=tmp,
                  stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            base = [java, "-jar", jar, "-x", idx]
        A.run(base + ["-i", item, "-af", "wav", "-dir", tmp], cwd=tmp,
              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        wavs = [os.path.join(dp, fn) for dp, _, fns in os.walk(tmp) for fn in fns if fn.lower().endswith(".wav")]
        if len(wavs) != 1:
            sys.exit(f"[verify] expected one wav from jPSXdec for {item}, got {wavs}")
        with wave.open(wavs[0], "rb") as w:
            if w.getnchannels() != 1 or w.getsampwidth() != 2:
                sys.exit(f"[verify] {item}: jPSXdec wav is not 16-bit mono")
            return np.frombuffer(w.readframes(w.getnframes()), np.int16)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


def verify_with_jpsxdec(image, pristine, t, expect, idx=None, jar=None):
    """Decode the whole stream from the patched `image` and from `pristine` with
    jPSXdec. Checks: same sample count; everything before the clip is
    bit-identical to the pristine decode; after the clip the decode re-converges
    with the pristine one within one sector (the original sectors that follow
    now start from OUR last two samples - a transient of a few ms that dies out
    because the ADPCM filters are stable); the whole stream equals what
    xa_adpcm.decode_sectors_float predicts from the sectors on disc (bit-exact
    model of jPSXdec); and reports the clip's SNR against `expect` (the padded take)."""
    item = f"{t.source_file}[{t.channel}]"
    got = jpsxdec_decode_stream(image, item, idx, jar)
    ref = jpsxdec_decode_stream(pristine, item, idx, jar)
    a = t.audio_first * X.SAMPLES_PER_SECTOR
    b = (t.audio_last + 1) * X.SAMPLES_PER_SECTOR
    ok = True
    if len(got) != len(ref):
        A.log(f"[verify] !! sample count changed: pristine {len(ref)}, patched {len(got)}")
        return False
    if len(got) < b:
        A.log(f"[verify] !! jPSXdec decoded only {len(got)} samples, clip range ends at {b}")
        return False
    A.log(f"[verify] jPSXdec decodes {t.stream} from the patched image to {len(got)} samples, "
          f"same as the pristine one")
    before = np.array_equal(got[:a], ref[:a])
    A.log(f"[verify] {a} samples before the clip are {'bit-identical' if before else '!! DIFFERENT'} "
          "to the pristine decode")
    ok &= before
    tail = np.nonzero(got[b:] != ref[b:])[0]
    if len(got) > b:
        if len(tail) == 0:
            A.log(f"[verify] {len(got) - b} samples after the clip are bit-identical to the pristine decode")
        else:
            n_tail = int(tail.max()) + 1
            mx = int(np.max(np.abs(got[b:].astype(np.int64) - ref[b:].astype(np.int64))))
            A.log(f"[verify] after the clip the decode re-converges with the pristine one within "
                  f"{n_tail} samples ({1000.0 * n_tail / t.rate:.1f} ms, {len(tail)} samples differ, "
                  f"max |diff| {mx}) - the expected ADPCM history transient; identical from there on")
            ok &= n_tail <= X.SAMPLES_PER_SECTOR
    # our model of the sectors vs jPSXdec, over the whole stream
    with open(image, "rb") as f:
        datas = [X.sector_sound_area(read_sector(f, t.disc_sector(k)))
                 for k in range(len(got) // X.SAMPLES_PER_SECTOR)]
    model, _, _ = X.decode_sectors_float(datas)
    md = np.abs(model.astype(np.int64) - got.astype(np.int64))
    mx = int(md.max()) if len(md) else 0
    if mx == 0:
        A.log("[verify] jPSXdec output matches decode_sectors_float of the sectors on disc bit for bit")
    else:
        A.log(f"[verify] jPSXdec output differs from decode_sectors_float of the sectors on disc by up to "
              f"{mx} LSB on {int(np.count_nonzero(md))} samples")
    ok &= mx <= 1
    A.log(f"[verify] clip range: SNR {X.snr_db(expect, got[a:b]):.1f} dB vs the take "
          f"(max |err| {int(np.max(np.abs(got[a:b].astype(np.int64) - expect.astype(np.int64))))})")
    return ok


# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--clip", required=True, help="clip_id from the manifest, e.g. TALK05_ch00_c000")
    ap.add_argument("--wav", required=True, help="the replacement take (any WAV; mono 37800 Hz preferred)")
    ap.add_argument("--image", required=True, help="disc image to patch IN PLACE (a copy; never the pristine)")
    ap.add_argument("--copy-from", help="source image to copy to --image if it does not exist "
                                        "(default: filemap source_bin / $AC1_BIN)")
    ap.add_argument("--manifest", default=MANIFEST, help=f"default {MANIFEST}")
    ap.add_argument("--gain-db", type=float, default=0.0, help="gain applied to the take before encoding")
    ap.add_argument("--trim", action="store_true", help="cut a take that exceeds the sector budget")
    ap.add_argument("--dry-run", action="store_true", help="encode and report but write nothing")
    ap.add_argument("--xa-out", help="also write the raw 2352-byte sectors produced, in stream order")
    ap.add_argument("--verify-jpsxdec", action="store_true",
                    help="decode the patched stream with jPSXdec and report the SNR against the take")
    ap.add_argument("--idx", help="existing jPSXdec index of the pristine disc (sector layout is the same), "
                                  "for --verify-jpsxdec; default <AUDIO_DIR>/ac1.idx if present")
    ap.add_argument("--jar", help="jpsxdec.jar for --verify-jpsxdec")
    args = ap.parse_args()

    t = Target(load_row(args.manifest, args.clip))
    A.log(f"[reinject] {t.clip_id}: {t.source_file} channel {t.channel}, audio sectors "
          f"{t.audio_first}..{t.audio_last} ({t.n_sectors} sectors = {t.budget_s:.3f} s) "
          f"at disc sectors {t.sector_first}..{t.sector_last} step {t.stride}")

    take, notes = prepare_take(args.wav, t.rate, args.gain_db)
    for n in notes:
        A.log(f"  - {n}")
    need = -(-len(take) // X.SAMPLES_PER_SECTOR)
    A.log(f"  take: {len(take)} samples = {len(take)/t.rate:.3f} s -> {need} sectors")
    if need > t.n_sectors:
        over = len(take) - t.budget_samples
        if not args.trim:
            sys.exit(f"take is {over/t.rate:.3f} s ({over} samples) longer than the clip's budget of "
                     f"{t.budget_s:.3f} s ({t.n_sectors} sectors); shorten it or pass --trim")
        A.log(f"  - trimmed {over/t.rate:.3f} s off the end (--trim)")
        take = take[:t.budget_samples]
    padded = X.pad_to_sectors(take, t.n_sectors)

    # --- the image
    if not args.dry_run:
        for p in pristine_paths():
            if same_file(args.image, p):
                sys.exit(f"refusing to write the pristine image {args.image}; give --image a copy")
        if not os.path.exists(args.image):
            src = args.copy_from or A.disc_bin()
            A.log(f"[reinject] copying {src} -> {args.image}")
            shutil.copyfile(src, args.image)
            cue = os.path.splitext(args.image)[0] + ".cue"
            if not os.path.exists(cue):
                with open(cue, "w") as c:
                    c.write(f'FILE "{os.path.basename(args.image)}" BINARY\n'
                            f'  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
        image = args.image
        mode = "r+b"
    else:
        image = args.copy_from or (args.image if os.path.exists(args.image) else A.disc_bin())
        mode = "rb"

    with open(image, mode) as f:
        sectors = t.target_sectors()
        originals = []
        for k, sec in zip(range(t.audio_first, t.audio_last + 1), sectors):
            raw = read_sector(f, sec)
            check_target_sector(sec, raw, t, k)
            originals.append(raw)
        p1, p2 = seed_history(f, t)
        A.log(f"  decoder history at clip start: ({p1}, {p2}) from {t.audio_first} preceding sector(s)")

        def prog(i, n):
            if i == n or i % 10 == 0:
                A.log(f"  encoding sector {i}/{n}")
        datas, _, _, sq = X.encode_stream(padded, p1, p2, progress=prog)
        new_sectors = [X.build_form2_sector(o, d) for o, d in zip(originals, datas)]
        dec, _, _ = X.decode_sectors(datas, p1, p2)
        A.log(f"  encoder self-check: SNR {X.snr_db(padded, dec):.1f} dB over {len(padded)} samples "
              f"(max |err| {int(np.max(np.abs(dec.astype(np.int64) - padded.astype(np.int64))))})")
        changed = sum(1 for o, n in zip(originals, new_sectors) if o != n)

        if args.xa_out:
            with open(args.xa_out, "wb") as xo:
                for s in new_sectors:
                    xo.write(s)
            A.log(f"  wrote {len(new_sectors)} raw sectors to {args.xa_out}")

        if args.dry_run:
            A.log(f"[reinject] dry run: would rewrite {len(new_sectors)} sectors "
                  f"({changed} differ from the image) of {image}")
            return

        for sec, s in zip(sectors, new_sectors):
            f.seek(sec * X.SECTOR_RAW)
            f.write(s)
        f.flush()
        # read back
        back = [read_sector(f, sec) for sec in sectors]
        for sec, s, b in zip(sectors, new_sectors, back):
            if b != s:
                sys.exit(f"sector {sec}: read-back differs from what was written")
            if int.from_bytes(b[X.EDC_OFF:X.EDC_OFF + 4], "little") != X.form2_edc(b):
                sys.exit(f"sector {sec}: EDC does not verify after write")
        rb, _, _ = X.decode_sectors([X.sector_sound_area(b) for b in back], p1, p2)
    A.log(f"[reinject] wrote {len(new_sectors)} sectors of {t.source_file} channel {t.channel} "
          f"into {args.image}; read-back decodes to {len(rb)} samples, SNR {X.snr_db(padded, rb):.1f} dB vs the take")

    if args.verify_jpsxdec:
        idx = args.idx or os.path.join(A.AUDIO_DIR, "ac1.idx")
        pristine = args.copy_from or A.disc_bin()
        if not verify_with_jpsxdec(args.image, pristine, t, padded, idx if os.path.exists(idx) else None, args.jar):
            sys.exit("[verify] FAILED")


if __name__ == "__main__":
    main()
