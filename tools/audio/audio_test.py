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


if __name__ == "__main__":
    unittest.main(verbosity=1)
