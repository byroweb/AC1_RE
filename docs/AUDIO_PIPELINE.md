# Voice-line pipeline — `tools/audio/`

Target: **SLUS-01323 (v1.1)**. Extracts every spoken line on the disc, transcribes
it in whichever language it is actually in (the disc has Japanese lines left in),
gates out ambience / sound effects, groups the clips by speaker, lines each
transcript up with the mission-text corpus and writes a manifest the owner
verifies **by listening instead of typing**. The Japanese and non-speech clips
are located so their XA slots can be **reclaimed for other content** (§6,
`disc_map/voice_reclaimable.tsv`), not to be translated.
Companion to [`AC1_TEXT_SYSTEM.md`](AC1_TEXT_SYSTEM.md) / [`MIS_FORMAT.md`](MIS_FORMAT.md)
(the text side of a localization) — this is the audio side, and §7 is the dub:
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
| 6 match | `match_text.py` | English text of speech clips + `disc_map/extracted/MIS_T` → `matches.json` |
| 7 manifest | `build_manifest.py` | all of the above → **`disc_map/voice_manifest.tsv`**, `review.html` |
| 8 refs | `xa_refs.py` | `disc_map/extracted/FDAT_T/entries/200.bin` (sound sequences) + mission scripts → `xa_line_refs.json` |
| 9 reclaim | `reclaimable.py` | manifest + refs + `channels.json` → **`disc_map/voice_reclaimable.tsv`** (§6) |
| 10 dub | `reinject_xa.py` | a take WAV + a manifest row → the row's sectors of a **copy** of the disc (§7) |

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
  line exists on the disc anyway — see §6). Records carry `schema: 2`; older
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
| `transcript` | Whisper's transcript in the clip's own language (the Japanese text for a `speech_ja` clip); blank for `ambience` / `sfx` |
| `language`, `language_prob` | Whisper's detected language (ISO 639-1: `en`, `ja`, …) and its probability |
| `kind` | `speech_en` / `speech_ja` / `ambience` / `sfx` / `unknown` — **the owner's column**: change it and it sticks (TSV edit or review-page dropdown) |
| `kind_auto` | what `classify.py` said; `kind != kind_auto` is how a re-run recognises an owner edit |
| `confidence`, `avg_logprob`, `no_speech_prob` | Whisper decode stats; `confidence = exp(avg_logprob)·(1−no_speech_prob)` |
| `matched_string_id`, `matched_text`, `similarity` | best corpus string (0–100) |
| `status` | `empty` (no text, or not speech) / `check` / `matched` (≥85) / `verified` (owner) |
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
The *kind · lang* column shows the kind (with the gate's verdict in brackets when
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
- The in-mission radio lines have **no on-screen text** in AC1, so most TALK rows will
  not match the corpus — that is expected; matching exists for the lines that are
  catalogued (mission names, cutscene text) and to catch near-duplicates.
- Silero VAD found no speech in `ACOPA`/`ACOPB`/`STAFF`/`DEMOPLAY` (music only).
  `--method energy` or lowering the VAD threshold in `split_by_vad` would force clips
  out of them if a line is suspected.
- Clustering threshold: short, radio-filtered lines make embeddings noisy. If two
  speakers merge, lower `--threshold` (or set `--n-voices`); if one speaker splits,
  raise it. Name the clusters only after the cut looks right.
- Everything runs on CPU. The Intel Arc GPU is unused (no CUDA; CTranslate2 has no
  oneAPI build).

## 6. Reclaimable XA slots (`disc_map/voice_reclaimable.tsv`)

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
mono take of up to that length (§7). It is **not contiguous free space**: the other
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

Side finding worth its own task: the sequence blob holds the **on-screen subtitle
of every radio line** — the text `match_text.py` could never find in `MIS.T`.
`xa_line_refs.json` (local) carries it per line.

## 7. Reinjection — putting a dub back (CONFIRMED on a copy of the disc)

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

Not yet exercised: **playback in the game** (DuckStation / hardware) of a patched
image — the sector format is byte-for-byte what the disc uses, so this is expected
to just work, but it is the next thing to check, ideally with a mission that
triggers the dubbed channel.

For the STR endings, the speech sits mixed into the stereo music track; a dub there
means re-mixing the whole track (music stem is not available separately) and
re-encoding the stride-8 stereo XA — the codec here is mono-only (stereo groups
interleave L/R units), so that is a remix job plus a small encoder extension, not a
line swap.
