"""
core/fdat.py — read FDAT.T entries (for mission data) from the disc image.

FDAT.T (file id 2) is a count-first .T container; mission N is described by entries
2N (objective code) and 2N+1 (chunk stream). We only need raw entry bytes here; the
container is split by tools/extract/t_repack.py (via core.pa_parser.read_container).
"""
from __future__ import annotations
import struct, re
from pathlib import Path
from core.pa_parser import read_container

# FDAT.T disc sectors (SLUS-01323 v1.1). Overridable via the jPSXdec index.
FDAT_SECTORS = (72189, 85330)


def fdat_sectors(index_path=None):
    if index_path and Path(index_path).exists():
        for line in Path(index_path).read_text(errors="replace").splitlines():
            if "ID:GG/COM/FDAT.T|" in line and "|Type:File|" in line:
                m = re.search(r"Sectors:(\d+)-(\d+)", line)
                if m:
                    return int(m.group(1)), int(m.group(2))
    return FDAT_SECTORS


_cache = {}


def fdat_entries(bin_path, index_path=None):
    key = str(bin_path)
    if key not in _cache:
        a, b = fdat_sectors(index_path)
        _cache[key] = read_container(bin_path, a, b)
    return _cache[key]


def mission_entry(bin_path, n, odd=True, index_path=None):
    """Mission N's chunk-stream entry (2N+1) by default, else objective code (2N)."""
    ents = fdat_entries(bin_path, index_path)
    idx = 2 * n + (1 if odd else 0)
    return ents[idx] if idx < len(ents) else b""


# Entries past the mission pairs (docs/REFERENCE.md "FDAT.T entry indices",
# docs/LEVEL_LOADING.md §4).
FIXED_ROLES = {
    200: "shared scene stream",
    201: "front-end / text overlay (ovl201)",
    202: "mission (mode 0) overlay (ovl202)",
    203: "mode-1 overlay (ovl203)",
    204: "mode-2 overlay (ovl204)",
}

# Arena/versus stage pairs loaded by the mode overlays, not by the mission overlay:
# stage S -> entries 2S, 2S+1 above the base (docs/MISSION_OVERLAY_CODE.md §c).
MODE_STAGE_BASES = ((160, 170, "mode-2 overlay"), (180, 190, "mode-1 overlay"))


def fdat_child_info(bin_path, index_path=None):
    """List FDAT.T's entries for tree expansion: [(index, size_bytes, role)].

    Mission N occupies entries 2N (objective code) and 2N+1 (chunk stream);
    160-169 and 180-189 are the same pair layout for the mode-2 and mode-1
    overlays' stages; 200-204 are the shared scene stream and the four code
    overlays. A zero-length slice is an unused slot.
    """
    out = []
    for i, ent in enumerate(fdat_entries(bin_path, index_path)):
        kind = "objective" if i % 2 == 0 else "chunk-stream"
        mode = next(((a, lbl) for a, b, lbl in MODE_STAGE_BASES if a <= i < b), None)
        if not ent:
            role = "empty"
        elif i in FIXED_ROLES:
            role = FIXED_ROLES[i]
        elif mode:
            role = f"{mode[1]} stage {(i - mode[0]) // 2} {kind}"
        else:
            role = f"mission {i // 2} {kind}"
        out.append((i, len(ent), role))
    return out
