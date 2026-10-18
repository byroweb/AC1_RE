# Voice-line pipeline — `tools/audio/`

Target: **SLUS-01323 (v1.1)**. Extracts every spoken line on the disc, transcribes
it in whichever language it is actually in (the disc has Japanese lines left in),
gates out ambience / sound effects, groups the clips by speaker, lines each
transcript up with the mission-text corpus and writes a manifest the owner
verifies **by listening instead of typing**. The Japanese and non-speech clips
are located so their XA slots can be **reclaimed for other content** (§7,
`disc_map/voice_reclaimable.tsv`), not to be translated.
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

`disc_files.json` lists each STR's audio as an XA child (`GG/STR/ACED1.STR[0.0]`
etc., with Channel / Stereo? / Samples/Sec / Sector stride); the pipeline still
enumerates streams from its own fresh jPSXdec index rather than from the JSON.

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
tools/audio/.venv/Scripts/python tools/audio/run_all.py            # everything, ~35 min CPU
tools/audio/.venv/Scripts/python tools/audio/run_all.py --from classify   # after editing a kind
tools/audio/.venv/Scripts/python tools/audio/run_all.py --from refs       # just the reclaim report
tools/audio/.venv/Scripts/python tools/audio/run_all.py --from match --out /other/checkout/AUDIO \
    --corpus /other/checkout/MIS_T/toc.json --fdat-entries /other/checkout/FDAT_T/entries   # from a worktree
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
| 3 transcribe | `transcribe.py` | clips → `transcripts.json` (language per clip, Japanese text + English gloss) |
| 4 classify | `classify.py` | clips + transcripts (+ the manifest's `kind_override`) → `kinds.json` |
| 5 speakers | `speakers.py` | speech clips → `speakers.json`, `speakers_emb.npz` |
| 6 match | `match_text.py` | English text of speech clips + the radio subtitles in `FDAT_T/entries/200.bin` + `disc_map/extracted/MIS_T` → `matches.json` (free match + the clip's own line) |
| 7 manifest | `build_manifest.py` | all of the above → **`disc_map/voice_manifest.tsv`**, `review.html` |
| 8 refs | `xa_refs.py` | `disc_map/extracted/FDAT_T/entries/200.bin` (sound sequences) + mission scripts → `xa_line_refs.json` |
| 9 reclaim | `reclaimable.py` | manifest + refs + `channels.json` → **`disc_map/voice_reclaimable.tsv`** (§7) |
| 10 dub | `reinject_xa.py` | a take WAV + a manifest row → the row's sectors of a **copy** of the disc (§6) |

### Stage notes
- **Split.** TALK channels use a numpy RMS gate (20 ms frames, −40 dBFS open,
  8 dB hysteresis, 350 ms close gap, 150 ms minimum, 100 ms padding; the gate
  rises with a hiss floor by at most 10 dB). STR tracks have music under the voice,
  so they use Silero VAD (bundled with faster-whisper) on a 16 kHz downmix. Both are
  deterministic; the energy gate is unit-tested in `audio_test.py`.
- **Transcribe.** faster-whisper `large-v3`, int8, beam 5, `condition_on_previous_text`
  off, word timestamps on. Per clip, two steps:
  1. **language detection** with no prompt (`detect_language`): code, probability
     and the top-5 language probabilities go into the record. The first version of
     this stage forced `language=en`, and Whisper obligingly wrote confident English
     over the Japanese lines ("This is Valder." for こちらバルダー) and over music;
     detection is what fixes that. `--language en` restores the forced mode.
  2. **transcription in the detected language.** Only English clips get the
     `initial_prompt` of AC1 vocabulary (Raven, Ravens' Nest, Chrome, Murakumo
     Millennium, Nine-Ball, Hustler One, Progtech, AC, MT, Isaac City, mission
     names); a Japanese clip is decoded as Japanese with no prompt, so `text` is the
     Japanese line. Edit `AC1_PROMPT` in `transcribe.py` or pass `--prompt-file`.
  There is deliberately **no translation pass**: the Japanese lines are found to
  reclaim their space, not to read them (and the English subtitle of every radio
  line exists on the disc anyway — see §7). Records carry `schema: 2`; older
  records are redone automatically on the next run. On these clips the detector
  is decisive: English lines sit at 0.79–0.99 and the Japanese ones at 0.93–1.00,
  with `en`/`ja` never close on the same clip. Result on this disc: **108 en, 24 ja,
  1 undecided** (`nn` 0.25 on a music-only slice).
- **Classify (the non-speech gate).** `classify.py` tags every clip
  `speech_en` / `speech_ja` / `ambience` / `sfx` / `unknown` from three independent
  views and writes `kinds.json`:
  - Whisper's own doubt: `no_speech_prob` (≥ 0.6 = "no speech", 0.3–0.6 = grey),
    the detected language's probability (< 0.5 = coin toss), the stock hallucination
    phrases ("Thank you for watching", "you", …) or empty text;
  - the signal, 16 kHz mono, no model: `voiced` (fraction of active frames with an
    autocorrelation pitch peak > 0.5 in 60–400 Hz), spectral `flat`ness over
    100–4000 Hz, `env_std` (std of frame level in dB — syllables make speech bumpy),
    `mod` (share of envelope modulation at the 2–10 Hz syllable rate), `crest`
    (peak/RMS, dB) and duration. "Noise-like" is `voiced < 0.08 and flat > 0.25`;
    "steady" is `env_std < 3 dB or mod < 0.15`.
  - the owner: the manifest's **`kind` column is the owner's**. A row whose `kind`
    differs from its `kind_auto` (edited in the TSV or with the review page's
    dropdown) keeps that kind on every re-run and feeds it back into this stage.
  Evidence against speech is scored (Whisper doubt, noise-like, steady, weak
  language, hallucinated text) against evidence for it (sure language, low
  `no_speech_prob`, pitched bumpy signal); a clip loses its speech status only
  when the "against" side wins **and** at least one hard signal fired (Whisper
  says no speech, or the signal is noise-like / steady, or the text is a
  hallucination on a coin-toss language). A non-speech clip under 1.5 s or with a
  crest above 20 dB is `sfx`, otherwise `ambience`. Everything else is speech in
  the detected language, or `unknown` when neither `en` nor `ja` reaches 0.5.
  `uncertain` is set whenever the views disagree (e.g. Whisper is sure, the
  signal says hiss; or `en` and `ja` within 0.3 of each other): the manifest notes
  it and the review page filters on it — those are the rows to listen to first.
  Calibration on the AC1 clips: TALK speech has `voiced` 0.03–0.84 (the vocoded
  Nine-Ball-style lines in TALK04 are the low end), `flat` 0.02–0.23, `env_std`
  5.6–9.8 dB; STR speech over music drops to `env_std` 3.9–4.6 and `voiced` 0.00–0.27
  (the music masks the pitch tracker — hence the margins above). The only clips
  the gate removes on this disc are the four music-only slices the VAD cut out of
  the ending movies (`ACED3_str_c000`, `ACED3_str_c008`, `ACED5_str_c000/c001`),
  all with a hallucinated "Thank you for watching" at language probability ≤ 0.30.
  The thresholds are plain constants at the top of `classify.py`; `--print` dumps
  one feature line per clip for retuning, `--no-override` ignores the manifest.
  Downstream, `speakers.py` embeds every clip but clusters only speech (ambience /
  sfx land in the pseudo-voice `nonspeech`; `--include-nonspeech` to cluster all),
  `match_text.py` skips non-speech and non-English clips (the corpus is English),
  and `build_manifest.py` blanks the transcript of non-speech rows, keeping
  Whisper's hidden guess in `auto_notes`. Result on this disc: **105 speech_en,
  24 speech_ja, 3 sfx, 1 ambience, 0 unknown; 2 uncertain** (`TALK02_ch10_c000`,
  `TALK04_ch09_c000`: English at 0.79, vocoded voices — both really are speech).
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
- **Match.** Two corpora, scored with rapidfuzz `token_sort_ratio` on normalised
  text (lower-case words, digits spelled out so the subtitles' timer digits
  "30 seconds" meet Whisper's "thirty seconds"):
  1. **the radio subtitles** — the sound-sequence blob of `FDAT.T` entry 200 (§7:
     every sequence is opcodes + the on-screen English subtitle of the line it
     plays). One candidate per sequence with text, id `SEQ:<snd>:X<n>` (sequence
     `snd` plays TALK line `n = 16·file + channel`) or `SEQ:<snd>` when the
     sequence has no `X` opcode (music cues, text-only radio); the leading
     "Radio:" / "COM :" speaker tag is split into a `speaker` field. On by
     default when `disc_map/extracted/FDAT_T/entries/200.bin` exists (or an
     `xa_line_refs.json` from `xa_refs.py` is in the work dir); `--fdat-entries`
     points elsewhere, `--no-sequences` leaves it out. 163 sequences carry text,
     78 of them play a TALK line.
  2. every draw_string message of `GG/MS/MIS.T` (split exactly as `scan_text.py`
     does, plus per-sentence candidates). Ids follow `disc_map/text_inventory.csv`:
     `MIS_T:<entry>:+0x<off>[#sN]`. Add corpora with `--corpus <toc.json>` or
     `--all-extracted`.

  Each clip gets the free best match over the union (`best_*`, top-3 candidates
  kept for the review page) **and, for TALK clips, a direct hit**: the clip's own
  line number is known from its file and channel (`xa_refs.line_number`), so the
  transcript is scored against that line's own subtitle — the whole text, each
  sentence and each run of consecutive sentences (`#sI-J`), since a channel split
  into several clips yields a clip per sentence or two — and `own_line_id /
  own_line_text / own_line_speaker / own_line_similarity` record it. That is the
  script line the slot is supposed to say, and the manifest prefers it (§4).
  Result on this disc: **82 TALK clips have their line's subtitle, 81 match it at
  ≥ 85** (the one below 60 is `TALK01_ch09_c000`: the Japanese take under
  "A...a friend! W..we're saved! Thank God!!"); the other 23 TALK clips sit in
  lines 57, 67 and 80–95, which no sequence plays, so they have no subtitle
  (the Japanese slots). STR clips have no own line and keep the free match.

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
| `transcript` | Whisper's transcript in the clip's own language (the Japanese text for a `speech_ja` clip); blank for `ambience` / `sfx` |
| `language`, `language_prob` | Whisper's detected language (ISO 639-1: `en`, `ja`, …) and its probability |
| `kind` | `speech_en` / `speech_ja` / `ambience` / `sfx` / `unknown` — **the owner's column**: change it and it sticks (TSV edit or review-page dropdown) |
| `kind_auto` | what `classify.py` said; `kind != kind_auto` is how a re-run recognises an owner edit |
| `confidence`, `avg_logprob`, `no_speech_prob` | Whisper decode stats; `confidence = exp(avg_logprob)·(1−no_speech_prob)` |
| `matched_string_id`, `matched_text`, `similarity` | for a TALK clip whose own line has a subtitle: **that subtitle** (`SEQ:<snd>:X<n>[#s…]`) and the transcript's score against it, whatever the free corpus match said; otherwise the best corpus string (0–100) |
| `own_line` | the TALK line number `16·file + channel` the clip's channel plays as (blank for STR); cross-reference to `voice_reclaimable.tsv` and `xa_refs.py --print` |
| `status` | `empty` (no text, or not speech) / `check` / `matched` (own subtitle, or without one a corpus string, at ≥85) / `verified` (owner) |
| `auto_notes` | pipeline flags: the gate's reasons and hidden guess for non-speech, "Japanese line", "classification uncertain", kind overridden, processed voice, same recording as …, cutscene track, low confidence |
| `verified_text`, `verified`, `kind` (when changed), `notes` | owner columns — survive re-runs (carried over by `clip_id`); `notes` is yours, never auto-filled |

Column order: the pre-language columns keep their order (the new ones sit between
`transcript` and `confidence`), `clip_id`s are unchanged, and the review page's
stored edits (same localStorage key and field names) still apply. `voice_label` is
carried **per clip**: when a re-cluster renumbers the voices, each labelled clip
votes its old label onto its new `voice`, so names follow the speaker, not the id.

**Workflow:** open `disc_map/extracted/AUDIO/review.html` (local file, never publish —
it plays game audio). Rows are sorted lowest confidence first; play, fix the text or
press *accept* on a corpus match, tick *verified*, name voices in the voice box.
The *corpus match* column shows the slot's own subtitle for TALK rows (tagged
*own line* with its speaker tag; the other candidates, including a free corpus
hit that differs, are the small buttons) and "no subtitle" for the lines no
sequence plays. The *kind · lang* column shows the kind (with the gate's verdict in brackets when
you changed it), the detected language and its probability, and the kind
dropdown; the header counts clips per kind and language and the *kind* filter
has `uncertain` for the rows the gate was unsure about. Edits persist in the
browser; **Export TSV** downloads the merged manifest — copy it over
`disc_map/voice_manifest.tsv` and commit. A changed `kind` is honoured by the next
`classify.py` run (the clip is then also re-clustered / matched / reported
accordingly): `run_all.py --from classify`. Edits made with the previous version of
the page (before the kind column existed) still load: same storage key, same
field names, and a voice label follows its clip to the clip's current voice id.

## 5. Caveats

- Whisper hallucinates on 1-second clips and on music; the gate (`kind`) and
  `no_speech_prob` catch most of it, but every `check` row and every `uncertain`
  one needs a listen. The gate was tuned on this disc's 133 clips: a new split or
  model may move the feature distributions (see the constants in `classify.py`).
- Language detection is per clip on the first 30 s; a clip that mixes languages
  (none found here) would get one language for the whole clip.
- The in-mission radio lines have no text in `MIS.T`; their subtitles live in the
  sound-sequence blob (§7) and are matched per slot (own line, §3). A TALK row
  whose own subtitle scores low is either a mis-transcription or a slot whose
  audio is not the subtitle's line (`auto_notes` says so; TALK01 ch9 is the one
  such slot on this disc). Only the STR cutscene rows and the Japanese slots are
  left to the free corpus match.
- Silero VAD found no speech in `ACOPA`/`ACOPB`/`STAFF`/`DEMOPLAY` (music only).
  `--method energy` or lowering the VAD threshold in `split_by_vad` would force clips
  out of them if a line is suspected.
- Clustering threshold: short, radio-filtered lines make embeddings noisy. If two
  speakers merge, lower `--threshold` (or set `--n-voices`); if one speaker splits,
  raise it. Name the clusters only after the cut looks right.
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

### Repeating the test: `tools/duckstation/radio_line.py`

The procedure above is one committed tool (standard library only for the emulator
part; numpy/scipy/PyAV from `tools/audio/.venv` for the correlation). Given a clip id
from the voice manifest (or a raw `--id`), the `.cue` of the disc copy and a state id
from `disc_map/states/STATES.tsv`, it stages the state into a scratch slot with the
header's media path rewritten to that `.cue`, boots the copy (shutting down whatever
runs first), loads the slot, breaks on `0x8005804C`, calls `FUN_8004DB18(id)` with all
registers saved and restored, records with the `capture` MCP tool, decodes the MP4 and
prints the NCC of the take and of every channel decode of the stream against the capture:

```bash
tools/audio/.venv/Scripts/python tools/duckstation/radio_line.py \
    --clip TALK00_ch12_c000 --disc ~/dev/ac1_dub.cue --state M30-ctrl-01 --slot 9 \
    --capture out/dub.mp4 --take take.wav --wav-out out/dub.wav --json out/dub.json \
    --shutdown --boot-after "${AC1_BIN%.bin}.cue" --load-after 3 --delete-slot
tools/audio/.venv/Scripts/python tools/duckstation/radio_line.py \
    --clip TALK00_ch12_c000 --disc "${AC1_BIN%.bin}.cue" --capture out/pristine.mp4   # reference run
tools/audio/.venv/Scripts/python tools/duckstation/radio_line.py \
    --clip TALK00_ch12_c000 --analyse-only out/dub.mp4 --take take.wav         # correlate later
```

`--disc` may be the pristine `.cue` for a reference run (it is only booted, never
written). The slot is refused unless it is empty, archived in `STATES.tsv`, or was
written by the tool itself (it leaves a `.radio_line.json` marker next to the slot
file and `--delete-slot` only removes slots with a matching marker; `--force`
overrides both). Expect ~3 minutes per run: once a breakpoint has been used, every
MCP call takes ~2 s for the rest of the emulator session, most of it in the restore
of the 35 saved registers. The tool checks that the XA file table entry's `CdlLOC`
matches the manifest's first sector of the stream and that the radio-state id at
`0x80041BBE` is the requested one, and logs the drive's LBA so a run that never
read the stream's sectors is visible.

Re-run on 2026-10-18 with the tool on a fresh copy (`TALK00_ch12_c000`, same take,
state `M30-ctrl-01`, slot 9, 2 min 43 s wall clock): file table entry 3 `CdlLOC` = LBA
60284 as in the manifest, the player returned in 6.2 s, radio state `3c 00 01 01 ff`
then `f0` after the line, the drive logged inside `TALK00.XA` throughout (LBA
60682 → 61640), and the capture's NCC was **0.950 against the take** at 0.56 s,
0.043 against the original `TALK00_ch12` and ≤ 0.063 against every other channel of
the stream — the documented result, reproduced.

Not done: hardware (a burned disc / ODE) and a line triggered by a mission script
rather than the debugger — the latter only needs a save state in a mission whose
script has `play_sound` (missions 1, 3, 4, 6, 9, 10, 12, 13, 14, 17, 18, 21, 26,
27, 33, 40, 46, 49), none of which is reachable from the archived states yet.

For the STR endings, the speech sits mixed into the stereo music track; a dub there
means re-mixing the whole track (music stem is not available separately) and
re-encoding the stride-8 stereo XA — the codec here is mono-only (stereo groups
interleave L/R units), so that is a remix job plus a small encoder extension, not a
line swap.

## 7. Reclaimable XA slots (`disc_map/voice_reclaimable.tsv`)

The Japanese lines and the non-speech slices are dead weight for an English or
other-language build. `reclaimable.py` lists every clip whose `kind` is
`speech_ja`, `ambience` or `sfx` with where it sits and what it holds, plus a
subtotal per file and a grand total; `xa_refs.py` adds whether anything on the
disc still points at the line. Both read the committed manifest, so a `kind`
you change in the review page changes the report on the next run.

**What a reclaimed clip is — and is not.** The TALK files interleave 16 mono
channels with sector stride 16 and the game plays a line as *(file, channel)*,
streaming that channel from the file's first sector. A reclaimed clip is a
**mono 37.8 kHz XA slot of `audio_sectors` sectors in that channel** (4032
samples = 0.1067 s each), which `tools/audio/reinject_xa.py` can re-fill with any
mono take of up to that length (§6). It is **not contiguous free space**: the other
15 channels' sectors between its sectors stay in use, and nothing else can be
stored there without re-authoring the whole file. The byte columns are
`xa_payload_bytes = audio_sectors × 2304` (the ADPCM sound data of a Form 2
audio sector) and `raw_bytes = audio_sectors × 2352` (what the sectors occupy on
the image). A **whole channel** is free only when every clip in it is
reclaimable (`channel_free`); the channel's zero padding and inter-clip silence
are then free too, hence `channel_sectors ≥` the clip sectors. STR clips are
speech mixed into the ending movies' stereo music track and cannot be reclaimed
without remixing — they are listed with `reclaimable = no`.

**Result on this disc** (classifier output, before the owner's listening pass):

| what | clips | audio sectors | seconds | XA payload |
| --- | --- | --- | --- | --- |
| `GG/BGM/TALK05.XA` — 16 channels, each the **same 0.85 s Japanese take** (lines 80–95, ids `0x80`–`0x8F`) | 16 | 128 | 13.6 | 294 912 B |
| `GG/BGM/TALK03.XA` ch 9 — four Japanese clips (line 57, `0x69`) | 4 | 66 | 6.8 | 152 064 B |
| `GG/BGM/TALK04.XA` ch 3 — three Japanese clips (line 67, `0x73`) | 3 | 65 | 6.7 | 149 760 B |
| `GG/BGM/TALK01.XA` ch 9 — the same 0.85 s Japanese take again (line 25, `0x49`) | 1 | 8 | 0.85 | 18 432 B |
| STR music-only slices (`ACED3_str_c000/c008`, `ACED5_str_c000/c001`) | 4 | — | — | not reclaimable |
| **total, TALK** | **24** | **267** | **28.0** | **615 168 B** (627 984 B raw) |

19 whole channels are free (all 16 of TALK05, TALK03 ch9, TALK04 ch3, TALK01 ch9);
with their padding that is 285 channel sectors (TALK03 ch9 is 76 sectors long, TALK04 ch3 73, the TALK05 channels 8 each).

**Reference scan (CONFIRMED mechanism, static result).** Read the mission overlay
(`disc_map/overlays/ovl202_mission.bin`, load base `0x8004ADA0`):
`play_sound <snd>` in a mission's chunk-4 script → `FUN_800529F0(0,0,snd)` →
`u16 table[snd]` of the **sound-sequence blob** → the sequence interpreter
`0x800532A0`. A sequence is ASCII text: opcode letter + decimal operand
(parser `0x80052350`), e.g. `M3X40N`, followed by the line's **English subtitle**
in the font encoding (`A`=0xAA, `a`=0xC4, `0`=0xA0, `.`=0x8E …). Opcode **`X n`**
(and `x`) calls `FUN_8004DB18(n + 0x30)`, i.e. TALK line `n = 16·file + channel`;
`V n`/`v n` is the music player (`0x90 + n`); no other letter reaches the XA player
(jump table `0x8004B0A8`). The blob is in **`FDAT.T` entry 200** (three copies,
offsets 244236 / 256064 / 269092; 200 sequences; the game loads it at
`0x801FD850`). Two anchors tie it all together: mission 49's script plays
sequences 91 and 94 = `X74` / `X77` = TALK04 ch10 / ch13, whose Whisper
transcripts are exactly the subtitles "Your fate is sealed." / "Go back. It is
not too late."; mission 27 plays 57 = `X40` = TALK02 ch8 "With your skill…".

Status per line (`referenced` column of the report; `xa_refs.py --print` lists all 96):
- **referenced** — some sequence contains `X n`. 17 lines are reached from a
  chunk-4 script's `play_sound`; another 61 sit in sequences that no chunk-4 script
  calls, so the primary objective VM or game code must trigger those (not
  scanned) — they count as referenced too.
- **unreferenced** — **no sequence contains `X n` at all**: lines 57, 67 and
  80–95, i.e. exactly the Japanese slots in TALK03 ch9, TALK04 ch3 and TALK05.
  Nothing can play them through the sequence path. The one Japanese slot that *is*
  wired is TALK01 ch9 (line 25, sequence 38, subtitle "A...a friend! We're saved!
  Thank God!!" — the English take was evidently never put there; sequence 38 is
  itself called by no chunk-4 script).
- **unchecked** — STR tracks (not addressable lines) or `xa_line_refs.json` missing.

Caveats: this is a static scan. `FUN_800529F0` / `FUN_8004DB18` can be called with
constants from code (the mission-result jingles are: sequence 101), the mode1 /
mode2 overlays and the arena are not scanned, and the scripts' `call_db18`
operands are small state values (1, 2), not line ids. "Unreferenced" means
"nothing found", not "never played"; confirm in DuckStation
(`tools/duckstation/radio_line.py`) before overwriting a slot that matters.

The sequence blob holds the **on-screen subtitle of every radio line** — the text
`match_text.py` could never find in `MIS.T`; since the subtitle corpus (§3) it is
what the manifest's `matched_text` shows for TALK rows. `xa_line_refs.json`
(local) carries it per line. Font codes `0x80`–`0x89` in the subtitles are a
second digit set (the timer digits: "30 seconds", "2 minutes", the "3, 2, 1"
countdown of sequence 15 — inferred from the audio, not from code); a few
subtitles embed opcode letters (`W11` waits in the countdowns), which
`decode_text` leaves as `<57><31><31>`.
