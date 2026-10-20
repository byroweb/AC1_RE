"""
Wrapper around the jPSXdec JAR for subprocess calls.
All output paths are relative to the caller's working directory.
"""
import os
import subprocess
import re
import struct
import math
import logging
import numpy as np
from pathlib import Path
from dataclasses import dataclass, field
from typing import Optional

log = logging.getLogger("ac1mod")


def _find_jar() -> Path:
    """$JPSXDEC_JAR, else a jpsxdec/ folder next to the app (the old AC1mod
    layout), else a per-user unpacked release under ~/dev/jpsxdec (Windows RE box)
    or ~/Applications/jpsxdec (Ubuntu) — the same search as tools/audio/ac1audio.py."""
    env = os.environ.get("JPSXDEC_JAR")
    if env:
        return Path(env)
    home = Path.home()
    cands = [Path(__file__).parent.parent / "jpsxdec" / "jpsxdec.jar"]
    for root in (home / "dev" / "jpsxdec", home / "Applications" / "jpsxdec"):
        cands += sorted(root.glob("*/jpsxdec.jar")) + [root / "jpsxdec.jar"]
    return next((c for c in cands if c.exists()), cands[0])


def _find_java() -> str:
    """java from $JAVA_HOME/bin, else PATH. jPSXdec v2.x runs on Temurin 21."""
    jh = os.environ.get("JAVA_HOME")
    if jh:
        for name in ("java.exe", "java"):
            p = Path(jh) / "bin" / name
            if p.exists():
                return str(p)
    return "java"


JPSXDEC_JAR = _find_jar()
JAVA = _find_java()


def _run(args: list[str], cwd: Path = None) -> tuple[int, str, str]:
    """Run jPSXdec with given args. If cwd is None, use repository root.

    Returns (returncode, stdout, stderr).
    """
    # Ensure cwd is a Path or None
    repo_root = Path(__file__).parent.parent.resolve()
    if cwd is None:
        cwd = repo_root
    else:
        cwd = Path(cwd).resolve()

    cmd = [JAVA, "-jar", str(JPSXDEC_JAR)] + args
    log.debug(f"Running jpsxdec: {' '.join(cmd)}  cwd={cwd}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=str(cwd))
    except FileNotFoundError as e:
        log.error("Java not found: %s", e)
        return 1, "", str(e)

    if result.returncode != 0:
        log.warning("jPSXdec returned %s: stdout=%s stderr=%s", result.returncode, result.stdout[:200], result.stderr[:200])
    else:
        log.debug("jPSXdec finished: stdout=%s", result.stdout[:200])
    return result.returncode, result.stdout, result.stderr


def build_index(bin_path: Path, index_path: Path) -> tuple[bool, str]:
    """Run jPSXdec -f <bin> -x <index> to build the index file."""
    rc, out, err = _run(["-f", str(bin_path), "-x", str(index_path)])
    return rc == 0, err or out


def extract_all(index_path: Path, out_dir: Path) -> tuple[bool, str]:
    """
    Extract every item in the index to out_dir.
    Uses -x <index> -a to process all items.
    Images are saved as PNG. Files extracted as-is.
    """
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    # Remove existing .tim/.png files to enforce overwrite policy
    try:
        for p in out_dir.rglob("*.png"):
            try:
                p.unlink()
            except OSError:
                log.debug("Could not remove %s", p)
        for p in out_dir.rglob("*.tim"):
            try:
                p.unlink()
            except OSError:
                log.debug("Could not remove %s", p)
    except Exception:
        # keep going even if cleanup fails
        log.debug("Cleanup of existing images failed or skipped")

    # Run jPSXdec from repository root and let the -dir option control output
    # Process only image items so TIM/Images are decoded
    # Request saving original TIM files for image items (no decoding)
    rc, out, err = _run([
        "-x", str(index_path), "-a", "image", "-imgfmt", "tim", "-dir", str(out_dir)
    ])
    return rc == 0, err or out


def replace_tim(index_path: Path, item_number: int, new_tim_path: Path, bin_path: Path) -> tuple[bool, str]:
    """
    Replace a TIM image in the BIN using jPSXdec's -replacetim flag.
    jPSXdec writes directly into the BIN file at bin_path.
    """
    rc, out, err = _run([
        "-x", str(index_path),
        "-i", str(item_number),
        "-replacetim", str(new_tim_path),
    ])
    return rc == 0, err or out


def save_tim_for_item(index_path: Path, item_number: int, out_dir: Path) -> tuple[bool, str]:
    """Invoke jPSXdec to save the TIM for a single index item number into out_dir.

    Returns (ok, output_message).
    """
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    rc, out, err = _run([
        "-x", str(index_path),
        "-i", str(item_number),
        "-imgfmt", "tim",
        "-dir", str(out_dir),
    ])
    return rc == 0, err or out


# ---------------------------------------------------------------------------
# Index file parser
# ---------------------------------------------------------------------------

@dataclass
class IndexEntry:
    number: int
    name: str
    full_id: str           # full jPSXdec ID e.g. "GG/COM/RTIM.T[0]"
    entry_type: str        # "File", "Image", "Video", "Audio"
    sector_start: int
    sector_end: int
    details: str           # raw details string
    parent_name: Optional[str] = None

    # Byte size as reported by jPSXdec's index (File items) or computed for
    # synthetic container children. None when jPSXdec gives no size (Tim/XA/Video,
    # which it describes by dimensions/duration instead).
    size_bytes: Optional[int] = None

    # Set on synthetic children of a custom .T container (e.g. FDAT.T) so they
    # are read as an in-memory slice of the cached container instead of a file.
    container_index: Optional[int] = None

    # Parsed image fields (only for Image type)
    width: Optional[int] = None
    height: Optional[int] = None
    palette_count: Optional[int] = None

    # Parsed audio fields (only for Audio type)
    audio_sample_rate: Optional[int] = None
    audio_channels: Optional[int] = None
    audio_xa_channel: Optional[int] = None   # CD subheader channel (XA interleave)
    audio_xa_file: Optional[int] = None      # CD subheader file number
    audio_sector_stride: Optional[int] = None  # disc sectors between this channel's sectors

    # Paths set after extraction
    png_path: Optional[Path] = None
    tim_path: Optional[Path] = None
    wav_path: Optional[Path] = None

    # Multiple extracted variants (palette/frame variants)
    png_paths: list[Path] = field(default_factory=list)
    tim_paths: list[Path] = field(default_factory=list)
    wav_paths: list[Path] = field(default_factory=list)

    # Replacement state
    replacement_gif: Optional[Path] = None
    replacement_valid: Optional[bool] = None
    validation_errors: list[str] = field(default_factory=list)

    @property
    def is_image(self) -> bool:
        return self.entry_type == "Image"

    @property
    def is_audio(self) -> bool:
        return self.entry_type == "Audio"

    @property
    def display_name(self) -> str:
        return self.name

    @property
    def indent_level(self) -> int:
        return 1 if re.search(r'\[\d+\]', self.name) else 0


def parse_index_file(index_path: Path) -> list[IndexEntry]:
    """
    Parse jPSXdec's .idx file into a list of IndexEntry objects.

    jPSXdec v2.x writes key:value pipe-delimited lines like:
      #:330|ID:GG/COM/RTIM.T[0]|Sectors:85348-85364|Type:Tim|Dimensions:128x256|Palettes:1
      #:4|ID:GG/BGM/BGM00.XA|Sectors:124-22795|Type:File|Size:46432256|Path:...|...
      #:2579|ID:GG/STR/ACED1.STR[0]|Sectors:...|Type:Video|Dimensions:320x240|...

    TIM image types appear as Type:Tim (not Type:Image).
    The ID field contains the full path including directory prefix.
    """
    entries = []
    if not index_path.exists():
        return entries

    current_parent = None  # most recent non-child entry name

    with open(index_path, "r", errors="replace") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("Log") or line.startswith("jPSXdec"):
                continue

            # Parse all key:value fields from the pipe-delimited line
            fields: dict[str, str] = {}
            for part in line.split("|"):
                part = part.strip()
                if ":" in part:
                    k, _, v = part.partition(":")
                    fields[k.strip()] = v.strip()

            # Must have at minimum # and ID and Type
            if "#" not in fields or "ID" not in fields or "Type" not in fields:
                continue

            try:
                number = int(fields["#"])
            except ValueError:
                continue

            full_id = fields["ID"]          # e.g. "GG/COM/RTIM.T[8]"
            # Use just the filename portion as display name
            name = full_id.split("/")[-1]   # e.g. "RTIM.T[8]"

            raw_type = fields["Type"]       # File, Tim, XA, Video, etc.

            # Normalise type for our purposes
            if raw_type == "Tim":
                entry_type = "Image"
            elif raw_type in ("XA", "SPU", "Audio", "ADPCM", "IKI_Audio"):
                entry_type = "Audio"
            elif raw_type == "Video":
                entry_type = "Video"
            else:
                entry_type = "File"

            # Sectors: "85348-85364"
            sector_start = sector_end = 0
            sectors_str = fields.get("Sectors", "")
            m = re.match(r'(\d+)-(\d+)', sectors_str)
            if m:
                sector_start = int(m.group(1))
                sector_end = int(m.group(2))

            # Build a details string for display
            details = fields.get("Size", "")
            size_bytes = int(details) if details.isdigit() else None

            # Determine parent — child items have bracketed suffix like [0] or [0.0]
            # Use base name (strip bracket suffix) so parent lookup is robust
            is_child = bool(re.search(r'\[[\d.]+\]', name))
            base_name = re.sub(r'\[[\d.]+\].*$', '', name)
            if is_child:
                parent = base_name
            else:
                parent = None
                current_parent = name

            entry = IndexEntry(
                number=number,
                name=name,
                full_id=full_id,
                entry_type=entry_type,
                sector_start=sector_start,
                sector_end=sector_end,
                details=details,
                parent_name=parent,
                size_bytes=size_bytes,
            )

            # Parse image dimensions for TIM entries
            if entry_type == "Image":
                dim = fields.get("Dimensions", "")
                m = re.match(r'(\d+)x(\d+)', dim)
                if m:
                    entry.width = int(m.group(1))
                    entry.height = int(m.group(2))
                pal = fields.get("Palettes", "")
                if pal.isdigit():
                    entry.palette_count = int(pal)

            # Parse audio metadata where jPSXdec provides it. AC1's XA lines look
            # like: |File:1|Channel:0|Stereo?:No|Samples/Sec:37800|Bits/Sample:4|Sector stride:16
            if entry_type == "Audio":
                for key in ("Samples/Sec", "SampleRate", "SamplesPerSecond", "Freq"):
                    if key in fields and fields[key].isdigit():
                        entry.audio_sample_rate = int(fields[key])
                        break
                if "Stereo?" in fields:
                    entry.audio_channels = 2 if fields["Stereo?"].lower() == "yes" else 1
                else:
                    for key in ("Channels", "Ch"):
                        if key in fields and fields[key].isdigit():
                            entry.audio_channels = int(fields[key])
                            break
                # CD subheader channel number — which interleaved stream this is.
                if "Channel" in fields and fields["Channel"].lstrip("-").isdigit():
                    entry.audio_xa_channel = int(fields["Channel"])
                if fields.get("File", "").isdigit():
                    entry.audio_xa_file = int(fields["File"])
                if fields.get("Sector stride", "").isdigit():
                    entry.audio_sector_stride = int(fields["Sector stride"])

            entries.append(entry)

    return entries


# ---------------------------------------------------------------------------
# TIM file header parser  (Sony Psy-Q File Formats spec, Chapter 3)
# ---------------------------------------------------------------------------

@dataclass
class TimInfo:
    bpp: int               # 4, 8, 16, or 24
    has_clut: bool
    clut_x: int
    clut_y: int
    clut_width: int        # number of CLUT entries per row
    clut_height: int       # number of CLUT rows (palettes)
    pixel_x: int           # VRAM destination X
    pixel_y: int           # VRAM destination Y
    pixel_width: int       # stored width in VRAM 16-bit words
    pixel_height: int
    image_width: int       # actual pixel width (accounts for bpp packing)
    image_height: int
    file_size: int
    clut_words: tuple = ()  # the CLUT's u16 entries, all rows (encode_to_tim keeps their STP bits)


def decode_tim_to_rgba(tim_path: Path, palette_index: int = 0) -> Optional[tuple[int, int, bytes]]:
    """Decode a TIM file to raw RGBA8888 bytes.

    Returns (width, height, rgba_bytes) or None on failure.
    For 4/8bpp with no embedded CLUT, falls back to grayscale.
    palette_index selects which CLUT row for multi-palette TIMs.

    Fast path: the per-pixel work is vectorised with numpy (~30x faster than
    the scalar loop, byte-identical output). Anything the fast path can't
    safely handle — truncated pixel data, malformed headers — falls back to
    the scalar decoder below, which remains the source of truth.
    """
    try:
        data = tim_path.read_bytes()
    except OSError:
        return None
    fast = _decode_tim_numpy(data, palette_index)
    if fast is not None:
        return fast
    return _decode_tim_scalar(data, palette_index)


def _decode_tim_numpy(data: bytes, palette_index: int = 0) -> Optional[tuple[int, int, bytes]]:
    """Vectorised TIM decode. Returns None (deferring to the scalar decoder)
    on any header anomaly or truncated pixel region, so callers always get the
    proven scalar result in edge cases."""
    if len(data) < 12 or data[0] != 0x10 or data[1] != 0x00:
        return None

    flag = struct.unpack_from("<I", data, 4)[0]
    bpp = {0: 4, 1: 8, 2: 16, 3: 24}.get(flag & 0x03, 0)
    has_clut = bool(flag & 0x08)
    if bpp == 0:
        return None

    offset = 8
    clut = None                  # np.uint8 array (N, 4) or None
    entries_per_palette = 0
    if has_clut:
        if len(data) < offset + 12:
            return None
        clut_block_len = struct.unpack_from("<I", data, offset)[0]
        clut_w = struct.unpack_from("<H", data, offset + 8)[0]
        clut_h = struct.unpack_from("<H", data, offset + 10)[0]
        entries_per_palette = clut_w
        base = offset + 12
        n = clut_w * clut_h
        if base + n * 2 > len(data):
            return None          # truncated CLUT -> scalar handles it
        words = np.frombuffer(data, dtype="<u2", count=n, offset=base).astype(np.uint32)
        r = ((words & 0x1F) << 3).astype(np.uint8)
        g = (((words >> 5) & 0x1F) << 3).astype(np.uint8)
        b = (((words >> 10) & 0x1F) << 3).astype(np.uint8)
        a = np.where(words == 0, 0, 255).astype(np.uint8)
        clut = np.stack([r, g, b, a], axis=1)
        offset += clut_block_len

    if len(data) < offset + 12:
        return None
    pix_w = struct.unpack_from("<H", data, offset + 8)[0]
    pix_h = struct.unpack_from("<H", data, offset + 10)[0]
    pix_start = offset + 12

    if bpp == 4:
        img_w = pix_w * 4
    elif bpp == 8:
        img_w = pix_w * 2
    elif bpp == 16:
        img_w = pix_w
    else:
        img_w = (pix_w * 2) // 3
    if img_w == 0 or pix_h == 0:
        return None

    row_bytes = pix_w * 2
    # Need the full row-strided region present; otherwise defer to scalar,
    # which reproduces the exact per-pixel truncation behaviour.
    if len(data) < pix_start + pix_h * row_bytes:
        return None
    rows = np.frombuffer(data, dtype=np.uint8, count=pix_h * row_bytes,
                         offset=pix_start).reshape(pix_h, row_bytes)
    pal_off = palette_index * entries_per_palette if entries_per_palette > 0 else 0

    if bpp in (4, 8):
        if bpp == 8:
            raw = rows[:, :img_w].astype(np.int64)
        else:  # 4bpp: low nibble first, then high nibble
            idxb = np.empty((pix_h, img_w), dtype=np.int64)
            idxb[:, 0::2] = rows & 0x0F
            idxb[:, 1::2] = rows >> 4
            raw = idxb
        idx = raw + pal_off
        if clut is not None and len(clut) > 0:
            valid = idx < len(clut)
            gathered = clut[np.where(valid, idx, 0)]
            gv = (raw * 17).astype(np.uint8) if bpp == 4 else raw.astype(np.uint8)
            gray = np.stack([gv, gv, gv, np.full(gv.shape, 255, np.uint8)], axis=-1)
            out = np.where(valid[..., None], gathered, gray).astype(np.uint8)
        else:
            gv = (raw * 17).astype(np.uint8) if bpp == 4 else raw.astype(np.uint8)
            out = np.stack([gv, gv, gv, np.full(gv.shape, 255, np.uint8)], axis=-1)
        return img_w, pix_h, out.tobytes()

    if bpp == 16:
        words = rows.view("<u2")[:, :img_w].astype(np.uint32)
        r = ((words & 0x1F) << 3).astype(np.uint8)
        g = (((words >> 5) & 0x1F) << 3).astype(np.uint8)
        b = (((words >> 10) & 0x1F) << 3).astype(np.uint8)
        a = np.where(words == 0, 0, 255).astype(np.uint8)
        out = np.stack([r, g, b, a], axis=-1)
        return img_w, pix_h, out.tobytes()

    # 24bpp is vanishingly rare (none exist in AC1) so there's no corpus to
    # bit-verify a vectorised path against — defer to the proven scalar decoder.
    return None


def _decode_tim_scalar(data: bytes, palette_index: int = 0) -> Optional[tuple[int, int, bytes]]:
    """Reference scalar TIM decoder (source of truth; fallback for the numpy
    fast path on truncated/edge-case data)."""

    if len(data) < 12 or data[0] != 0x10 or data[1] != 0x00:
        return None

    flag = struct.unpack_from("<I", data, 4)[0]
    bpp_code = flag & 0x03
    has_clut = bool(flag & 0x08)
    bpp_map = {0: 4, 1: 8, 2: 16, 3: 24}
    bpp = bpp_map.get(bpp_code, 0)
    if bpp == 0:
        return None

    offset = 8
    clut: list[tuple[int, int, int, int]] = []
    entries_per_palette = 0

    if has_clut:
        if len(data) < offset + 12:
            return None
        clut_block_len = struct.unpack_from("<I", data, offset)[0]
        clut_w = struct.unpack_from("<H", data, offset + 8)[0]
        clut_h = struct.unpack_from("<H", data, offset + 10)[0]
        entries_per_palette = clut_w
        base = offset + 12
        for i in range(clut_w * clut_h):
            pos = base + i * 2
            if pos + 2 > len(data):
                break
            word = struct.unpack_from("<H", data, pos)[0]
            r = (word & 0x1F) << 3
            g = ((word >> 5) & 0x1F) << 3
            b = ((word >> 10) & 0x1F) << 3
            a = 0 if word == 0 else 255
            clut.append((r, g, b, a))
        offset += clut_block_len

    if len(data) < offset + 12:
        return None

    pix_w = struct.unpack_from("<H", data, offset + 8)[0]
    pix_h = struct.unpack_from("<H", data, offset + 10)[0]
    pix_start = offset + 12

    if bpp == 4:
        img_w = pix_w * 4
    elif bpp == 8:
        img_w = pix_w * 2
    elif bpp == 16:
        img_w = pix_w
    else:
        img_w = (pix_w * 2) // 3

    if img_w == 0 or pix_h == 0:
        return None

    pal_off = palette_index * entries_per_palette if entries_per_palette > 0 else 0
    row_bytes = pix_w * 2
    rgba = bytearray(img_w * pix_h * 4)

    if bpp in (4, 8):
        for y in range(pix_h):
            row_start = pix_start + y * row_bytes
            for x in range(img_w):
                if bpp == 4:
                    byte_pos = row_start + x // 2
                    if byte_pos >= len(data):
                        continue
                    nibble = (data[byte_pos] >> ((x & 1) * 4)) & 0x0F
                    idx = pal_off + nibble
                else:
                    byte_pos = row_start + x
                    if byte_pos >= len(data):
                        continue
                    idx = pal_off + data[byte_pos]
                if clut and idx < len(clut):
                    r, g, b, a = clut[idx]
                else:
                    raw = idx - pal_off
                    v = raw * 17 if bpp == 4 else raw
                    r, g, b, a = v, v, v, 255
                out = (y * img_w + x) * 4
                rgba[out] = r; rgba[out + 1] = g; rgba[out + 2] = b; rgba[out + 3] = a

    elif bpp == 16:
        for y in range(pix_h):
            for x in range(img_w):
                pos = pix_start + y * row_bytes + x * 2
                if pos + 2 > len(data):
                    continue
                word = struct.unpack_from("<H", data, pos)[0]
                r = (word & 0x1F) << 3
                g = ((word >> 5) & 0x1F) << 3
                b = ((word >> 10) & 0x1F) << 3
                a = 0 if word == 0 else 255
                out = (y * img_w + x) * 4
                rgba[out] = r; rgba[out + 1] = g; rgba[out + 2] = b; rgba[out + 3] = a

    else:  # 24bpp: 3 bytes/pixel packed into 16-bit word rows
        for y in range(pix_h):
            for x in range(img_w):
                pos = pix_start + y * row_bytes + x * 3
                if pos + 3 > len(data):
                    continue
                out = (y * img_w + x) * 4
                rgba[out] = data[pos]; rgba[out + 1] = data[pos + 1]
                rgba[out + 2] = data[pos + 2]; rgba[out + 3] = 255

    return img_w, pix_h, bytes(rgba)


def parse_tim(tim_path: Path) -> Optional[TimInfo]:
    """
    Parse a TIM file header per the Sony spec.
    Returns None if the file is not a valid TIM.
    """
    try:
        data = tim_path.read_bytes()
    except OSError:
        return None

    if len(data) < 12:
        return None

    # Magic: 0x10 at byte 0, version 0x00 at byte 1
    if data[0] != 0x10 or data[1] != 0x00:
        return None

    flag = struct.unpack_from("<I", data, 4)[0]
    bpp_code = flag & 0x03
    has_clut = bool(flag & 0x08)

    bpp_map = {0: 4, 1: 8, 2: 16, 3: 24}
    bpp = bpp_map.get(bpp_code, 0)
    if bpp == 0:
        return None

    offset = 8
    clut_x = clut_y = clut_w = clut_h = 0
    clut_words = ()

    if has_clut:
        if len(data) < offset + 12:
            return None
        # clut_length = struct.unpack_from("<I", data, offset)[0]  # not needed
        clut_x = struct.unpack_from("<H", data, offset + 4)[0]
        clut_y = struct.unpack_from("<H", data, offset + 6)[0]
        clut_w = struct.unpack_from("<H", data, offset + 8)[0]
        clut_h = struct.unpack_from("<H", data, offset + 10)[0]
        clut_block_len = struct.unpack_from("<I", data, offset)[0]
        n_words = min(clut_w * clut_h, max(0, (len(data) - offset - 12) // 2))
        clut_words = struct.unpack_from(f"<{n_words}H", data, offset + 12)
        offset += clut_block_len

    if len(data) < offset + 12:
        return None

    # pixel_length = struct.unpack_from("<I", data, offset)[0]
    pix_x = struct.unpack_from("<H", data, offset + 4)[0]
    pix_y = struct.unpack_from("<H", data, offset + 6)[0]
    pix_w = struct.unpack_from("<H", data, offset + 8)[0]   # VRAM words wide
    pix_h = struct.unpack_from("<H", data, offset + 10)[0]

    # Convert stored VRAM width to actual pixel width
    if bpp == 4:
        img_w = pix_w * 4
    elif bpp == 8:
        img_w = pix_w * 2
    else:
        img_w = pix_w  # 16bpp and 24bpp: 1 word = 1 pixel (approx)

    return TimInfo(
        bpp=bpp,
        has_clut=has_clut,
        clut_x=clut_x,
        clut_y=clut_y,
        clut_width=clut_w,
        clut_height=clut_h,
        pixel_x=pix_x,
        pixel_y=pix_y,
        pixel_width=pix_w,
        pixel_height=pix_h,
        image_width=img_w,
        image_height=pix_h,
        file_size=len(data),
        clut_words=tuple(clut_words),
    )


# ---------------------------------------------------------------------------
# Audio info / extraction / validation
# ---------------------------------------------------------------------------

@dataclass
class AudioInfo:
    sample_rate: int        # Hz, e.g. 37800
    channels: int           # 1 = mono, 2 = stereo
    sample_width: int       # bytes per sample (1=8-bit, 2=16-bit, 3=24-bit, 4=32-bit)
    num_frames: int         # total PCM frames
    duration_secs: float
    file_size: int

    @property
    def bit_depth(self) -> int:
        return self.sample_width * 8

    @property
    def duration_str(self) -> str:
        total = int(self.duration_secs)
        m, s = divmod(total, 60)
        ms = int((self.duration_secs - total) * 100)
        return f"{m}:{s:02d}.{ms:02d}"


def parse_wav_info(wav_path: Path) -> Optional[AudioInfo]:
    import wave as _wave
    try:
        with _wave.open(str(wav_path), 'rb') as wf:
            channels = wf.getnchannels()
            sampwidth = wf.getsampwidth()
            framerate = wf.getframerate()
            nframes = wf.getnframes()
        duration = nframes / framerate if framerate > 0 else 0.0
        return AudioInfo(
            sample_rate=framerate,
            channels=channels,
            sample_width=sampwidth,
            num_frames=nframes,
            duration_secs=duration,
            file_size=wav_path.stat().st_size,
        )
    except Exception as e:
        log.debug("parse_wav_info %s: %s", wav_path, e)
        return None


def extract_all_audio(index_path: Path, out_dir: Path) -> tuple[bool, str]:
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    rc, out, err = _run([
        "-x", str(index_path), "-a", "audio", "-soundfmt", "wav", "-dir", str(out_dir)
    ])
    return rc == 0, err or out


def extract_audio_for_item(index_path: Path, item_number: int, out_dir: Path) -> tuple[bool, str]:
    out_dir = Path(out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    rc, out, err = _run([
        "-x", str(index_path),
        "-i", str(item_number),
        "-soundfmt", "wav",
        "-dir", str(out_dir),
    ])
    return rc == 0, err or out


def validate_and_encode_audio(
    input_path: Path,
    ref: AudioInfo,
    out_path: Path,
) -> tuple[bool, str, Optional["AudioInfo"]]:
    """
    Validate and prepare a WAV replacement for a PS1 audio stream.

    Rules:
      - Only WAV accepted; anything else → error.
      - Sample rate mismatch → error (must match for XA injection).
      - Channel count mismatch → error.
      - Input longer than reference → error, import refused.
      - Input shorter → pad tail with silence to exactly match ref length.
      - Bit depth > 16 → clamped to 16-bit PCM (lossy).
      - Bit depth 8 → expanded to 16-bit signed.

    Returns (ok, message, output_AudioInfo).
    """
    import wave as _wave
    import array as _array

    if input_path.suffix.lower() != '.wav':
        return False, "Only WAV files are accepted for audio replacement.", None

    src = parse_wav_info(input_path)
    if src is None:
        return False, "Could not read WAV file — is it a valid PCM WAV?", None

    if src.sample_rate != ref.sample_rate:
        return (
            False,
            f"Sample rate mismatch: file is {src.sample_rate} Hz, "
            f"original is {ref.sample_rate} Hz. "
            f"Re-export your audio at exactly {ref.sample_rate} Hz.",
            None,
        )

    if src.channels != ref.channels:
        ch_hint = "Convert to mono." if ref.channels == 1 else "Convert to stereo."
        return (
            False,
            f"Channel count mismatch: file has {src.channels} channel(s), "
            f"original has {ref.channels}. {ch_hint}",
            None,
        )

    if src.num_frames > ref.num_frames:
        over_secs = (src.num_frames - ref.num_frames) / ref.sample_rate
        return (
            False,
            f"Audio is too long by {over_secs:.2f}s "
            f"({src.num_frames} frames vs {ref.num_frames} max). "
            f"Trim the file before importing.",
            None,
        )

    # Read raw PCM bytes
    with _wave.open(str(input_path), 'rb') as wf:
        raw = wf.readframes(src.num_frames)

    clamped_msg = ""
    out_sampwidth = src.sample_width

    if src.sample_width == 4:
        # 32-bit → 16-bit: shift right 16
        arr = _array.array('i')
        arr.frombytes(raw)
        out_arr = _array.array('h', (v >> 16 for v in arr))
        raw = out_arr.tobytes()
        out_sampwidth = 2
        clamped_msg = " (bit depth clamped 32→16)"
    elif src.sample_width == 3:
        # 24-bit → 16-bit: drop low byte
        n = len(raw) // 3
        out_arr = _array.array('h')
        for i in range(n):
            b0, b1, b2 = raw[i*3], raw[i*3+1], raw[i*3+2]
            val = b0 | (b1 << 8) | (b2 << 16)
            if val >= 0x800000:
                val -= 0x1000000
            out_arr.append(val >> 8)
        raw = out_arr.tobytes()
        out_sampwidth = 2
        clamped_msg = " (bit depth clamped 24→16)"
    elif src.sample_width == 1:
        # 8-bit unsigned → 16-bit signed
        out_arr = _array.array('h', ((b - 128) * 256 for b in raw))
        raw = out_arr.tobytes()
        out_sampwidth = 2
        clamped_msg = " (bit depth expanded 8→16)"

    # Pad with silence if shorter than reference
    frame_bytes = src.channels * out_sampwidth
    current_frames = len(raw) // frame_bytes
    padded_msg = ""
    if current_frames < ref.num_frames:
        pad_frames = ref.num_frames - current_frames
        raw = raw + bytes(frame_bytes * pad_frames)
        pad_secs = pad_frames / src.sample_rate
        padded_msg = f" (padded +{pad_secs:.2f}s silence)"

    # Ensure output is exactly ref.num_frames long
    raw = raw[:ref.num_frames * frame_bytes]

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with _wave.open(str(out_path), 'wb') as wf:
        wf.setnchannels(src.channels)
        wf.setsampwidth(out_sampwidth)
        wf.setframerate(src.sample_rate)
        wf.writeframes(raw)

    result = AudioInfo(
        sample_rate=src.sample_rate,
        channels=src.channels,
        sample_width=out_sampwidth,
        num_frames=ref.num_frames,
        duration_secs=ref.num_frames / src.sample_rate,
        file_size=out_path.stat().st_size,
    )
    return True, f"ok{clamped_msg}{padded_msg}", result


# ---------------------------------------------------------------------------
# TIM encoder — convert PNG/GIF/TIM → TIM matching a reference TimInfo
# ---------------------------------------------------------------------------

def _quantize_with_alpha(
    img_rgba,
    n_colors: int,
    alpha_threshold: int = 128,
) -> tuple:
    """
    Quantize an RGBA PIL Image to n_colors palette entries.

    When any pixel has alpha < alpha_threshold, index 0 is reserved for
    transparent pixels and the remaining n_colors-1 slots hold opaque colors.
    This mirrors the PS1 convention where CLUT[0] = 0x0000 = transparent.

    Returns (pixels, palette_rgb, has_transparency):
      pixels        : list[int] of palette indices, length = w*h
      palette_rgb   : list[(r,g,b)] of length n_colors
      has_transparency: bool
    """
    rgba_data = list(img_rgba.getdata())
    has_transparency = any(px[3] < alpha_threshold for px in rgba_data)

    max_opaque = n_colors - 1 if has_transparency else n_colors

    rgb = img_rgba.convert("RGB")
    q = rgb.quantize(colors=max_opaque)
    raw_pal = q.getpalette() or []
    q_pixels = list(q.getdata())

    if has_transparency:
        pixels = [
            0 if px[3] < alpha_threshold else q_pixels[i] + 1
            for i, px in enumerate(rgba_data)
        ]
        palette = [(0, 0, 0)]
        for i in range(max_opaque):
            if i * 3 + 2 < len(raw_pal):
                palette.append((raw_pal[i * 3], raw_pal[i * 3 + 1], raw_pal[i * 3 + 2]))
            else:
                palette.append((0, 0, 0))
    else:
        pixels = q_pixels
        palette = []
        for i in range(n_colors):
            if i * 3 + 2 < len(raw_pal):
                palette.append((raw_pal[i * 3], raw_pal[i * 3 + 1], raw_pal[i * 3 + 2]))
            else:
                palette.append((0, 0, 0))

    return pixels, palette, has_transparency


def encode_to_tim(input_path: Path, ref: TimInfo, out_path: Path) -> tuple[bool, str]:
    """Convert a PNG/GIF/TIM to a TIM file matching ref's structure.

    All positional/structural fields are taken from ref:
      pixel_x, pixel_y, pixel_width, pixel_height, bpp, has_clut,
      clut_x, clut_y, clut_width, clut_height.
    Only CLUT color data and pixel data come from input_path. Every CLUT row of a
    multi-palette TIM gets the same quantized palette. A CLUT word keeps the STP
    bit of ref's word at the same position (0x8000 when ref.clut_words is empty);
    opaque black is always 0x8000, since 0x0000 is the transparent sentinel.
    """
    suffix = input_path.suffix.lower()
    if suffix == ".tim":
        return _patch_tim_coords(input_path, ref, out_path)

    try:
        from PIL import Image as PILImage
    except ImportError:
        return False, "Pillow not installed — run: pip install Pillow"

    try:
        img = PILImage.open(str(input_path)).convert("RGBA")
    except Exception as e:
        return False, f"Could not open image: {e}"

    if img.size != (ref.image_width, ref.image_height):
        img = img.resize((ref.image_width, ref.image_height), PILImage.LANCZOS)

    buf = bytearray(b'\x10\x00\x00\x00')

    if ref.bpp == 4:
        bpp_code, n_colors = 0, 16
    elif ref.bpp == 8:
        bpp_code, n_colors = 1, 256
    elif ref.bpp == 16:
        bpp_code, n_colors = 2, 0
    else:
        bpp_code, n_colors = 3, 0

    buf += struct.pack("<I", bpp_code | (0x08 if ref.has_clut else 0))

    if n_colors:
        pixels, palette, has_transparency = _quantize_with_alpha(img, n_colors)

        if ref.has_clut:
            total = ref.clut_width * ref.clut_height
            words = []
            for i in range(total):
                k = i % n_colors            # entry k of palette row i // n_colors
                r8, g8, b8 = palette[k]
                if k == 0 and has_transparency:
                    w = 0x0000  # transparent sentinel
                else:
                    w = (r8 >> 3) | ((g8 >> 3) << 5) | ((b8 >> 3) << 10)
                    if not w:
                        w = 0x8000  # opaque black: 0x0000 would be transparent
                    elif i < len(ref.clut_words):
                        w |= ref.clut_words[i] & 0x8000  # keep the original STP bit
                    else:
                        w |= 0x8000
                words.append(w)
            clut_data = struct.pack(f"<{len(words)}H", *words)
            buf += struct.pack("<I", 12 + len(clut_data))
            buf += struct.pack("<HH", ref.clut_x, ref.clut_y)
            buf += struct.pack("<HH", ref.clut_width, ref.clut_height)
            buf += clut_data

        row_bytes = ref.pixel_width * 2
        pix_data = bytearray(row_bytes * ref.image_height)
        for y in range(ref.image_height):
            for x in range(ref.image_width):
                idx = pixels[y * ref.image_width + x] & 0xFF
                if ref.bpp == 4:
                    bp = y * row_bytes + x // 2
                    pix_data[bp] |= (idx & 0x0F) << ((x & 1) * 4)
                else:
                    pix_data[y * row_bytes + x] = idx

    elif ref.bpp == 16:
        pix_list = list(img.getdata())
        row_bytes = ref.pixel_width * 2
        pix_data = bytearray(row_bytes * ref.image_height)
        for y in range(ref.image_height):
            for x in range(ref.image_width):
                rr, gg, bb, aa = pix_list[y * ref.image_width + x]
                if aa < 128:
                    word = 0
                else:
                    word = (rr >> 3) | ((gg >> 3) << 5) | ((bb >> 3) << 10) | 0x8000
                bp = y * row_bytes + x * 2
                pix_data[bp] = word & 0xFF
                pix_data[bp + 1] = (word >> 8) & 0xFF

    else:  # 24bpp
        pix_list = list(img.getdata())
        row_bytes = ref.pixel_width * 2
        pix_data = bytearray(row_bytes * ref.image_height)
        for y in range(ref.image_height):
            for x in range(ref.image_width):
                bp = y * row_bytes + x * 3
                if bp + 2 >= len(pix_data):
                    break
                rr, gg, bb, _ = pix_list[y * ref.image_width + x]
                pix_data[bp] = rr; pix_data[bp + 1] = gg; pix_data[bp + 2] = bb

    buf += struct.pack("<I", 12 + len(pix_data))
    buf += struct.pack("<HH", ref.pixel_x, ref.pixel_y)
    buf += struct.pack("<HH", ref.pixel_width, ref.image_height)
    buf += bytes(pix_data)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(bytes(buf))
    return True, str(out_path)


def _patch_tim_coords(input_path: Path, ref: TimInfo, out_path: Path) -> tuple[bool, str]:
    """Copy a TIM, patching all positional fields from ref."""
    try:
        data = bytearray(input_path.read_bytes())
    except OSError as e:
        return False, str(e)
    if len(data) < 12 or data[0] != 0x10:
        return False, "Not a valid TIM file"

    flag = struct.unpack_from("<I", data, 4)[0]
    has_clut = bool(flag & 0x08)
    offset = 8

    if has_clut and len(data) >= offset + 12:
        clut_block_len = struct.unpack_from("<I", data, offset)[0]
        struct.pack_into("<HH", data, offset + 4, ref.clut_x, ref.clut_y)
        offset += clut_block_len

    if len(data) >= offset + 12:
        struct.pack_into("<HH", data, offset + 4, ref.pixel_x, ref.pixel_y)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(bytes(data))
    return True, str(out_path)
