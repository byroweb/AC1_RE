#!/usr/bin/env python3
"""
reinject_test.py - unit tests for the XA re-injection path (synthetic data, no disc).

  * xa_adpcm.edc / form2_edc - against known CRC vectors and a hand-built sector;
  * sound-group packing / unpacking - nibble placement and parameter copies;
  * the ADPCM codec - decoder on a hand-decodable unit, encoder round trips on
    silence, DC, a sine and noise, with the history carried across sectors;
  * build_form2_sector - subheader kept, spare bytes zeroed, EDC recomputed,
    bad input refused;
  * reinject_xa.Target - the manifest row -> disc sector arithmetic.

  tools/audio/.venv/Scripts/python tools/audio/reinject_test.py
"""
import os
import sys
import tempfile
import unittest

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import xa_adpcm as X  # noqa: E402
from reinject_xa import Target  # noqa: E402

SYNC = b"\x00" + b"\xff" * 10 + b"\x00"


def raw_sector(subheader, data, spare=b"\0" * 20, edc_ok=True, header=b"\x15\x58\x40\x02"):
    s = bytearray(SYNC + header + subheader + subheader + data + spare)
    e = X.edc(s[16:2348]) if edc_ok else 0
    s += e.to_bytes(4, "little")
    return bytes(s)


class EDC(unittest.TestCase):
    def test_table_vector(self):
        # Reflected CRC-32 with polynomial 0xD8018001, no init, no final xor:
        # the byte 0x80 shifts straight out as the polynomial, zeros stay zero,
        # and the function is linear over XOR.
        self.assertEqual(X.edc(b"\x80"), 0xD8018001)
        self.assertEqual(X.edc(b""), 0)
        self.assertEqual(X.edc(b"\x00" * 2332), 0)
        a = bytes(range(0, 64))
        b = bytes(range(64, 128))
        self.assertEqual(X.edc(bytes(x ^ y for x, y in zip(a, b))), X.edc(a) ^ X.edc(b))
        self.assertEqual(X.edc(a + b"\x00"), (X.edc(a) >> 8) ^ X._EDC_TABLE[X.edc(a) & 0xFF])

    def test_form2_edc_covers_subheader_and_user_data(self):
        sub = bytes([1, 3, 0x64, 0])
        data = bytes(range(256)) * 9
        s = raw_sector(sub, data)
        self.assertEqual(X.form2_edc(s), int.from_bytes(s[2348:], "little"))
        # a change anywhere in subheader or user data changes the EDC...
        s2 = bytearray(s)
        s2[17] = 4
        self.assertNotEqual(X.form2_edc(s2), X.form2_edc(s))
        s3 = bytearray(s)
        s3[2340] ^= 1  # inside the 20 spare bytes
        self.assertNotEqual(X.form2_edc(s3), X.form2_edc(s))
        # ...but the sync/header are not covered
        s4 = bytearray(s)
        s4[13] ^= 0xFF
        self.assertEqual(X.form2_edc(s4), X.form2_edc(s))


class Packing(unittest.TestCase):
    def test_group_roundtrip(self):
        rng = np.random.default_rng(1)
        units = [((f << 4) | sh, [int(v) for v in rng.integers(0, 16, 28)])
                 for f, sh in zip(rng.integers(0, 4, 8), rng.integers(0, 13, 8))]
        g = X.pack_group(units)
        self.assertEqual(len(g), 128)
        self.assertEqual(g[0:4], g[4:8])
        self.assertEqual(g[8:12], g[12:16])
        self.assertEqual(X.group_units(g), units)

    def test_nibble_placement(self):
        # unit 0 sample 0 -> low nibble of byte 0x10; unit 1 sample 0 -> high nibble;
        # unit 2 sample 0 -> low nibble of byte 0x11; unit 0 sample 1 -> byte 0x14.
        units = [(0, [0] * 28) for _ in range(8)]
        units[0] = (0, [0xA] + [0] * 27)
        units[1] = (0, [0xB] + [0] * 27)
        units[2] = (0, [0xC] + [0] * 27)
        units[7] = (0, [0] * 27 + [0xD])
        g = X.pack_group(units)
        self.assertEqual(g[0x10], 0xBA)
        self.assertEqual(g[0x11], 0x0C)
        self.assertEqual(g[0x10 + 4 * 27 + 3], 0xD0)

    def test_sector_sizes(self):
        self.assertEqual(X.SAMPLES_PER_SECTOR, 4032)
        d, _, _, _ = X.encode_sector_data(np.zeros(4032, np.int16))
        self.assertEqual(len(d), 2304)


class Codec(unittest.TestCase):
    def test_decode_unit_by_hand(self):
        # filter 0, shift 12 -> sample = nibble (sign-extended), no prediction
        s, p1, p2 = X.decode_unit(0x0C, [1, 15, 8, 7] + [0] * 24)
        self.assertEqual(s[:4], [1, -1, -8, 7])
        self.assertEqual((p1, p2), (0, 0))
        # filter 0, shift 0 -> nibble << 12
        s, _, _ = X.decode_unit(0x00, [1, 15, 7, 8] + [0] * 24)
        self.assertEqual(s[:4], [4096, -4096, 28672, -32768])
        # filter 1 (K0=60/64): zero nibbles decay the history
        s, _, _ = X.decode_unit(0x1C, [0] * 28, p1=6400, p2=0)
        self.assertEqual(s[0], 6000)
        self.assertEqual(s[1], (6000 * 60 + 32) >> 6)

    def test_decode_clamps(self):
        s, _, _ = X.decode_unit(0x10, [7] * 28)  # shift 0, filter 1: grows past 32767
        self.assertEqual(max(s), 32767)
        s, _, _ = X.decode_unit(0x10, [8] * 28)
        self.assertEqual(min(s), -32768)

    def test_silence_is_exact(self):
        x = np.zeros(4032 * 2, np.int16)
        d, p1, p2, err = X.encode_stream(x)
        self.assertEqual(err, 0)
        y, _, _ = X.decode_sectors(d)
        self.assertTrue(np.array_equal(y, x))

    def test_dc_and_small_values_exact(self):
        x = np.full(4032, 5, np.int16)
        x[100:200] = -7
        d, _, _, err = X.encode_stream(x)
        y, _, _ = X.decode_sectors(d)
        self.assertEqual(err, 0)
        self.assertTrue(np.array_equal(y, x))

    def test_sine_roundtrip_snr(self):
        n = 4032 * 3
        x = (12000 * np.sin(2 * np.pi * 440 * np.arange(n) / 37800)).astype(np.int16)
        d, _, _, _ = X.encode_stream(x)
        y, _, _ = X.decode_sectors(d)
        self.assertEqual(len(y), n)
        self.assertGreater(X.snr_db(x, y), 45.0)

    def test_noise_roundtrip_snr(self):
        rng = np.random.default_rng(7)
        x = rng.normal(0, 4000, 4032 * 2).clip(-32768, 32767).astype(np.int16)
        d, _, _, _ = X.encode_stream(x)
        y, _, _ = X.decode_sectors(d)
        self.assertGreater(X.snr_db(x, y), 15.0)  # white noise is the worst case for 4-bit ADPCM

    def test_history_carries_across_sectors(self):
        # Encoding two sectors in one go must equal encoding them one after the
        # other with the returned history - and differ from a cold restart.
        n = 4032 * 2
        x = (9000 * np.sin(2 * np.pi * 300 * np.arange(n) / 37800)).astype(np.int16)
        d_all, _, _, _ = X.encode_stream(x)
        d0, p1, p2, _ = X.encode_sector_data(x[:4032])
        d1, _, _, _ = X.encode_sector_data(x[4032:], p1, p2)
        self.assertEqual(d_all, [d0, d1])
        d1_cold, _, _, _ = X.encode_sector_data(x[4032:])
        self.assertNotEqual(d1, d1_cold)
        # and the decoder follows the same rule
        y_all, _, _ = X.decode_sectors(d_all)
        y0, q1, q2 = X.decode_sector_data(d0)
        y1, _, _ = X.decode_sector_data(d1, q1, q2)
        self.assertTrue(np.array_equal(y_all, np.concatenate([y0, y1])))

    def test_float_decoder_tracks_integer_decoder(self):
        # jPSXdec keeps the history as doubles; the two drift apart by a few
        # LSB (65 dB down on real speech) but decode the same signal
        n = 4032 * 2
        x = (12000 * np.sin(2 * np.pi * 440 * np.arange(n) / 37800)).astype(np.int16)
        d, _, _, _ = X.encode_stream(x)
        yi, _, _ = X.decode_sectors(d)
        yf, _, _ = X.decode_sectors_float(d)
        self.assertEqual(len(yf), n)
        self.assertGreater(X.snr_db(yi, yf), 55.0)
        self.assertGreater(X.snr_db(x, yf), 45.0)
        # silence and tiny values are exact in both
        z, _, _, _ = X.encode_stream(np.zeros(4032, np.int16))
        self.assertTrue(np.array_equal(X.decode_sectors_float(z)[0], np.zeros(4032, np.int16)))

    def test_stereo_decode_splits_even_odd_units(self):
        # even units carry L, odd units R, each with its own history
        t = np.arange(2016 * 2) / 37800
        left = (9000 * np.sin(2 * np.pi * 300 * t)).astype(np.int16)
        right = (6000 * np.sin(2 * np.pi * 1700 * t)).astype(np.int16)
        datas, h = [], [0, 0, 0, 0]
        for sec in range(2):
            groups = []
            for g in range(X.GROUPS_PER_SECTOR):
                units = []
                for u in range(4):
                    k = sec * 2016 + (g * 4 + u) * 28
                    pl, nl, h[0], h[1], _ = X.encode_unit(left[k:k + 28], h[0], h[1])
                    pr, nr, h[2], h[3], _ = X.encode_unit(right[k:k + 28], h[2], h[3])
                    units += [(pl, nl), (pr, nr)]
                groups.append(X.pack_group(units))
            datas.append(b"".join(groups))
        y = X.decode_stream_float(datas, stereo=True)
        self.assertEqual(len(y), 2 * len(left))
        self.assertGreater(X.snr_db(left, y[0::2]), 40.0)
        self.assertGreater(X.snr_db(right, y[1::2]), 40.0)
        # history runs on across sectors
        y0, hist = X.decode_sector_data_stereo_float(datas[0])
        y1, _ = X.decode_sector_data_stereo_float(datas[1], hist)
        self.assertTrue(np.array_equal(y, np.concatenate([y0, y1])))
        # mono path of decode_stream_float == decode_sectors_float
        self.assertTrue(np.array_equal(X.decode_stream_float(datas), X.decode_sectors_float(datas)[0]))

    def test_full_scale_does_not_crash(self):
        x = np.array([32767, -32768] * 2016, np.int16)
        d, _, _, _ = X.encode_stream(x)
        y, _, _ = X.decode_sectors(d)
        self.assertEqual(len(y), 4032)

    def test_pad_to_sectors(self):
        self.assertEqual(len(X.pad_to_sectors(np.zeros(10, np.int16))), 4032)
        self.assertEqual(len(X.pad_to_sectors(np.zeros(4032, np.int16))), 4032)
        self.assertEqual(len(X.pad_to_sectors(np.zeros(4033, np.int16))), 8064)
        self.assertEqual(len(X.pad_to_sectors(np.zeros(10, np.int16), 3)), 3 * 4032)
        with self.assertRaises(ValueError):
            X.pad_to_sectors(np.zeros(4033, np.int16), 1)


class Form2Sector(unittest.TestCase):
    def test_build_keeps_subheader_and_recomputes_edc(self):
        sub_last = bytes([1, 5, 0xE4, 0])  # EOF | RT | Form2 | Audio, as the disc's last sectors
        orig = raw_sector(sub_last, bytes([0x55]) * 2304, spare=bytes(range(20)))
        data = bytes([0xAA]) * 2304
        new = X.build_form2_sector(orig, data)
        self.assertEqual(len(new), 2352)
        self.assertEqual(new[:24], orig[:24])                       # sync, header, subheader
        self.assertEqual(new[24:2328], data)
        self.assertEqual(new[2328:2348], b"\0" * 20)                  # spare zeroed
        self.assertEqual(int.from_bytes(new[2348:], "little"), X.form2_edc(new))
        self.assertNotEqual(new[2348:], orig[2348:])
        self.assertEqual(X.subheader(new), (1, 5, 0xE4, 0))

    def test_build_refuses_bad_input(self):
        good = raw_sector(bytes([1, 0, 0x64, 0]), b"\0" * 2304)
        with self.assertRaises(ValueError):
            X.build_form2_sector(good[:-1], b"\0" * 2304)
        with self.assertRaises(ValueError):
            X.build_form2_sector(good, b"\0" * 2303)
        not_audio = raw_sector(bytes([1, 0, 0x08, 0]), b"\0" * 2304)   # data submode
        with self.assertRaises(ValueError):
            X.build_form2_sector(not_audio, b"\0" * 2304)
        broken = bytearray(good)
        broken[20] = 9  # subheader copy differs
        with self.assertRaises(ValueError):
            X.build_form2_sector(bytes(broken), b"\0" * 2304)

    def test_stream_sound_areas_picks_channel_audio(self):
        # one channel's Form 2 audio sectors, in disc order; other channels and
        # data sectors interleaved in the range are skipped
        secs = [raw_sector(bytes([1, 1, 0x64, 0]), bytes([1]) * 2304),
                raw_sector(bytes([1, 2, 0x64, 0]), bytes([2]) * 2304),
                raw_sector(bytes([1, 1, 0x08, 0]), bytes([3]) * 2304),    # data
                raw_sector(bytes([1, 1, 0x64, 0]), bytes([4]) * 2304)]
        fd, path = tempfile.mkstemp(suffix=".bin")
        try:
            with os.fdopen(fd, "wb") as f:
                f.write(bytes(2352) * 5 + b"".join(secs))
            self.assertEqual(X.stream_sound_areas(path, 5, 8, 1),
                             [bytes([1]) * 2304, bytes([4]) * 2304])
            self.assertEqual(X.stream_sound_areas(path, 6, 8, 2), [bytes([2]) * 2304])
            self.assertEqual(X.stream_sound_areas(path, 5, 5, 1), [bytes([1]) * 2304])
        finally:
            os.remove(path)

    def test_encode_decode_through_sector(self):
        x = (7000 * np.sin(2 * np.pi * 1000 * np.arange(4032) / 37800)).astype(np.int16)
        d, _, _, _ = X.encode_sector_data(x)
        s = X.build_form2_sector(raw_sector(bytes([1, 2, 0x64, 0]), b"\0" * 2304), d)
        y, _, _ = X.decode_sector_data(X.sector_sound_area(s))
        self.assertGreater(X.snr_db(x, y), 45.0)


class ManifestTarget(unittest.TestCase):
    ROW = {"clip_id": "TALK03_ch09_c002", "source_file": "GG/BGM/TALK03.XA", "stream": "TALK03_ch09",
           "channel": "9", "sector_first": "68629", "sector_last": "68725", "audio_sector_first": "58",
           "audio_sector_last": "64", "stride": "16", "rate": "37800"}

    def test_budget_and_sectors(self):
        t = Target(dict(self.ROW))
        self.assertEqual(t.n_sectors, 7)
        self.assertEqual(t.budget_samples, 7 * 4032)
        self.assertAlmostEqual(t.budget_s, 7 * 4032 / 37800)
        self.assertEqual(t.stream_first, 68629 - 16 * 58)
        self.assertEqual(t.target_sectors(), [68629 + 16 * k for k in range(7)])
        self.assertEqual(t.target_sectors()[-1], 68725)
        self.assertEqual(t.disc_sector(0), t.stream_first)

    def test_inconsistent_row_refused(self):
        bad = dict(self.ROW)
        bad["sector_last"] = "68726"
        with self.assertRaises(SystemExit):
            Target(bad)

    def test_str_row_refused(self):
        row = dict(self.ROW, source_file="GG/STR/ACED1.STR", stream="ACED1_str", stride="8")
        with self.assertRaises(SystemExit):
            Target(row)


if __name__ == "__main__":
    unittest.main(verbosity=1)
