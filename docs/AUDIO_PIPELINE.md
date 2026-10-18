# Voice-line pipeline — `tools/audio/`

Target: **SLUS-01323 (v1.1)**. Extracts every spoken line on the disc, transcribes
it, groups the clips by speaker, lines each transcript up with the mission-text
corpus and writes a manifest the owner verifies **by listening instead of typing**.
Companion to [`AC1_TEXT_SYSTEM.md`](AC1_TEXT_SYSTEM.md) / [`MIS_FORMAT.md`](MIS_FORMAT.md)
(the text side of a localization) — this is the audio side, and the groundwork for a
dub.

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

## 6. Reinjection — how a dub would go back (HYPOTHESIS, not yet exercised)

The game addresses a line as *(file, channel, start sector)*, so a dubbed line must
occupy **the same channel and no more audio sectors** than the original: for each
manifest row, `audio_sector_first..audio_sector_last` is the budget
(`(n+1)·4032/37800` s of mono audio). Longer takes must be trimmed or sped up.

1. Produce the new take as 37800 Hz mono 16-bit WAV, length ≤ the sector budget
   (pad with silence to a whole number of sectors).
2. Encode to XA-ADPCM with the **same parameters**: 37800 Hz, mono, 4-bit, file
   number 1, channel = the stream's channel, stride 16 (i.e. one sector of this
   channel every 16 disc sectors). Candidate encoders: **psxavenc** (`-t xa`, set
   rate/bits/channel). jPSXdec can then write the encoded sectors over the item in
   place, keeping subheaders and EDC: `java -jar jpsxdec.jar -x ac1.idx -i
   "GG/BGM/TALK03.XA[12]" -replacexa -xa <encoded.xa>` (v2.1; it refuses a
   replacement that does not fit — see the manual's *Replace XA* section).
3. Write each encoded sector over disc sector `sector_first + 16·k` of the original
   image, **Mode 2 Form 2** layout: 12 sync + 4 header + 8 subheader (file, channel,
   submode `0x64` = real-time|form2|audio, coding `0x00` mono/37800/4-bit — `0x01`
   for the stereo STR tracks — repeated once) + 2304 ADPCM bytes + 20 unused + 4-byte
   EDC. The last sector of a stream carries the EOF/EOR submode bits (`0x80`/`0x01`);
   copy them from the original sector. `tools/disc/reinject.py` writes Form 1 (2048-B
   user data at +24) and must not be used as-is for XA; a `--form2` mode that copies
   the 2336-B payload and recomputes EDC is the obvious extension.
4. Leave the other 15 channels' sectors untouched (they are interleaved between
   yours) and re-run `extract_voice.py` on the patched image: the decoded sample
   counts and `audio_test.py`'s arithmetic are the regression check.

For the STR endings, the speech sits mixed into the stereo music track; a dub there
means re-mixing the whole track (music stem is not available separately) and
re-encoding the stride-8 stereo XA — feasible with the same tools, but it is a
remix job, not a line swap.
