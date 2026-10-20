"""
core/xa.py — XA-ADPCM preview decode for AC1mod (C hot loop).

Decodes an XA audio stream straight out of the disc image, with no jPSXdec /JVM
round-trip — so previewing audio is a few-ms in-process decode instead of a
~2.8s subprocess per item.

The decoder of record is tools/audio/xa_adpcm.py: stream_sound_areas() picks this
stream's Form 2 audio sectors (subheader channel + submode) and
decode_stream_float() decodes them. That pure-Python decode is slow, so the
serial ADPCM IIR also exists as a bit-identical C port (core/xa.c, auto-built to
libxa.dll/.so and called via ctypes); without a compiler we use the Python one,
and callers can fall back to jPSXdec if this raises.
"""
from __future__ import annotations
import os
import wave
import ctypes
import shutil
import subprocess
import numpy as np

from core import _tools  # noqa: F401  (puts tools/audio on sys.path)
import xa_adpcm

_HERE = os.path.dirname(os.path.abspath(__file__))
_SRC = os.path.join(_HERE, "xa.c")
_SO = os.path.join(_HERE, "libxa" + (".dll" if os.name == "nt" else ".so"))


# ----------------------------------------------------------- C lib loader -----

def _which(name):
    return shutil.which(name)          # honours PATHEXT (gcc.exe) on Windows


def _build_lib():
    cc = os.environ.get("CC") or _which("cc") or _which("gcc") or _which("clang")
    if not cc:
        return None
    # -ffp-contract=off: no FMA fusing, so the doubles match xa_adpcm.py exactly.
    tail = ["-O3", "-ffp-contract=off", "-fPIC", "-shared", _SRC, "-o", _SO, "-lm"]
    for extra in (["-march=native"], []):
        try:
            subprocess.run([cc] + extra + tail, check=True,
                           capture_output=True, timeout=60)
            return _SO
        except Exception:
            continue
    return None


_LIB = None
_LIB_TRIED = False
_u8 = np.ctypeslib.ndpointer(dtype=np.uint8, flags="C_CONTIGUOUS")
_i16 = np.ctypeslib.ndpointer(dtype=np.int16, flags="C_CONTIGUOUS")


def _lib():
    """Lazily build+load libxa. None if unavailable (use the Python decoder)."""
    global _LIB, _LIB_TRIED
    if _LIB is not None or _LIB_TRIED:
        return _LIB
    _LIB_TRIED = True
    so = _SO
    if not (os.path.exists(so) and os.path.getmtime(so) >= os.path.getmtime(_SRC)):
        so = _build_lib()
    if not so:
        return None
    try:
        lib = ctypes.CDLL(so)
        lib.xa_decode.restype = None
        lib.xa_decode.argtypes = [_u8, ctypes.c_int, ctypes.c_int, _i16]
        _LIB = lib
    except Exception:
        _LIB = None
    return _LIB


# --------------------------------------------------------------- decode -------

def decode_xa(bin_path, sector_start, sector_end, channel=0, stereo=True):
    """Decode an XA stream to interleaved int16 PCM.
    Returns (channels, samples_int16) or (channels, None) if no audio found."""
    areas = xa_adpcm.stream_sound_areas(bin_path, sector_start, sector_end, channel)
    channels = 2 if stereo else 1
    if not areas:
        return channels, None
    lib = _lib()
    if lib is None:
        return channels, xa_adpcm.decode_stream_float(areas, stereo)
    groups = np.frombuffer(b"".join(areas), np.uint8)
    n = len(groups) // xa_adpcm.GROUP_BYTES
    out = np.empty(n * xa_adpcm.UNITS_PER_GROUP * xa_adpcm.UNIT_SAMPLES, np.int16)
    lib.xa_decode(groups, n, channels, out)
    return channels, out


def decode_xa_to_wav(bin_path, sector_start, sector_end, out_path,
                     channel=0, stereo=True, sample_rate=37800):
    """Decode an XA stream and write a 16-bit PCM WAV. Returns out_path or None."""
    channels, pcm = decode_xa(bin_path, sector_start, sector_end, channel, stereo)
    if pcm is None:
        return None
    with wave.open(str(out_path), "wb") as wf:
        wf.setnchannels(channels)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        wf.writeframes(pcm.tobytes())
    return out_path
