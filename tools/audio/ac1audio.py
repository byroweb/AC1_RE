#!/usr/bin/env python3
"""
ac1audio.py — shared helpers for the AC1 voice-line pipeline (tools/audio/).

Covers the bits every stage needs: where the local (never committed) audio
work dir is, how to find your disc / Java / jPSXdec, how to read a jPSXdec
.idx, and — the part that matters for re-injection — the XA sector arithmetic
that maps a PCM sample range back to the disc sectors it was decoded from.

XA-ADPCM facts used here (CD-ROM XA, Mode 2 Form 2, 2324-B payload):
  * one sector = 18 sound groups x 128 B; 4-bit coding packs 8 sound units of
    28 samples per group  -> 18*8*28 = 4032 samples per sector (mono),
    2016 sample-frames per sector (stereo); 8-bit coding halves both.
  * a stream's sectors are interleaved with the other channels of the same
    file: audio sector k of stream S lives at disc sector
        S.sector_first + k * S.stride
    (stride 16 for the mono TALK files, 8 for the stereo BGM / STR tracks).
  * jPSXdec's .idx lists each stream with its Sectors:first-last, Channel,
    Stereo?, Samples/Sec, Bits/Sample and Sector stride.

Nothing here reads the game disc except `disc_bin()` resolving its path.
"""
import glob
import json
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FILEMAP = os.path.join(ROOT, "disc_map", "disc_files.json")
# Local work dir for decoded audio. Gitignored (disc_map/extracted/ is derived game data).
AUDIO_DIR = os.environ.get("AC1_AUDIO_DIR") or os.path.join(ROOT, "disc_map", "extracted", "AUDIO")

SECTOR_RAW = 2352
XA_RATE_DEFAULT = 37800

# jPSXdec defaults: a per-user unpacked release under ~/dev/jpsxdec (Windows RE box
# layout, see tools/README.md) or ~/Applications/jpsxdec (the Ubuntu layout).
JPSXDEC_SEARCH = [
    os.path.join(os.path.expanduser("~"), "dev", "jpsxdec", "*", "jpsxdec.jar"),
    os.path.join(os.path.expanduser("~"), "dev", "jpsxdec", "jpsxdec.jar"),
    os.path.join(os.path.expanduser("~"), "Applications", "jpsxdec", "*", "jpsxdec.jar"),
    os.path.join(os.path.expanduser("~"), "Applications", "jpsxdec", "jpsxdec.jar"),
]


# Windows consoles default to cp1252; transcripts now carry Japanese, so make the
# standard streams UTF-8 (unencodable characters replaced) instead of crashing.
for _stream in (sys.stdout, sys.stderr):
    if hasattr(_stream, "reconfigure"):
        try:
            _stream.reconfigure(encoding="utf-8", errors="replace")
        except (ValueError, OSError):
            pass


def log(*a):
    print(*a, file=sys.stderr, flush=True)


def ensure_dir(p):
    os.makedirs(p, exist_ok=True)
    return p


def load_json(p, default=None):
    if not os.path.exists(p):
        if default is not None:
            return default
        sys.exit(f"missing {p} — run the earlier pipeline stage first")
    with open(p, encoding="utf-8") as f:
        return json.load(f)


def save_json(p, obj):
    ensure_dir(os.path.dirname(p))
    tmp = p + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(obj, f, indent=1)
    os.replace(tmp, p)


# ---------------------------------------------------------------- local tools
def disc_bin(explicit=None):
    """Your disc image: --bin, else the filemap's source_bin if set locally, else $AC1_BIN.
    The committed disc_files.json carries no path (bring your own disc)."""
    if explicit:
        p = explicit
    else:
        p = None
        if os.path.exists(FILEMAP):
            with open(FILEMAP) as f:
                p = json.load(f).get("source_bin")
        p = p or os.environ.get("AC1_BIN")
    if not p or not os.path.exists(p):
        sys.exit("disc image not found: set AC1_BIN to your SLUS-01323 v1.1 .bin (or pass --bin)")
    return p


def find_java():
    """java from $JAVA_HOME/bin, else PATH. jPSXdec v2.x runs on Temurin 21."""
    jh = os.environ.get("JAVA_HOME")
    if jh:
        for exe in ("java.exe", "java"):
            p = os.path.join(jh, "bin", exe)
            if os.path.exists(p):
                return p
    p = shutil.which("java")
    if not p:
        sys.exit("java not found: set JAVA_HOME (JDK 21) or put java on PATH")
    return p


def find_jpsxdec(explicit=None):
    """jpsxdec.jar: --jar, else $JPSXDEC_JAR, else the usual unpacked-release spots."""
    cands = [explicit, os.environ.get("JPSXDEC_JAR")]
    for pat in JPSXDEC_SEARCH:
        cands += sorted(glob.glob(pat))
    for c in cands:
        if c and os.path.exists(c):
            return c
    sys.exit("jpsxdec.jar not found: set JPSXDEC_JAR (see tools/README.md § jPSXdec)")


def run(cmd, **kw):
    log("  $", " ".join(f'"{c}"' if " " in c else c for c in cmd))
    return subprocess.run(cmd, check=True, **kw)


# ---------------------------------------------------------------- jPSXdec .idx
def parse_kv(line):
    """jPSXdec rows are pipe-delimited key:value (ID/Path may contain ':' and '/').
    Same rule as tools/extract/build_filemap.py."""
    d = {}
    for field in line.rstrip("\n").split("|"):
        if ":" in field:
            k, _, v = field.partition(":")
            d[k.strip()] = v.strip()
    return d


def parse_sectors(s):
    m = re.match(r"(\d+)-(\d+)", s or "")
    return (int(m.group(1)), int(m.group(2))) if m else (None, None)


def parse_idx(path):
    """Return the list of item dicts from a jPSXdec .idx (each with idx/id/type/
    sector_first/sector_last plus the raw key:value fields)."""
    items = []
    with open(path, "r", errors="replace") as f:
        for raw in f:
            if not raw.startswith("#:"):
                continue
            kv = parse_kv(raw)
            first, last = parse_sectors(kv.get("Sectors"))
            items.append({
                "idx": int(kv.get("#", -1)),
                "id": kv.get("ID", ""),
                "type": kv.get("Type", ""),
                "sector_first": first,
                "sector_last": last,
                "raw": kv,
            })
    return items


def build_index(bin_path, idx_path, jar=None, java=None):
    """java -jar jpsxdec.jar -f <bin> -x <idx>  (about 6 s for the AC1 disc)."""
    java = java or find_java()
    jar = jar or find_jpsxdec()
    ensure_dir(os.path.dirname(idx_path))
    # cwd = the work dir: jPSXdec writes index.log / save.log / debug00.log into cwd
    run([java, "-jar", jar, "-f", bin_path, "-x", os.path.abspath(idx_path)],
        cwd=os.path.dirname(os.path.abspath(idx_path)))
    if not os.path.exists(idx_path):
        sys.exit("jPSXdec did not write the index")
    return idx_path


# ---------------------------------------------------------------- XA streams
class XAStream:
    """One interleaved XA channel stream as jPSXdec indexes it.

    sector_first/sector_last are inclusive DISC sectors of the first/last audio
    sector of this stream; audio sector k sits at sector_first + k*stride.
    """

    def __init__(self, id, sector_first, sector_last, stride, channel=0,
                 stereo=False, rate=XA_RATE_DEFAULT, bits=4):
        self.id = id
        self.sector_first = int(sector_first)
        self.sector_last = int(sector_last)
        self.stride = int(stride)
        self.channel = int(channel)
        self.stereo = bool(stereo)
        self.rate = int(rate)
        self.bits = int(bits)
        span = self.sector_last - self.sector_first
        if span < 0 or span % self.stride:
            raise ValueError(f"{id}: sector span {span} not a multiple of stride {self.stride}")

    @classmethod
    def from_idx_item(cls, it):
        kv = it["raw"]
        return cls(it["id"], it["sector_first"], it["sector_last"],
                   kv.get("Sector stride", 1), kv.get("Channel", 0),
                   kv.get("Stereo?", "No") == "Yes", kv.get("Samples/Sec", XA_RATE_DEFAULT),
                   kv.get("Bits/Sample", 4))

    # --- sizes
    @property
    def n_sectors(self):
        return (self.sector_last - self.sector_first) // self.stride + 1

    @property
    def samples_per_sector(self):
        """PCM sample-frames decoded from one audio sector."""
        per = 4032 if self.bits == 4 else 2016
        return per // (2 if self.stereo else 1)

    @property
    def total_samples(self):
        return self.n_sectors * self.samples_per_sector

    @property
    def duration_s(self):
        return self.total_samples / self.rate

    # --- mapping
    def sector_of_sample(self, sample):
        """Disc sector holding sample-frame `sample` (0-based within the stream)."""
        if sample < 0 or sample >= self.total_samples:
            raise ValueError(f"sample {sample} outside stream of {self.total_samples}")
        return self.sector_first + (sample // self.samples_per_sector) * self.stride

    def audio_sector_index(self, sample):
        return sample // self.samples_per_sector

    def sector_range(self, start, end):
        """Inclusive disc-sector range covering sample-frames [start, end)."""
        if end <= start:
            raise ValueError("empty range")
        return (self.sector_of_sample(start), self.sector_of_sample(end - 1))

    def sample_range_of_sector(self, sector):
        """[start, end) sample-frames decoded from disc sector `sector`."""
        off = sector - self.sector_first
        if off < 0 or off % self.stride or sector > self.sector_last:
            raise ValueError(f"sector {sector} not in stream {self.id}")
        k = off // self.stride
        return (k * self.samples_per_sector, (k + 1) * self.samples_per_sector)

    def to_dict(self):
        return {
            "id": self.id, "sector_first": self.sector_first, "sector_last": self.sector_last,
            "stride": self.stride, "channel": self.channel, "stereo": self.stereo,
            "rate": self.rate, "bits": self.bits, "n_sectors": self.n_sectors,
            "samples_per_sector": self.samples_per_sector, "total_samples": self.total_samples,
        }

    @classmethod
    def from_dict(cls, d):
        return cls(d["id"], d["sector_first"], d["sector_last"], d["stride"], d["channel"],
                   d["stereo"], d["rate"], d["bits"])


def is_talk_item(it):
    return it["type"] == "XA" and re.match(r"GG/BGM/TALK\d\d\.XA\[\d+\]$", it["id"]) is not None


def is_str_audio_item(it):
    # jPSXdec v2.1 lists a video's audio as a child XA item: GG/STR/ACED1.STR[0.0]
    return it["type"] == "XA" and re.match(r"GG/STR/[A-Z0-9]+\.STR\[\d+\.\d+\]$", it["id"]) is not None


def stream_short_name(item_id):
    """GG/BGM/TALK03.XA[12] -> TALK03_ch12 ; GG/STR/ACED1.STR[0.0] -> ACED1_str"""
    m = re.match(r"GG/BGM/(TALK\d\d)\.XA\[(\d+)\]$", item_id)
    if m:
        return f"{m.group(1)}_ch{int(m.group(2)):02d}"
    m = re.match(r"GG/STR/([A-Z0-9]+)\.STR\[\d+\.\d+\]$", item_id)
    if m:
        return f"{m.group(1)}_str"
    return re.sub(r"[^A-Za-z0-9]+", "_", item_id).strip("_")


def source_kind(item_id):
    return "TALK" if "/TALK" in item_id else ("STR" if ".STR[" in item_id else "XA")
