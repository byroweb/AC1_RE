#!/usr/bin/env python3
"""
audio_test.py — unit tests for the voice pipeline's bookkeeping.

Covers the two pieces that must be right for re-injection to be possible later:
  * ac1audio.XAStream — sample <-> disc-sector arithmetic for interleaved XA
    streams (mono stride-16 TALK channels, stereo stride-8 STR tracks), and the
    jPSXdec .idx parser that feeds it;
  * split_utterances.split_by_energy — the silence splitter, on synthetic audio.

Needs only numpy (the tools/audio/.venv has it). No game data is read.
  tools/audio/.venv/Scripts/python tools/audio/audio_test.py
"""
import os
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ac1audio as A  # noqa: E402
import build_manifest as M  # noqa: E402
import classify as C  # noqa: E402
import reclaimable as R  # noqa: E402
import transcribe as T  # noqa: E402
import xa_refs as X  # noqa: E402
from split_utterances import split_by_energy  # noqa: E402

RATE = 37800

IDX_SAMPLE = """\
; jPSXdec index sample (two streams + one video, as v2.1 writes them)
Filename:fake.bin|Sector size:2352|Sector count:100|First sector offset:0
[Index]
#:226|ID:GG/BGM/TALK00.XA|Sectors:60284-62971|Type:File|Size:5505024|Path:GG/BGM/TALK00.XA|Has mode 2 form 2:Yes|Has CD audio:No
#:227|ID:GG/BGM/TALK00.XA[0]|Sectors:60284-60908|Type:XA|File:1|Channel:0|Stereo?:No|Samples/Sec:37800|Bits/Sample:4|Sector stride:16|Disc speed:2x
#:2591|ID:GG/STR/ACED5.STR[0]|Sectors:155981-157931|Type:Video|Dimensions:320x240|Frame Count:390
#:2592|ID:GG/STR/ACED5.STR[0.0]|Sectors:155988-157788|Type:XA|File:1|Channel:1|Stereo?:Yes|Samples/Sec:37800|Bits/Sample:4|Sector stride:8|Disc speed:2x
"""


class XAStreamMath(unittest.TestCase):
    def talk(self):
        # GG/BGM/TALK05.XA[0] as indexed: 8 audio sectors, stride 16, mono 4-bit
        return A.XAStream("GG/BGM/TALK05.XA[0]", 71740, 71852, 16, channel=0, stereo=False)

    def test_sizes_mono(self):
        s = self.talk()
        self.assertEqual(s.n_sectors, 8)
        self.assertEqual(s.samples_per_sector, 4032)
        self.assertEqual(s.total_samples, 32256)  # == what jPSXdec decodes for this item
        self.assertAlmostEqual(s.duration_s, 32256 / 37800)

    def test_sizes_stereo(self):
        s = A.XAStream("GG/STR/ACED5.STR[0.0]", 155988, 157788, 8, channel=1, stereo=True)
        self.assertEqual(s.n_sectors, 226)
        self.assertEqual(s.samples_per_sector, 2016)
        self.assertEqual(s.total_samples, 226 * 2016)

    def test_eight_bit(self):
        s = A.XAStream("x", 100, 116, 16, stereo=False, bits=8)
        self.assertEqual(s.samples_per_sector, 2016)
        s = A.XAStream("x", 100, 116, 16, stereo=True, bits=8)
        self.assertEqual(s.samples_per_sector, 1008)

    def test_sector_of_sample(self):
        s = self.talk()
        self.assertEqual(s.sector_of_sample(0), 71740)
        self.assertEqual(s.sector_of_sample(4031), 71740)
        self.assertEqual(s.sector_of_sample(4032), 71756)      # next audio sector = +stride
        self.assertEqual(s.sector_of_sample(32255), 71852)     # last sample -> sector_last
        with self.assertRaises(ValueError):
            s.sector_of_sample(32256)

    def test_sector_range_and_inverse(self):
        s = self.talk()
        self.assertEqual(s.sector_range(0, 4032), (71740, 71740))
        self.assertEqual(s.sector_range(4000, 4100), (71740, 71756))  # straddles a boundary
        self.assertEqual(s.sector_range(8064, 32256), (71772, 71852))
        self.assertEqual(s.sample_range_of_sector(71756), (4032, 8064))
        self.assertEqual(s.audio_sector_index(8063), 1)
        with self.assertRaises(ValueError):
            s.sample_range_of_sector(71757)  # not on the stride grid
        with self.assertRaises(ValueError):
            s.sector_range(10, 10)

    def test_every_sample_round_trips(self):
        s = A.XAStream("x", 1000, 1000 + 16 * 4, 16)
        for smp in range(0, s.total_samples, 97):
            sec = s.sector_of_sample(smp)
            a, b = s.sample_range_of_sector(sec)
            self.assertTrue(a <= smp < b)

    def test_bad_span_rejected(self):
        with self.assertRaises(ValueError):
            A.XAStream("x", 100, 109, 16)

    def test_dict_round_trip(self):
        s = A.XAStream("GG/STR/ACED5.STR[0.0]", 155988, 157788, 8, channel=1, stereo=True)
        d = s.to_dict()
        t = A.XAStream.from_dict(d)
        self.assertEqual(t.to_dict(), d)
        self.assertTrue(d["stereo"])


class IdxParsing(unittest.TestCase):
    def test_parse_and_classify(self):
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "t.idx")
            with open(p, "w") as f:
                f.write(IDX_SAMPLE)
            items = A.parse_idx(p)
        self.assertEqual([it["idx"] for it in items], [226, 227, 2591, 2592])
        talk = [it for it in items if A.is_talk_item(it)]
        strs = [it for it in items if A.is_str_audio_item(it)]
        self.assertEqual([t["id"] for t in talk], ["GG/BGM/TALK00.XA[0]"])
        self.assertEqual([t["id"] for t in strs], ["GG/STR/ACED5.STR[0.0]"])
        st = A.XAStream.from_idx_item(talk[0])
        self.assertEqual((st.sector_first, st.sector_last, st.stride, st.stereo, st.rate),
                         (60284, 60908, 16, False, 37800))
        self.assertEqual(st.n_sectors, 40)
        st2 = A.XAStream.from_idx_item(strs[0])
        self.assertTrue(st2.stereo)
        self.assertEqual(st2.stride, 8)

    def test_short_names(self):
        self.assertEqual(A.stream_short_name("GG/BGM/TALK03.XA[12]"), "TALK03_ch12")
        self.assertEqual(A.stream_short_name("GG/STR/ACED1.STR[0.0]"), "ACED1_str")
        self.assertEqual(A.source_kind("GG/BGM/TALK03.XA[12]"), "TALK")
        self.assertEqual(A.source_kind("GG/STR/ACED1.STR[0.0]"), "STR")


def burst(t0, t1, level_db=-12.0, seed=0):
    """White-noise burst spanning [t0, t1) seconds at the given RMS level."""
    n = int((t1 - t0) * RATE)
    rng = np.random.default_rng(seed)
    y = rng.standard_normal(n).astype(np.float32)
    return int(t0 * RATE), y / (np.sqrt((y ** 2).mean()) + 1e-9) * 10 ** (level_db / 20)


def build(total_s, bursts):
    x = np.zeros(int(total_s * RATE), dtype=np.float32)
    for t0, t1, lvl in bursts:
        s, y = burst(t0, t1, lvl, seed=int(t0 * 1000))
        x[s:s + len(y)] += y
    return x


class SilenceSplitter(unittest.TestCase):
    TOL = int(0.15 * RATE)  # 100 ms pad + one 20 ms window + hop slop

    def test_three_utterances_over_digital_silence(self):
        x = build(6.0, [(0.5, 1.5, -12), (2.5, 2.9, -15), (4.0, 5.2, -10)])
        spans = split_by_energy(x, RATE)
        self.assertEqual(len(spans), 3)
        for (s, e), (t0, t1, _) in zip(spans, [(0.5, 1.5, 0), (2.5, 2.9, 0), (4.0, 5.2, 0)]):
            self.assertLessEqual(abs(s - int(t0 * RATE)), self.TOL)
            self.assertLessEqual(abs(e - int(t1 * RATE)), self.TOL)
            self.assertLess(s, e)

    def test_short_blip_dropped_and_small_gap_bridged(self):
        # 50 ms click: below min_speech_ms -> dropped. Two bursts 150 ms apart: one clip.
        x = build(5.0, [(0.5, 0.55, -12), (2.0, 2.6, -12), (2.75, 3.3, -12)])
        spans = split_by_energy(x, RATE, min_speech_ms=150, min_gap_ms=350)
        self.assertEqual(len(spans), 1)
        s, e = spans[0]
        self.assertLessEqual(abs(s - int(2.0 * RATE)), self.TOL)
        self.assertLessEqual(abs(e - int(3.3 * RATE)), self.TOL)

    def test_gap_longer_than_min_gap_splits(self):
        x = build(5.0, [(1.0, 1.6, -12), (2.1, 2.7, -12)])  # 500 ms apart
        self.assertEqual(len(split_by_energy(x, RATE, min_gap_ms=350)), 2)
        self.assertEqual(len(split_by_energy(x, RATE, min_gap_ms=700)), 1)

    def test_low_noise_floor_does_not_trigger(self):
        rng = np.random.default_rng(1)
        x = (rng.standard_normal(3 * RATE) * 10 ** (-60 / 20)).astype(np.float32)  # -60 dBFS hiss
        self.assertEqual(split_by_energy(x, RATE), [])
        x[RATE:2 * RATE] += burst(1.0, 2.0, -20)[1]
        self.assertEqual(len(split_by_energy(x, RATE)), 1)

    def test_runs_to_end_and_clamps(self):
        x = build(2.0, [(1.5, 2.0, -12)])
        spans = split_by_energy(x, RATE)
        self.assertEqual(len(spans), 1)
        self.assertEqual(spans[0][1], len(x))
        self.assertGreaterEqual(spans[0][0], 0)

    def test_empty_and_silent(self):
        self.assertEqual(split_by_energy(np.zeros(0, np.float32), RATE), [])
        self.assertEqual(split_by_energy(np.zeros(RATE, np.float32), RATE), [])

    def test_stereo_input_downmixed(self):
        mono = build(3.0, [(1.0, 2.0, -12)])
        stereo = np.stack([mono, mono * 0.5], axis=1)
        self.assertEqual(len(split_by_energy(stereo, RATE)), 1)

    def test_clip_sectors_follow_from_samples(self):
        # Splitter output feeds XAStream.sector_range: a 1 s clip at 0.5 s in a
        # TALK channel covers audio sectors 4..14 -> disc sectors first+4*16..first+14*16
        st = A.XAStream("GG/BGM/TALK00.XA[3]", 60287, 60287 + 16 * 60, 16, channel=3)
        x = build(3.0, [(0.5, 1.5, -12)])
        (s, e), = split_by_energy(x, RATE)
        a, b = st.sector_range(s, e)
        self.assertEqual(a, 60287 + (s // 4032) * 16)
        self.assertEqual(b, 60287 + ((e - 1) // 4032) * 16)
        self.assertEqual((a - 60287) % 16, 0)
        self.assertEqual((b - 60287) % 16, 0)


# ----------------------------------------------------------------- the gate
SR16 = 16000


def synth_speech(seconds=3.0, f0=150.0, syllable_hz=4.0, sr=SR16):
    """Voiced buzz (sawtooth, 12 harmonics) with syllable-rate amplitude bumps —
    enough like speech for the signal features: pitched, bumpy envelope."""
    t = np.arange(int(seconds * sr)) / sr
    y = sum(np.sin(2 * np.pi * f0 * k * t) / k for k in range(1, 13))
    env = 0.55 + 0.45 * np.sign(np.sin(2 * np.pi * syllable_hz * t))   # on/off syllables
    y = (y * env).astype(np.float32)
    return 0.3 * y / (np.abs(y).max() + 1e-9)


def synth_noise(seconds=3.0, level_db=-20.0, sr=SR16, seed=0):
    rng = np.random.default_rng(seed)
    y = rng.standard_normal(int(seconds * sr)).astype(np.float32)
    return y / (np.sqrt((y ** 2).mean()) + 1e-9) * 10 ** (level_db / 20)


def whisper_rec(language="en", prob=0.97, nsp=0.01, text="Raven, the target is in this sector.",
                lp=-0.15, probs=None):
    rec = {"language": language, "language_prob": prob, "no_speech_prob": nsp, "text": text,
           "avg_logprob": lp, "lang_probs": probs if probs is not None else {language: prob}}
    rec["lang_probs"].setdefault("en", prob if language == "en" else 0.01)
    rec["lang_probs"].setdefault("ja", prob if language == "ja" else 0.01)
    return rec


class SignalFeatures(unittest.TestCase):
    def test_speech_like_signal(self):
        f = C.audio_features(synth_speech())
        self.assertGreater(f["voiced"], 0.5)          # pitched frames
        self.assertLess(f["flat"], C.FLAT_NOISE)      # harmonic, not noise
        self.assertGreater(f["env_std"], C.ENV_STEADY_DB)   # syllable bumps
        self.assertGreater(f["mod"], C.MOD_SPEECH)    # 4 Hz modulation
        self.assertAlmostEqual(f["dur"], 3.0, places=2)

    def test_steady_noise_bed(self):
        f = C.audio_features(synth_noise())
        self.assertLess(f["voiced"], C.VOICED_SPEECH)
        self.assertGreater(f["flat"], C.FLAT_NOISE)
        self.assertLess(f["env_std"], C.ENV_STEADY_DB)
        self.assertLess(f["crest"], C.SFX_CREST_DB)

    def test_click_is_spiky(self):
        y = synth_noise(1.0, -45.0)
        y[8000:8040] += 0.9
        f = C.audio_features(y)
        self.assertGreater(f["crest"], C.SFX_CREST_DB)
        self.assertLess(f["dur"], C.SFX_MAX_S)

    def test_short_and_empty_input(self):
        self.assertEqual(C.audio_features(np.zeros(0, np.float32))["dur"], 0.0)
        f = C.audio_features(np.zeros(100, np.float32))
        self.assertIn("voiced", f)


class NonSpeechGate(unittest.TestCase):
    def setUp(self):
        self.speechy = C.audio_features(synth_speech())
        self.noisy = C.audio_features(synth_noise(4.0))
        self.click = C.audio_features(np.pad(synth_noise(0.3, -15.0, seed=2), (0, 4000)))

    def test_english_line(self):
        kind, unsure, why = C.classify_clip(whisper_rec(), self.speechy)
        self.assertEqual(kind, "speech_en")
        self.assertFalse(unsure, why)

    def test_japanese_line_keeps_language(self):
        rec = whisper_rec("ja", 0.99, 0.03, "こちらバルダー。", lp=-0.6)
        kind, unsure, _ = C.classify_clip(rec, self.speechy)
        self.assertEqual(kind, "speech_ja")
        self.assertFalse(unsure)

    def test_whisper_no_speech_on_noise_is_ambience(self):
        rec = whisper_rec("en", 0.35, 0.85, "Thank you for watching.", lp=-1.2,
                          probs={"en": 0.35, "nn": 0.2, "ja": 0.05})
        kind, unsure, why = C.classify_clip(rec, self.noisy)
        self.assertEqual(kind, "ambience")
        self.assertFalse(unsure, why)
        self.assertIn("no_speech_prob", why)

    def test_short_noise_burst_is_sfx(self):
        rec = whisper_rec("en", 0.4, 0.7, "", probs={"en": 0.4, "ja": 0.1})
        kind, _unsure, _ = C.classify_clip(rec, self.click)
        self.assertEqual(kind, "sfx")

    def test_confident_english_over_noise_is_flagged_not_hidden(self):
        # Whisper is sure, the signal says hiss: keep it as speech but mark it uncertain
        rec = whisper_rec("en", 0.95, 0.02)
        kind, unsure, _ = C.classify_clip(rec, self.noisy)
        self.assertEqual(kind, "speech_en")
        self.assertTrue(unsure)

    def test_hallucination_text_with_doubt_is_not_speech(self):
        rec = whisper_rec("en", 0.6, 0.45, "you", lp=-1.0, probs={"en": 0.6, "ja": 0.1})
        kind, _unsure, why = C.classify_clip(rec, self.noisy)
        self.assertIn(kind, C.NONSPEECH)
        self.assertIn("hallucination", why)

    def test_language_coin_toss_is_unknown(self):
        rec = whisper_rec("nn", 0.3, 0.2, "hai wakarimashita", probs={"nn": 0.3, "en": 0.25, "ja": 0.2})
        kind, unsure, _ = C.classify_clip(rec, self.speechy)
        self.assertEqual(kind, "unknown")
        self.assertTrue(unsure)

    def test_en_vs_ja_close_call_is_uncertain(self):
        rec = whisper_rec("en", 0.5, 0.05, "Hai.", probs={"en": 0.5, "ja": 0.45})
        kind, unsure, _ = C.classify_clip(rec, self.speechy)
        self.assertEqual(kind, "speech_en")
        self.assertTrue(unsure)

    def test_music_slice_with_hallucinated_text_is_not_speech(self):
        # ACED5_str_c000-like: "Thank you for watching", en 0.30, nsp 0.29, pitched music bed
        rec = whisper_rec("en", 0.30, 0.29, "Thank you for watching.", lp=-0.6,
                          probs={"en": 0.3, "nn": 0.2, "ja": 0.05})
        kind, _unsure, why = C.classify_clip(rec, self.speechy)
        self.assertIn(kind, C.NONSPEECH)
        self.assertIn("hallucination", why)

    def test_is_hallucination(self):
        self.assertTrue(C.is_hallucination(""))
        self.assertTrue(C.is_hallucination("Thank you."))
        self.assertTrue(C.is_hallucination(" you "))
        self.assertFalse(C.is_hallucination("Raven, come in."))

    def test_override_wins(self):
        self.assertEqual(M.effective_kind("ambience", "speech_ja"), "speech_ja")
        self.assertEqual(M.effective_kind("ambience", ""), "ambience")
        self.assertEqual(M.effective_kind("ambience", "bogus"), "ambience")
        self.assertEqual(M.effective_kind("", ""), "unknown")
        # the owner's column is `kind`; it counts as an override only when it differs from kind_auto
        self.assertEqual(M.owner_kind({"kind": "sfx", "kind_auto": "ambience"}), "sfx")
        self.assertEqual(M.owner_kind({"kind": "ambience", "kind_auto": "ambience"}), "")
        self.assertEqual(M.owner_kind({"kind": "speech_ja", "kind_auto": ""}), "speech_ja")
        self.assertEqual(M.owner_kind({"kind": "nonsense", "kind_auto": "sfx"}), "")
        self.assertEqual(M.owner_kind({}), "")

    def test_top_language_probs(self):
        top = T.top_language_probs([("nn", 0.4), ("cy", 0.3), ("de", 0.1), ("fr", 0.08),
                                    ("it", 0.05), ("pt", 0.04), ("ja", 0.02), ("en", 0.01)])
        self.assertEqual(len(top), 7)             # top 5 + en + ja always present
        self.assertIn("en", top)
        self.assertIn("ja", top)
        self.assertEqual(top["nn"], 0.4)


# ------------------------------------------------------------- the manifest
OLD_COLUMNS = [  # the pre-language manifest header; must stay a subsequence of COLUMNS
    "clip_id", "source_file", "stream", "channel", "clip", "sector_first", "sector_last",
    "audio_sector_first", "audio_sector_last", "stride", "start_sample", "end_sample", "rate",
    "duration_s", "voice", "voice_label", "f0_hz", "processed", "transcript", "confidence",
    "avg_logprob", "no_speech_prob", "matched_string_id", "matched_text", "similarity",
    "status", "auto_notes", "verified_text", "verified", "notes",
]


def clip(cid, stream="TALK00_ch00", idx=0, dur=4.0, kind="TALK"):
    return {"clip_id": cid, "stream": stream, "kind": kind, "source_file": "GG/BGM/TALK00.XA",
            "channel": 0, "clip_index": idx, "rate": 37800, "start_sample": 0,
            "end_sample": int(dur * 37800), "duration_s": dur, "sector_first": 60284,
            "sector_last": 60284 + 16 * 10, "audio_sector_first": 0, "audio_sector_last": 10,
            "stride": 16, "wav": f"clips/{cid}.wav"}


class ManifestColumns(unittest.TestCase):
    def fixture(self):
        clips = [clip("A_c000"), clip("B_c000", "TALK05_ch00", dur=0.85), clip("C_c000", "ACED1_str", kind="STR")]
        trans = {
            "A_c000": whisper_rec(),
            "B_c000": whisper_rec("ja", 0.99, 0.01, "こちらバルダー。", lp=-0.65),
            "C_c000": whisper_rec("en", 0.4, 0.8, "Thank you for watching.", lp=-1.3,
                                  probs={"en": 0.4, "ja": 0.05}),
        }
        kinds = {
            "A_c000": {"kind": "speech_en", "kind_auto": "speech_en", "uncertain": False, "reasons": ""},
            "B_c000": {"kind": "speech_ja", "kind_auto": "speech_ja", "uncertain": False, "reasons": ""},
            "C_c000": {"kind": "ambience", "kind_auto": "ambience", "uncertain": False,
                       "reasons": "no_speech_prob 0.80; unvoiced noise-like signal"},
        }
        spk = {"clips": {"A_c000": {"voice": "voice_03", "f0_hz": 220.0, "processed": False},
                         "B_c000": {"voice": "voice_00", "f0_hz": 110.0, "processed": False},
                         "C_c000": {"voice": "nonspeech", "f0_hz": None, "processed": False}},
               "voices": []}
        matches = {"A_c000": {"best_id": "MIS_T:1:+0x0000", "best_text": "Raven, the target is in this sector.",
                              "similarity": 100.0, "candidates": []}}
        # the owner's previous manifest: labels under OLD voice ids, a verified line, and a kind
        # the owner changed by hand (kind != kind_auto)
        old = {"A_c000": {"clip_id": "A_c000", "voice": "voice_00", "voice_label": "operator",
                          "verified_text": "Raven, the target is in this sector.", "verified": "1", "notes": "ok"},
               "B_c000": {"clip_id": "B_c000", "voice": "voice_07", "voice_label": "Valder", "verified": "",
                          "verified_text": "", "notes": ""},
               "C_c000": {"clip_id": "C_c000", "voice": "voice_09", "voice_label": "", "verified": "",
                          "verified_text": "", "notes": "", "kind": "sfx", "kind_auto": "ambience"}}
        return clips, trans, spk, matches, kinds, old

    def test_columns_are_a_stable_superset(self):
        it = iter(M.COLUMNS)
        self.assertTrue(all(c in it for c in OLD_COLUMNS), "old columns must keep their order")
        for c in ("language", "language_prob", "kind", "kind_auto"):
            self.assertIn(c, M.COLUMNS)
        for c in ("transcript_src", "translation_en", "kind_override"):
            self.assertNotIn(c, M.COLUMNS)   # no translation step, kind itself is the owner's column
        self.assertEqual(len(set(M.COLUMNS)), len(M.COLUMNS))
        self.assertTrue(set(M.CARRY) <= set(M.COLUMNS))

    def test_rows_text_kind_and_carry_over(self):
        rows = {r["clip_id"]: r for r in M.build_rows(*self.fixture())}
        for r in rows.values():
            self.assertEqual(set(r), set(M.COLUMNS))
        a, b, c = rows["A_c000"], rows["B_c000"], rows["C_c000"]
        # English line: transcript as before
        self.assertEqual((a["kind"], a["kind_auto"], a["language"]), ("speech_en", "speech_en", "en"))
        self.assertEqual(a["transcript"], "Raven, the target is in this sector.")
        self.assertEqual(a["status"], "verified")
        self.assertEqual(a["notes"], "ok")
        # Japanese line: the Japanese text IS the transcript, no translation anywhere
        self.assertEqual((b["kind"], b["language"]), ("speech_ja", "ja"))
        self.assertEqual(b["transcript"], "こちらバルダー。")
        self.assertIn("Japanese line", b["auto_notes"])
        # ambience the owner re-tagged sfx: kind kept, gate's verdict in kind_auto, no fake transcript
        self.assertEqual((c["kind"], c["kind_auto"]), ("sfx", "ambience"))
        self.assertEqual(c["transcript"], "")
        self.assertEqual(c["status"], "empty")
        self.assertIn("Thank you for watching", c["auto_notes"])
        self.assertIn("set by owner", c["auto_notes"])
        self.assertEqual(c["voice"], "nonspeech")
        # voice labels followed their clips to the renumbered clusters
        self.assertEqual(a["voice_label"], "operator")   # was voice_00, now voice_03
        self.assertEqual(b["voice_label"], "Valder")     # was voice_07, now voice_00

    def test_manifest_round_trip(self):
        import csv
        rows = M.build_rows(*self.fixture())
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "m.tsv")
            M.write_manifest(p, rows)
            with open(p, encoding="utf-8", newline="") as f:
                rd = csv.DictReader(f, delimiter="\t")
                self.assertEqual(rd.fieldnames, M.COLUMNS)
                back = {r["clip_id"]: r for r in rd}
            self.assertEqual(back["B_c000"]["transcript"], "こちらバルダー。")
            self.assertEqual(back["C_c000"]["kind"], "sfx")
            # and the classify stage reads the owner's kind back out of it (only where it differs)
            self.assertEqual(C.load_overrides(p), {"C_c000": "sfx"})
            # a kind edited by hand in the TSV counts too
            back["A_c000"]["kind"] = "speech_ja"
            with open(p, "w", encoding="utf-8", newline="") as f:
                w = csv.DictWriter(f, fieldnames=M.COLUMNS, delimiter="\t", lineterminator="\n")
                w.writeheader()
                w.writerows(back.values())
            self.assertEqual(C.load_overrides(p), {"A_c000": "speech_ja", "C_c000": "sfx"})
            old_rows, _ = M.load_existing(p)
            again = {r["clip_id"]: r for r in M.build_rows(*self.fixture()[:5], old_rows)}
            self.assertEqual(again["C_c000"]["kind"], "sfx")
            self.assertEqual(again["A_c000"]["verified_text"], "Raven, the target is in this sector.")

    def test_missing_transcript_and_no_kinds(self):
        clips, trans, spk, matches, kinds, old = self.fixture()
        rows = {r["clip_id"]: r for r in M.build_rows(clips, {}, spk, {}, {}, {})}
        self.assertEqual(rows["A_c000"]["status"], "empty")
        self.assertEqual(rows["A_c000"]["kind"], "unknown")
        self.assertEqual(rows["A_c000"]["transcript"], "")


# ------------------------------------------------------- xa refs + reclaimable
def enc_text(s):
    """str -> the game's font encoding used by the sound-sequence subtitles."""
    out = bytearray()
    for ch in s:
        if "A" <= ch <= "Z":
            out.append(0xAA + ord(ch) - ord("A"))
        elif "a" <= ch <= "z":
            out.append(0xC4 + ord(ch) - ord("a"))
        elif "0" <= ch <= "9":
            out.append(0xA0 + ord(ch) - ord("0"))
        elif ch in " \n":
            out.append(ord(ch))
        else:
            out.append({".": 0x8E, ",": 0x8F, "?": 0x90, "!": 0x91, ":": 0x98, "'": 0x9D}[ch])
    return bytes(out)


def synth_blob(seqs):
    """A sound-sequence blob like FDAT entry 200's: header, u16 offset table, sequences."""
    body = b""
    offs = []
    table_len = 2 * len(seqs)
    start = len(X.BLOB_HEADER) + table_len
    for ops, text in seqs:
        offs.append(start + len(body))
        body += ops.encode() + enc_text(text) + b"\x00"
    import struct as st
    return X.BLOB_HEADER + b"".join(st.pack("<H", o) for o in offs) + body


class XaRefs(unittest.TestCase):
    SEQS = [("", ""), ("M3X0N", "Radio: Is that you, Raven?\n       The target is in this sector."),
            ("M1V0", "COM : Combat system engaged"), ("M1X74", "Your fate is sealed."),
            ("M2X40N", "Radio: With your skill, one on one isn't a fair fight.")]

    def test_parse_blob_and_text(self):
        data = b"\xff" * 37 + synth_blob(self.SEQS) + b"\x00" * 8
        bases = X.find_blobs(data)
        self.assertEqual(bases, [37])
        seqs = X.parse_blob(data, 37)
        self.assertEqual(len(seqs), 5)
        self.assertEqual(seqs[1]["ops"], "M3X0N")
        self.assertEqual(seqs[1]["tokens"], [("M", 3), ("X", 0), ("N", None)])
        self.assertEqual(seqs[1]["text"], "Radio: Is that you, Raven? The target is in this sector.")
        self.assertEqual(seqs[3]["tokens"], [("M", 1), ("X", 74)])
        self.assertEqual(seqs[3]["text"], "Your fate is sealed.")
        self.assertEqual(seqs[2]["tokens"], [("M", 1), ("V", 0)])

    def test_line_arithmetic(self):
        self.assertEqual(X.line_of(74), (0x7A, "TALK04", 10))     # mission 49: "Your fate is sealed."
        self.assertEqual(X.line_of(0), (0x30, "TALK00", 0))
        self.assertEqual(X.line_number("GG/BGM/TALK03.XA", 9), 57)
        self.assertEqual(X.line_number("GG/BGM/TALK05.XA", "15"), 95)
        self.assertIsNone(X.line_number("GG/STR/ACED1.STR", 1))

    def test_status(self):
        self.assertEqual(X.status_of(None)[0], "unchecked")
        self.assertEqual(X.status_of({"sequences": [91], "missions": [49]})[0], "referenced")
        st, why = X.status_of({"sequences": [38], "missions": []})
        self.assertEqual(st, "referenced")
        self.assertIn("no chunk-4 script", why)
        self.assertEqual(X.status_of({"sequences": [], "missions": []})[0], "unreferenced")

    def test_decode_text_unknown_codes(self):
        self.assertEqual(X.decode_text(enc_text("Go back. It is not too late.")), "Go back. It is not too late.")
        self.assertIn("<83>", X.decode_text(b"\x83" + enc_text(" minutes")))


class Reclaimable(unittest.TestCase):
    def manifest_rows(self):
        def row(cid, src, ch, clip, a, b, kind, dur, lang="ja"):
            return {"clip_id": cid, "source_file": src, "channel": str(ch), "clip": str(clip),
                    "sector_first": "1", "sector_last": "2", "audio_sector_first": str(a),
                    "audio_sector_last": str(b), "stride": "16", "duration_s": str(dur), "kind": kind,
                    "language": lang, "language_prob": "0.99"}
        return [
            row("TALK05_ch00_c000", "GG/BGM/TALK05.XA", 0, 0, 0, 7, "speech_ja", 0.853),
            row("TALK05_ch01_c000", "GG/BGM/TALK05.XA", 1, 0, 0, 7, "speech_ja", 0.853),
            row("TALK03_ch09_c000", "GG/BGM/TALK03.XA", 9, 0, 0, 9, "speech_ja", 1.08),
            row("TALK03_ch09_c001", "GG/BGM/TALK03.XA", 9, 1, 12, 49, "speech_en", 4.05, "en"),  # stays
            row("TALK00_ch00_c000", "GG/BGM/TALK00.XA", 0, 0, 0, 39, "speech_en", 4.27, "en"),
            row("ACED5_str_c000", "GG/STR/ACED5.STR", 1, 0, 100, 113, "ambience", 1.51, "en"),
        ]

    def test_report_rows_and_totals(self):
        refs = {"80": {"sequences": [], "missions": []}, "81": {"sequences": [], "missions": []},
                "57": {"sequences": [], "missions": []}}
        chan = {("GG/BGM/TALK05.XA", 0): 8, ("GG/BGM/TALK05.XA", 1): 8, ("GG/BGM/TALK03.XA", 9): 70}
        rep = R.build_report(self.manifest_rows(), refs, chan)
        clips = [r for r in rep if r["row"] == "clip"]
        self.assertEqual([r["clip_id"] for r in clips],
                         ["TALK05_ch00_c000", "TALK05_ch01_c000", "TALK03_ch09_c000", "ACED5_str_c000"])
        t5 = clips[0]
        self.assertEqual((t5["line"], t5["xa_id"], t5["audio_sectors"]), (80, "0x80", 8))
        self.assertEqual((t5["xa_payload_bytes"], t5["raw_bytes"]), (8 * 2304, 8 * 2352))
        self.assertEqual((t5["channel_sectors"], t5["channel_free"], t5["referenced"]), (8, "yes", "unreferenced"))
        t3 = clips[2]
        self.assertEqual(t3["channel_free"], "no (1 speech clip(s) remain)")
        self.assertEqual(t3["line"], 57)
        s = clips[3]
        self.assertEqual((s["reclaimable"], s["xa_payload_bytes"], s["referenced"]),
                         ("no (stereo movie track)", 0, "unchecked"))
        subs = [r for r in rep if r["row"] == "subtotal"]
        self.assertEqual([r["source_file"] for r in subs], ["GG/BGM/TALK05.XA", "GG/BGM/TALK03.XA", "GG/STR/ACED5.STR"])
        self.assertEqual(subs[0]["audio_sectors"], 16)
        self.assertEqual(subs[0]["channel_sectors"], 16)
        tot = rep[-1]
        self.assertEqual(tot["row"], "total")
        self.assertEqual(tot["audio_sectors"], 26)
        self.assertEqual(tot["xa_payload_bytes"], 26 * 2304)
        self.assertEqual(tot["channel_free"], "2 whole channel(s)")
        self.assertIn("3 unreferenced", tot["referenced"])

    def test_report_round_trip(self):
        import csv
        rep = R.build_report(self.manifest_rows(), {}, {})
        self.assertTrue(all(r["referenced"] == "unchecked" for r in rep if r["row"] == "clip"))
        with tempfile.TemporaryDirectory() as td:
            p = os.path.join(td, "r.tsv")
            R.write_report(p, rep)
            with open(p, encoding="utf-8", newline="") as f:
                rd = csv.DictReader(f, delimiter="\t")
                self.assertEqual(rd.fieldnames, R.COLUMNS)
                self.assertEqual(len(list(rd)), len(rep))


if __name__ == "__main__":
    unittest.main(verbosity=1)
