#!/usr/bin/env python3
"""
xa_adpcm.py - CD-ROM XA-ADPCM codec + Mode 2 Form 2 sector packing, in Python.

The re-injection half of the voice pipeline (docs/AUDIO_PIPELINE.md section 6) needs
three things that nothing else in the repo had: an XA-ADPCM *encoder*, the
Form 2 sector layout, and the CD-ROM EDC. All three live here as pure
functions over bytes/numpy so they can be unit-tested without a disc
(tools/audio/reinject_test.py); tools/audio/reinject_xa.py is the CLI.

XA-ADPCM, 4-bit (the only mode AC1 uses for speech; CONFIRMED on the disc):
  * a sector's 2304-byte sound area is 18 sound groups x 128 bytes;
  * a group holds 8 sound units of 28 samples. Mono: units 0..7 are consecutive
    in time. Stereo: even units are L, odd are R, each side with its own
    history (TALK is mono; the STR movie soundtracks are stereo).
  * group layout:  0x00-0x03 sound parameters of units 0..3   0x04-0x07 copy
                   0x08-0x0B sound parameters of units 4..7   0x0C-0x0F copy
                   0x10-0x7F 112 data bytes: byte 0x10 + 4*i + j holds sample i
                             of unit 2j (low nibble) and unit 2j+1 (high nibble)
  * sound parameter byte: bits 0-3 = shift s (0..12), bits 4-5 = filter f.
  * decode: t = sign-extended nibble
            s_n = (t << (12 - shift)) + ((s_n-1*K0[f] + s_n-2*K1[f] + 32) >> 6)
            clamped to int16; K0 = {0, 60, 115, 98}, K1 = {0, 0, -52, -55}.
    The two-sample history runs on across units AND sectors of a stream.
  * mono 4-bit: 18*8*28 = 4032 samples per sector (37800 Hz -> 0.1067 s).

Mode 2 Form 2 sector (2352 B):  12 sync | 4 header | 8 subheader (file, channel,
submode, coding; repeated) | 2324 user data (2304 ADPCM + 20 unused, zero on
this disc) | 4 EDC. The EDC is the CD-ROM EDC (CRC-32 with the reflected
polynomial 0xD8018001) over bytes 16..2347 = subheader + user data, stored
little-endian. Verified byte-exact against the TALK sectors of SLUS-01323.

Two decoders: `decode_*` is the integer formula from the PSX docs (nocash
psx-spx, what emulators compute, and the model the encoder's feedback loop
uses); `decode_*_float` keeps the history as unrounded doubles and rounds only
the output, which is what jPSXdec v2.1 does - CONFIRMED against its WAVs of
four TALK channels: bit-exact on the short one, within 1 LSB on ~0.01% of the
samples of the long ones (its exact rounding rule was not pinned down). The
integer one differs from jPSXdec by up to ~14 LSB on loud speech, 65 dB down.
Use the float one to predict jPSXdec's output. Over the whole v1.1 disc (96 TALK
channels + the 5 stereo ACED endings, 43.8M samples) it is off by 1 LSB on 3243
samples (0.007%); rounding half-to-even instead of half-up gives 3155, so neither
is jPSXdec's exact rule and half-up stays.

stream_sound_areas() picks one XA channel's Form 2 audio sectors out of a sector
range of a raw image (what one jPSXdec XA item decodes); decode_stream_float()
decodes them, mono or stereo. tools/ac1mod/core/xa.py feeds the same sound areas
to a C port of decode_stream_float for in-GUI preview.
"""
import math

import numpy as np

SECTOR_RAW = 2352
SUBHEADER_OFF = 16
DATA_OFF = 24
ADPCM_BYTES = 2304
USER_DATA = 2324          # Form 2 user data: 2304 ADPCM + 20 spare
EDC_OFF = 2348
GROUP_BYTES = 128
GROUPS_PER_SECTOR = 18
UNITS_PER_GROUP = 8
UNIT_SAMPLES = 28
SAMPLES_PER_SECTOR = GROUPS_PER_SECTOR * UNITS_PER_GROUP * UNIT_SAMPLES   # 4032 (mono 4-bit)

K0 = (0, 60, 115, 98)
K1 = (0, 0, -52, -55)

# submode bits
SM_EOR = 0x01
SM_VIDEO = 0x02
SM_AUDIO = 0x04
SM_DATA = 0x08
SM_TRIGGER = 0x10
SM_FORM2 = 0x20
SM_REALTIME = 0x40
SM_EOF = 0x80


# ------------------------------------------------------------------ EDC
def _make_edc_table():
    tab = []
    for i in range(256):
        e = i
        for _ in range(8):
            e = (e >> 1) ^ (0xD8018001 if e & 1 else 0)
        tab.append(e)
    return tab


_EDC_TABLE = _make_edc_table()


def edc(data):
    """CD-ROM EDC of `data` (bytes). For Form 2 pass sector[16:2348]."""
    e = 0
    for b in data:
        e = _EDC_TABLE[(e ^ b) & 0xFF] ^ (e >> 8)
    return e


def form2_edc(sector):
    """EDC a Mode 2 Form 2 raw sector should carry at bytes 2348..2351."""
    return edc(sector[SUBHEADER_OFF:EDC_OFF])


# ------------------------------------------------------------------ decoder
def _signed4(n):
    return n - 16 if n & 8 else n


def decode_unit(param, nibbles, p1=0, p2=0):
    """Decode one 28-sample unit. nibbles: 28 ints 0..15. Returns (samples, p1, p2)."""
    shift = 12 - (param & 0x0F)
    f = (param >> 4) & 3
    k0, k1 = K0[f], K1[f]
    out = []
    for n in nibbles:
        t = _signed4(n)
        s = (t << shift) + ((p1 * k0 + p2 * k1 + 32) >> 6)
        s = -32768 if s < -32768 else (32767 if s > 32767 else s)
        out.append(s)
        p2, p1 = p1, s
    return out, p1, p2


def decode_unit_float(param, nibbles, p1=0.0, p2=0.0):
    """jPSXdec-style decode of one unit: double history, Math.round on output."""
    shift = 12 - (param & 0x0F)
    f = (param >> 4) & 3
    k0, k1 = K0[f] / 64.0, K1[f] / 64.0
    out = []
    for n in nibbles:
        s = float(_signed4(n) << shift) + p1 * k0 + p2 * k1
        v = int(math.floor(s + 0.5))
        out.append(-32768 if v < -32768 else (32767 if v > 32767 else v))
        p2, p1 = p1, s
    return out, p1, p2


def group_units(group):
    """Split a 128-byte sound group into [(param, [28 nibbles]) for unit 0..7] (4-bit)."""
    assert len(group) == GROUP_BYTES
    params = list(group[0:4]) + list(group[8:12])
    units = []
    for u in range(UNITS_PER_GROUP):
        j, hi = divmod(u, 2)
        nib = []
        for i in range(UNIT_SAMPLES):
            b = group[16 + 4 * i + j]
            nib.append((b >> 4) & 0xF if hi else b & 0xF)
        units.append((params[u], nib))
    return units


def decode_sector_data(data, p1=0, p2=0):
    """Decode the 2304-byte sound area of one mono 4-bit sector.
    Returns (np.int16[4032], p1, p2) with the history carried for the next sector."""
    assert len(data) >= ADPCM_BYTES
    out = []
    for g in range(GROUPS_PER_SECTOR):
        for param, nib in group_units(data[g * GROUP_BYTES:(g + 1) * GROUP_BYTES]):
            s, p1, p2 = decode_unit(param, nib, p1, p2)
            out += s
    return np.asarray(out, dtype=np.int16), p1, p2


def decode_sectors(sector_datas, p1=0, p2=0):
    """Decode a sequence of 2304-byte sound areas (one stream, in order)."""
    parts = []
    for d in sector_datas:
        s, p1, p2 = decode_sector_data(d, p1, p2)
        parts.append(s)
    return (np.concatenate(parts) if parts else np.zeros(0, np.int16)), p1, p2


def decode_sector_data_float(data, p1=0.0, p2=0.0):
    """decode_sector_data with jPSXdec's double-precision history."""
    assert len(data) >= ADPCM_BYTES
    out = []
    for g in range(GROUPS_PER_SECTOR):
        for param, nib in group_units(data[g * GROUP_BYTES:(g + 1) * GROUP_BYTES]):
            s, p1, p2 = decode_unit_float(param, nib, p1, p2)
            out += s
    return np.asarray(out, dtype=np.int16), p1, p2


def decode_sectors_float(sector_datas, p1=0.0, p2=0.0):
    """decode_sectors with jPSXdec's double-precision history (bit-exact with its WAVs)."""
    parts = []
    for d in sector_datas:
        s, p1, p2 = decode_sector_data_float(d, p1, p2)
        parts.append(s)
    return (np.concatenate(parts) if parts else np.zeros(0, np.int16)), p1, p2


def decode_sector_data_stereo_float(data, hist=(0.0, 0.0, 0.0, 0.0)):
    """Decode the 2304-byte sound area of one STEREO 4-bit sector (jPSXdec-style
    double history). Even units are L, odd units R; hist = (l1, l2, r1, r2).
    Returns (np.int16[2 * 2016] interleaved L,R, hist)."""
    assert len(data) >= ADPCM_BYTES
    l1, l2, r1, r2 = hist
    left, right = [], []
    for g in range(GROUPS_PER_SECTOR):
        units = group_units(data[g * GROUP_BYTES:(g + 1) * GROUP_BYTES])
        for u in range(0, UNITS_PER_GROUP, 2):
            s, l1, l2 = decode_unit_float(*units[u], l1, l2)
            left += s
            s, r1, r2 = decode_unit_float(*units[u + 1], r1, r2)
            right += s
    out = np.empty(2 * len(left), np.int16)
    out[0::2] = left
    out[1::2] = right
    return out, (l1, l2, r1, r2)


def decode_stream_float(sector_datas, stereo=False):
    """Decode one stream's sound areas (e.g. from stream_sound_areas) with the
    jPSXdec-model float decoder. Returns np.int16 PCM, interleaved L,R if stereo."""
    parts = []
    if stereo:
        h = (0.0, 0.0, 0.0, 0.0)
        for d in sector_datas:
            s, h = decode_sector_data_stereo_float(d, h)
            parts.append(s)
    else:
        p1 = p2 = 0.0
        for d in sector_datas:
            s, p1, p2 = decode_sector_data_float(d, p1, p2)
            parts.append(s)
    return np.concatenate(parts) if parts else np.zeros(0, np.int16)


# ------------------------------------------------------------------ encoder
# candidate lanes: every (filter, shift) pair, searched exhaustively per unit
_FILT = np.repeat(np.arange(4), 13)                       # 52 lanes
_SHIFT = np.tile(np.arange(13), 4)
_K0 = np.array(K0, np.int64)[_FILT]
_K1 = np.array(K1, np.int64)[_FILT]
_SH = 12 - _SHIFT                                         # bits the nibble is shifted left
_HALF = np.where(_SH > 0, (1 << _SH) >> 1, 0)


def encode_unit(x, p1=0, p2=0):
    """Encode 28 samples (ints) given the decoder's history. Tries all 52
    (filter, shift) pairs with exact decoder feedback and keeps the least
    squared error. Returns (param, nibbles[28] as ints 0..15, p1, p2, sq_err)."""
    x = np.asarray(x, np.int64)
    assert x.shape == (UNIT_SAMPLES,)
    n = len(_FILT)
    a = np.full(n, int(p1), np.int64)
    b = np.full(n, int(p2), np.int64)
    err = np.zeros(n, np.int64)
    q = np.zeros((UNIT_SAMPLES, n), np.int64)
    for i in range(UNIT_SAMPLES):
        pred = (a * _K0 + b * _K1 + 32) >> 6
        r = x[i] - pred
        qi = (r + _HALF) >> _SH
        np.clip(qi, -8, 7, out=qi)
        rec = np.clip((qi << _SH) + pred, -32768, 32767)
        err += (rec - x[i]) ** 2
        q[i] = qi
        b, a = a, rec
    best = int(np.argmin(err))
    param = (int(_FILT[best]) << 4) | int(_SHIFT[best])
    nibbles = [int(v) & 0xF for v in q[:, best]]
    return param, nibbles, int(a[best]), int(b[best]), int(err[best])


def pack_group(units):
    """units: 8 x (param, [28 nibbles]) -> 128-byte sound group (mono 4-bit)."""
    assert len(units) == UNITS_PER_GROUP
    g = bytearray(GROUP_BYTES)
    params = [p for p, _ in units]
    g[0:4] = g[4:8] = bytes(params[0:4])
    g[8:12] = g[12:16] = bytes(params[4:8])
    for u, (_, nib) in enumerate(units):
        j, hi = divmod(u, 2)
        for i in range(UNIT_SAMPLES):
            v = nib[i] & 0xF
            g[16 + 4 * i + j] |= (v << 4) if hi else v
    return bytes(g)


def encode_sector_data(samples, p1=0, p2=0):
    """Encode exactly 4032 int16 samples into a 2304-byte sound area.
    Returns (bytes, p1, p2, sq_err)."""
    x = np.asarray(samples, np.int64)
    assert x.shape == (SAMPLES_PER_SECTOR,), x.shape
    groups = []
    toterr = 0
    for g in range(GROUPS_PER_SECTOR):
        units = []
        for u in range(UNITS_PER_GROUP):
            k = (g * UNITS_PER_GROUP + u) * UNIT_SAMPLES
            param, nib, p1, p2, e = encode_unit(x[k:k + UNIT_SAMPLES], p1, p2)
            units.append((param, nib))
            toterr += e
        groups.append(pack_group(units))
    return b"".join(groups), p1, p2, toterr


def pad_to_sectors(samples, n_sectors=None):
    """Zero-pad int16 samples to a whole number of sectors (at least n_sectors)."""
    x = np.asarray(samples, np.int16)
    need = -(-len(x) // SAMPLES_PER_SECTOR)
    if n_sectors is not None:
        if need > n_sectors:
            raise ValueError(f"{len(x)} samples need {need} sectors, budget is {n_sectors}")
        need = n_sectors
    return np.concatenate([x, np.zeros(need * SAMPLES_PER_SECTOR - len(x), np.int16)])


def encode_stream(samples, p1=0, p2=0, progress=None):
    """Encode int16 samples (a multiple of 4032 long; see pad_to_sectors) into a
    list of 2304-byte sound areas. Returns (datas, p1, p2, total_sq_err)."""
    x = np.asarray(samples, np.int16)
    if len(x) % SAMPLES_PER_SECTOR:
        raise ValueError("sample count is not a multiple of 4032 - pad first")
    datas = []
    toterr = 0
    n = len(x) // SAMPLES_PER_SECTOR
    for k in range(n):
        d, p1, p2, e = encode_sector_data(x[k * SAMPLES_PER_SECTOR:(k + 1) * SAMPLES_PER_SECTOR], p1, p2)
        datas.append(d)
        toterr += e
        if progress:
            progress(k + 1, n)
    return datas, p1, p2, toterr


# ------------------------------------------------------------------ sectors
def build_form2_sector(original, data, spare=b"\0" * 20):
    """New raw sector = original's sync/header/subheader + 2304 ADPCM bytes +
    20 spare bytes + fresh EDC. The subheader (and so the EOF/EOR bits of a
    stream's last sector) is copied verbatim from `original`."""
    if len(original) != SECTOR_RAW:
        raise ValueError(f"raw sector must be {SECTOR_RAW} bytes, got {len(original)}")
    if len(data) != ADPCM_BYTES:
        raise ValueError(f"sound area must be {ADPCM_BYTES} bytes, got {len(data)}")
    if len(spare) != USER_DATA - ADPCM_BYTES:
        raise ValueError("spare must be 20 bytes")
    sub = original[SUBHEADER_OFF:DATA_OFF]
    if sub[0:4] != sub[4:8]:
        raise ValueError(f"subheader copy mismatch {sub.hex()} - not a Mode 2 sector?")
    if not (sub[2] & SM_FORM2) or not (sub[2] & SM_AUDIO):
        raise ValueError(f"submode {sub[2]:#04x} is not Form 2 audio")
    s = bytearray(original[:DATA_OFF]) + bytearray(data) + bytearray(spare)
    s += form2_edc(s).to_bytes(4, "little")
    assert len(s) == SECTOR_RAW
    return bytes(s)


def subheader(sector):
    """(file, channel, submode, coding) of a raw Mode 2 sector."""
    sub = sector[SUBHEADER_OFF:DATA_OFF]
    return sub[0], sub[1], sub[2], sub[3]


def sector_sound_area(sector):
    return sector[DATA_OFF:DATA_OFF + ADPCM_BYTES]


def stream_sound_areas(bin_path, sector_first, sector_last, channel):
    """Sound areas (2304 B each, disc order) of XA `channel`'s Form 2 audio
    sectors within sector_first..sector_last of a raw 2352-byte image: the
    sectors one jPSXdec XA item decodes. Other channels' sectors and the video /
    data sectors interleaved in the range are skipped."""
    with open(bin_path, "rb") as f:
        f.seek(sector_first * SECTOR_RAW)
        raw = f.read((sector_last - sector_first + 1) * SECTOR_RAW)
    n = len(raw) // SECTOR_RAW
    sec = np.frombuffer(raw, np.uint8, count=n * SECTOR_RAW).reshape(n, SECTOR_RAW)
    sm = sec[:, SUBHEADER_OFF + 2]
    keep = (sec[:, SUBHEADER_OFF + 1] == channel) & ((sm & SM_AUDIO) != 0) & ((sm & SM_FORM2) != 0)
    return [row.tobytes() for row in sec[keep, DATA_OFF:DATA_OFF + ADPCM_BYTES]]


def snr_db(reference, test):
    """Signal-to-noise ratio of `test` against `reference` (int16 arrays, same length)."""
    r = np.asarray(reference, np.float64)
    t = np.asarray(test, np.float64)
    noise = np.sum((r - t) ** 2)
    sig = np.sum(r ** 2)
    if noise == 0:
        return float("inf")
    if sig == 0:
        return float("-inf")
    return 10.0 * np.log10(sig / noise)
