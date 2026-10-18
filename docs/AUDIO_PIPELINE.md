# Voice-line pipeline — `tools/audio/`

Target: **SLUS-01323 (v1.1)**. Extracts every spoken line on the disc, transcribes
it, groups the clips by speaker, lines each transcript up with the mission-text
corpus and writes a manifest the owner verifies **by listening instead of typing**.
Companion to [`AC1_TEXT_SYSTEM.md`](AC1_TEXT_SYSTEM.md) / [`MIS_FORMAT.md`](MIS_FORMAT.md)
(the text side of a localization) — this is the audio side, and §6 is the dub:
a replacement take goes back into its channel on a copy of the disc.

> No game audio is ever committed. Decoded WAVs, clips, model caches and the
> generated review page live under `disc_map/extracted/AUDIO/` (gitignored). The
> committed artefact is `disc_map/voice_manifest.tsv`: transcripts + metadata.

## 1. Where the speech is (CONFIRMED)

From the jPSXdec index of the disc (`disc_map/disc_files.json` carries the same
sector ranges):

| File | Sectors (disc) | Format | Content |
| --- | --- | --- | --- |
| `GG/BGM/TALK00.XA` … `TALK05.XA` | 60284–72187 | Mode 2 Form 2 XA-ADPCM, **16 interleaved mono channels** per file, 37800 Hz, 4-bit, **sector stride 16** | in-mission radio chatter: 96 channel streams, 7.4 min |
| `GG/STR/ACOPA.STR`, `ACOPB.STR` | 157932–176401 | STR video + interleaved **stereo** XA track, 37800 Hz, stride 8 | opening movie (music, no speech found) |
| `GG/STR/ACED1.STR` … `ACED5.STR` | 131042–157931 | same | endings — speech over music |
| `GG/STR/STAFF.STR`, `DEMOPLAY.STR` | 176402–209137 | same | credits / attract demo (music only) |
| `GG/BGM/BGM00/01.XA`, `CPU00/01.XA` | 124–45869 | XA | music — skipped |
| `GG/COM/MENU_VAB.T` | — | VAB (SPU ADPCM) | sound effects — out of scope |

A TALK channel is one XA stream: audio sector *k* of channel *c* sits at disc sector
`first(c) + 16·k`; each mono 4-bit sector decodes to **4032 PCM samples** (stereo:
2016 frames). jPSXdec's decoded sample counts match this arithmetic for all 105
streams (checked by `extract_voice.py`), which is what makes a clip's `start/end
sample` → `disc sector range` mapping exact. Most TALK channels hold a single line
(~4.5 s); a few hold two or three separated by silence, and every channel is
zero-padded to the file's end.

The committed `disc_files.json` was built from an older jPSXdec index that does not
list the STR audio tracks as separate items; the v2.1 index does
(`GG/STR/ACED1.STR[0.0]` etc.), so the pipeline enumerates streams from its own
fresh index rather than from the JSON.

## 2. Setup (once)

1. **JDK 21** — `JAVA_HOME` (Temurin at `C:/Users/<you>/dev/jdk/...` on the Windows RE
   box). jPSXdec v2.x runs on it.
2. **jPSXdec v2.1** — see [`tools/README.md`](../tools/README.md#jpsxdec-disc-indexer--xa--str-decoder):
   download `jpsxdec_v2.1-beta.zip` from the official release page, check its
   sha256 against the release's digest, unpack to `~/dev/jpsxdec/`. Set
   `JPSXDEC_JAR` if it lives elsewhere.
3. **Python venv** — `tools/audio/setup_audio.sh` creates `tools/audio/.venv` and
   installs `tools/audio/requirements.txt` (faster-whisper/CTranslate2, torch CPU
   wheels, SpeechBrain, rapidfuzz, numpy/scipy/soundfile). `--check` prints a health
   report. ffmpeg is **not** needed: jPSXdec decodes, numpy resamples.
4. `AC1_BIN` → your pristine `.bin` (2352-byte sectors).
5. The text corpus for matching: `python3 tools/extract/extract_t.py --file GG/MS/MIS.T --dump-entries`
   (writes `disc_map/extracted/MIS_T/entries/`).

Model weights download on first use: Whisper `large-v3` (~3 GB, Hugging Face cache)
and the ECAPA speaker model (~80 MB, into `disc_map/extracted/AUDIO/_models/`).

## 3. Run

```bash
tools/audio/.venv/Scripts/python tools/audio/run_all.py            # everything, ~25 min CPU
tools/audio/.venv/Scripts/python tools/audio/run_all.py --from speakers --threshold 0.65
tools/audio/.venv/Scripts/python tools/audio/run_all.py -- --model distil-large-v3   # faster Whisper
tools/audio/.venv/Scripts/python tools/audio/audio_test.py          # unit tests (no disc needed)
tools/audio/.venv/Scripts/python tools/audio/reinject_test.py       # XA codec / EDC / sector tests
```

Every stage resumes from what is on disk; `--force` redoes decode/split/transcribe.
On this PC (24 threads, no CUDA) `large-v3` int8 runs at roughly real time on the
133 clips; extraction and splitting take about a minute.

| Stage | Script | Reads → writes (under `disc_map/extracted/AUDIO/`) |
| --- | --- | --- |
| 1 extract | `extract_voice.py` | disc → `ac1.idx`, `channels/*.wav`, `channels.json` |
| 2 split | `split_utterances.py` | channels → `clips/*.wav`, `clips.json` |
| 3 transcribe | `transcribe.py` | clips → `transcripts.json` |
| 4 speakers | `speakers.py` | clips → `speakers.json`, `speakers_emb.npz` |
| 5 match | `match_text.py` | transcripts + `disc_map/extracted/MIS_T` → `matches.json` |
| 6 manifest | `build_manifest.py` | all of the above → **`disc_map/voice_manifest.tsv`**, `review.html` |
| 7 dub | `reinject_xa.py` | a take WAV + a manifest row → the row's sectors of a **copy** of the disc (§6) |

### Stage notes
- **Split.** TALK channels use a numpy RMS gate (20 ms frames, −40 dBFS open,
  8 dB hysteresis, 350 ms close gap, 150 ms minimum, 100 ms padding; the gate
  rises with a hiss floor by at most 10 dB). STR tracks have music under the voice,
  so they use Silero VAD (bundled with faster-whisper) on a 16 kHz downmix. Both are
  deterministic; the energy gate is unit-tested in `audio_test.py`.
- **Transcribe.** faster-whisper `large-v3`, int8, beam 5, `condition_on_previous_text`
  off, word timestamps on, with an `initial_prompt` of AC1 vocabulary (Raven, Ravens'
  Nest, Chrome, Murakumo Millennium, Nine-Ball, Hustler One, Progtech, AC, MT, Isaac
  City, mission names). Per clip: text, words, `avg_logprob`, `no_speech_prob`,
  compression ratio. Edit `AC1_PROMPT` in `transcribe.py` or pass `--prompt-file`.
- **Speakers.** ECAPA-TDNN embeddings (`speechbrain/spkrec-ecapa-voxceleb`),
  average-linkage agglomerative clustering on cosine distance, cut at `--threshold`
  0.72 (or `--n-voices N`; on these radio-filtered 1–6 s clips the pairwise distances
  are large — median 0.91 — and 0.7–0.75 yields 15–18 voices with few singletons). Clusters are `voice_00`, `voice_01`, … largest first; the
  summary gives clip count, total seconds, median F0, "processed" fraction and three
  example clips per voice so each voice can be named once. Also per clip: median F0
  (autocorrelation tracker), **harmonicity** and **spectral flatness**, from which the
  `processed` flag (robotic/vocoded voice) is derived (`harmonicity < 0.40` or
  `flatness > 0.2`; the TALK lines' median harmonicity is 0.51) — a heuristic,
  calibrate against the review page. Clips whose embeddings are within 0.03 of an
  earlier clip of the same length are tagged `duplicate_of` (the same recording
  reused in another TALK file) and noted in the manifest.
  `--recluster` re-cuts without re-embedding.
- **Match.** rapidfuzz `token_sort_ratio` of the normalised transcript against every
  draw_string message of `GG/MS/MIS.T` (split exactly as `scan_text.py` does, plus
  per-sentence candidates). Ids follow `disc_map/text_inventory.csv`:
  `MIS_T:<entry>:+0x<off>[#sN]`. Top-3 candidates are kept for the review page.
  Add corpora with `--corpus <toc.json>` or `--all-extracted`.

## 4. The manifest (`disc_map/voice_manifest.tsv`)

One row per clip, tab-separated:

| column | meaning |
| --- | --- |
| `clip_id` | `<stream>_cNNN`, e.g. `TALK03_ch12_c000`, `ACED4_str_c005` |
| `source_file`, `stream`, `channel`, `clip` | disc file, stream short name, XA channel number, clip index within the stream |
| `sector_first`, `sector_last` | inclusive **disc** sectors the clip was decoded from (on the stride grid) |
| `audio_sector_first/last`, `stride` | the same as 0-based audio-sector indices within the stream, and the interleave stride |
| `start_sample`, `end_sample`, `rate` | PCM sample range within the decoded stream (37800 Hz) |
| `duration_s` | clip length |
| `voice`, `voice_label` | cluster id, and the owner's name for it (filled once, carried across runs) |
| `f0_hz`, `processed` | median pitch; 1 = processed/robotic heuristic fired |
| `transcript`, `confidence`, `avg_logprob`, `no_speech_prob` | Whisper output; `confidence = exp(avg_logprob)·(1−no_speech_prob)` |
| `matched_string_id`, `matched_text`, `similarity` | best corpus string (0–100) |
| `status` | `empty` / `check` / `matched` (≥85) / `verified` (owner) |
| `auto_notes` | pipeline flags: processed voice, same recording as …, cutscene track, low confidence, stock Whisper hallucination phrase |
| `verified_text`, `verified`, `notes` | owner columns — survive re-runs (carried over by `clip_id`); `notes` is yours, never auto-filled |

**Workflow:** open `disc_map/extracted/AUDIO/review.html` (local file, never publish —
it plays game audio). Rows are sorted lowest confidence first; play, fix the text or
press *accept* on a corpus match, tick *verified*, name voices in the voice box.
Edits persist in the browser; **Export TSV** downloads the merged manifest — copy it
over `disc_map/voice_manifest.tsv` and commit.

## 5. Caveats

- Whisper hallucinates on 1-second clips and on music; `no_speech_prob` and the
  `empty` status catch most of it, but every `check` row needs a listen.
- The in-mission radio lines have **no on-screen text** in AC1, so most TALK rows will
  not match the corpus — that is expected; matching exists for the lines that are
  catalogued (mission names, cutscene text) and to catch near-duplicates.
- Silero VAD found no speech in `ACOPA`/`ACOPB`/`STAFF`/`DEMOPLAY` (music only).
  `--method energy` or lowering the VAD threshold in `split_by_vad` would force clips
  out of them if a line is suspected.
- Clustering threshold: short, radio-filtered lines make embeddings noisy. If two
  speakers merge, lower `--threshold` (or set `--n-voices`); if one speaker splits,
  raise it. Name the clusters only after the cut looks right.
- `disc_files.json` has no STR audio children; regenerate it from a v2.1 index
  (`tools/extract/build_filemap.py`) if the map should list them.
- Everything runs on CPU. The Intel Arc GPU is unused (no CUDA; CTranslate2 has no
  oneAPI build).

## 6. Reinjection — putting a dub back (CONFIRMED on a copy of the disc, plays in DuckStation)

The game addresses a line as *(file, channel, sector)*, so a dubbed line must
occupy **the same channel and no more audio sectors** than the original: for each
manifest row, `audio_sector_first..audio_sector_last` is the budget
(`n·4032/37800` s of mono audio — 0.853 s for an 8-sector line). Longer takes
must be shortened (or `--trim`med). `tools/audio/reinject_xa.py` does the whole
job from a manifest row:

```bash
tools/audio/.venv/Scripts/python tools/audio/reinject_xa.py \
    --clip TALK05_ch00_c000 --wav take.wav --image ~/dev/ac1_dub.bin --verify-jpsxdec
tools/audio/.venv/Scripts/python tools/audio/reinject_xa.py --clip ... --wav ... --image ... --dry-run
tools/audio/.venv/Scripts/python tools/audio/reinject_test.py      # codec / EDC / packing unit tests
```

`--image` is patched **in place** and must be a copy: the script refuses `$AC1_BIN`
(and the filemap's `source_bin`), and creates the copy from the pristine image
(+ a `.cue`) when it does not exist yet, so repeating the command dubs several
lines into one image. Only the TALK rows are accepted (STR rows are stereo
music+speech mixes, see below).

What it does, per row (all of it verified, see the round trip below):

1. **Take in.** Any WAV; stereo is downmixed, other rates are resampled to
   37800 Hz (scipy), `--gain-db` applied, then int16. A take longer than the
   budget is refused with the overshoot in seconds unless `--trim`. The take is
   padded with silence to the **whole** budget so no tail of the old line survives.
2. **Encode.** `tools/audio/xa_adpcm.py` is a pure-Python/numpy 4-bit XA-ADPCM
   codec. Per 28-sample sound unit it tries all 52 (filter 0–3 × shift 0–12)
   parameter pairs with exact decoder feedback and keeps the least-squares one;
   groups are packed as 16 parameter bytes (two copies each) + 112 nibble bytes,
   18 groups = 2304 bytes = 4032 samples per sector. ~45 ms per sector. The
   decoder's two-sample history runs across units *and sectors*, so before
   encoding a clip that does not start the stream the script decodes the
   channel's preceding sectors from the image to seed it (`TALK03_ch09_c002`:
   history `(63, 66)` after 58 sectors).
3. **Write Form 2 sectors.** For each of the budget's disc sectors
   `sector_first + 16·k` the original 24 bytes (sync, header, 8-byte subheader —
   file 1, channel, submode, coding `0x00` = mono/37800/4-bit) are kept, 2304
   ADPCM bytes + 20 zero bytes written, and the 4-byte EDC recomputed. Each
   target sector is checked first (sync, mode 2, channel = manifest channel,
   submode Form 2|audio, coding 0x00, original EDC valid). The 15 other channels'
   sectors in between are never touched.
4. **Read back** every written sector, verify its EDC, decode it and report the
   SNR against the padded take; `--xa-out` dumps the raw sectors; `--verify-jpsxdec`
   decodes the whole stream from the patched image and from the pristine one with
   jPSXdec (reusing `ac1.idx`, the layout is unchanged) and checks: same sample
   count, everything before the clip bit-identical, re-convergence after the clip
   within one sector, the stream equal to `xa_adpcm`'s model of the sectors on
   disc, and the clip's SNR.

### Disc facts (CONFIRMED on SLUS-01323)
- TALK sectors: subheader `01 cc 64 00` (file 1, channel, RT|Form2|Audio, mono
  37800 Hz 4-bit); the stream's last sector has submode **`0xE4`** = EOF set, EOR
  *not* set. The 20 bytes after the sound area are zero. Copying the original
  subheader keeps all of this.
- EDC: the CD-ROM EDC (reflected CRC-32, polynomial `0xD8018001`, no init/xor)
  over bytes 16..2347 (subheader + 2324 user bytes), little-endian at 2348 —
  byte-exact against the disc's TALK sectors, including the EOF ones.
- Decoding: emulators use the integer rule `s = (nibble << (12−shift)) +
  ((p1·K0 + p2·K1 + 32) >> 6)`, K0 = {0, 60, 115, 98}, K1 = {0, 0, −52, −55}
  (psx-spx). **jPSXdec v2.1 keeps the history as unrounded doubles** and rounds only
  the output; the two drift apart by up to ~14 LSB on loud speech (65 dB down).
  `xa_adpcm.decode_sectors_float` reproduces jPSXdec within 1 LSB on ~0.01 % of
  samples (bit-exact on short streams); `decode_sectors` is the integer rule.
- jPSXdec v2.1's command line has **no usable replace command** for us: the XA
  item help lists only `-af`/`-vol`, and `-replacexa`/`-xa` only answers
  "-replacexa option is missing -xa option" (the GUI may still offer it). The
  writer above is therefore self-contained; jPSXdec is only the independent
  decoder for verification.

### Round trip (what "verified" means)
`extract_voice.py` → clip WAV → `reinject_xa.py` into a copy → `extract_voice.py`
on the copy, on this disc:

| test | result |
| --- | --- |
| `TALK05_ch00_c000` (8 sectors, its own clip WAV re-encoded) | self-check 65.3 dB SNR (integer decoder), jPSXdec decode of the patched copy 70.3 dB vs the take, max error 25 LSB |
| `extract_voice.py --only TALK` on the patched copy | 96 streams, 0 sample-count mismatches; 95 channel WAVs byte-identical to the pristine decode, only `TALK05_ch00` differs (same 32256 samples) |
| `TALK03_ch09_c002` (audio sectors 58..64, mid-stream) | 66 dB SNR; 233 856 samples before the clip bit-identical; after it the decode re-converges with the pristine one within 73 samples (1.9 ms) — the ADPCM history transient as the following original sectors start from our last two samples; stable filters make it die out |
| 1.2 s / 44.1 kHz / stereo synthetic take into a 0.853 s budget | refused ("0.347 s longer"); with `--trim`: downmix + resample + cut, 55 dB SNR |
| pristine image as `--image` | refused; `--dry-run` reads it without writing |

### In-game playback (CONFIRMED in DuckStation, 2026-10-18)

The game plays a line written by `reinject_xa.py`. Test: `TALK00_ch12_c000`
("This is the guard. We've cornered the enemy...", 42 sectors = 4.48 s) was
replaced in a copy of the disc by an obviously different take — a 0.4 s 300→3000 Hz
chirp, 0.1 s silence, then the `TALK01_ch13_c000` line ("I think we've been
noticed...", another speaker) — with `--verify-jpsxdec` (55 dB SNR). The line was
then triggered in the running game, on the pristine image and on the patched copy,
and DuckStation's own media capture (`capture` MCP tool, AAC 44.1 kHz) was decoded
and cross-correlated (normalised, after resampling to 37800 Hz) against the take and
against all 96 TALK channel decodes:

| run | drive during the line | best match | NCC |
| --- | --- | --- | --- |
| pristine image | streams `TALK00.XA` (LBA 60442 → 62596 at 2×) | `TALK00_ch12` | **0.963** |
| patched copy | same sectors, same timing | the take (`take_ch12.wav`) | **0.950** |
| patched copy | | `TALK01_ch13` (the line inside the take) | 0.947 |
| patched copy | | `TALK00_ch12` (the original line) | < 0.13 (noise floor, like every other channel) |

Per-100 ms windows of the patched run correlate 0.76–1.00 with the take over its
whole 4.34 s (the chirp windows at 1.00), no window is silent, and the game's radio
state bytes (`0x80041BBE..C3`, below) go through the same values as with the
original line, so the dub plays cleanly at the original level with no dropout or
glitch audible in the capture. Nothing else in the mission changed (the other
channels are byte-identical anyway).

**How the line was triggered.** Missions 30 and 31 (the two offered on a new game)
play no radio line in their first 90 s — their chunk-4 scripts have no `play_sound`
— so the game's own radio player was called directly from the debugger:

- `FUN_8004DB18(u16 id)` is the XA line player (`id == 0xFFFF` stops). `id >> 4`
  indexes a 36-byte-per-entry XA file table at `0x80031764` (entries **3..8 =
  `TALK00..05`**, 0/1 = `BGM00/01`, 9..20 = the `CPUxx` arena tracks; `CdlLOC` of the
  file at `+32`, one u16 per channel at `+2·ch`, `0xE000 | n` for TALK channels),
  `id & 15` is the XA channel. It stores the id at `0x80041BBE`, builds the
  `CdlFILTER {file=1, chan}` on the stack, issues `CdControl(CdlSetfilter)` then
  `CdControl(CdlSetloc)` with the file's `CdlLOC`, and streams. The mission BGM goes
  through the same function (id `0x16` = `BGM01` channel 6 in mission 30), which is
  why the music stops while a line plays. The sound-sequence interpreter
  (`0x800532A0`, called from the script VM's `play_sound`) has an opcode that calls it
  with `operand + 0x30`, i.e. TALK lines are numbered 0..95 in `TALKnn` channel order.
- Procedure (sadnescity DuckStation MCP fork, port 2346): boot the copy's `.cue`,
  load `M30-ctrl-01` — **a state re-points DuckStation at the media path stored in
  its header**, so a copy of the `.sav` with the path at `offset_to_media_path`
  (`+0xac`/`+0xa8`) rewritten to the copy's `.cue` was staged in a scratch slot —
  put an execute breakpoint on a per-frame game function entry (`0x8005804C`),
  on the hit save all GPRs, set `a0 = 0x3C`, `ra = hook`, `pc = 0x8004DB18`,
  continue until the hook fires again (≈ the line's `CdControl` sync), restore
  every GPR, remove the breakpoint, start the capture and continue. After the
  line the drive keeps reading `TALK00.XA` to its end and the radio state byte
  `0x80041BC2` goes `0xFF → 0xF0`, on both images.

Not done: hardware (a burned disc / ODE) and a line triggered by a mission script
rather than the debugger — the latter only needs a save state in a mission whose
script has `play_sound` (missions 1, 3, 4, 6, 9, 10, 12, 13, 14, 17, 18, 21, 26,
27, 33, 40, 46, 49), none of which is reachable from the archived states yet.

For the STR endings, the speech sits mixed into the stereo music track; a dub there
means re-mixing the whole track (music stem is not available separately) and
re-encoding the stride-8 stereo XA — the codec here is mono-only (stereo groups
interleave L/R units), so that is a remix job plus a small encoder extension, not a
line swap.
