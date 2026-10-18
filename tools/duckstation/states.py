#!/usr/bin/env python3
"""
tools/duckstation/states.py — the AC1 save-state LIBRARY.

Problem it solves
-----------------
DuckStation (and its MCP server) only knows numbered slots 1-10 per game:
<DuckStation data>/savestates/SLUS-01323_<N>.sav. Earlier RE sessions cited slot
numbers in the docs and later sessions overwrote the slots, so no in-mission state
survived (docs/SAVESTATE_FORMAT.md "Findings", docs/LIVE_CHECKLIST.md S-1).

Two-layer scheme:
  * slots are SCRATCH — DuckStation writes them, nothing cites them;
  * the LIBRARY (a directory outside the repo, default ~/dev/ac1_states, env
    AC1_STATES_DIR) plus the committed manifest disc_map/states/STATES.tsv is the
    RECORD. Every archived state gets a stable ID ({context}-{phase}-{seq}, e.g.
    M01-ctrl-01; see disc_map/states/README.md) and docs cite that ID.

Subcommands
-----------
  archive <slot> <id> [--purpose ..] [--serves S-1,LVL-1] [--parent ID] [--caveats ..] [--note ..]
      copy the slot file into the library as <id>__<YYYYMMDD>__v<stateversion>.sav,
      write <same>.json (sidecar) and <same>.png (embedded thumbnail), append a
      manifest row. Automatic fields are read from the state (header + RAM);
      anything that cannot be read is recorded as "?" and the archive still happens.
  adopt <path> <id> [same manual flags] [--slot N] [--date YYYYMMDD] [--keep-name]
      register a .sav that already exists (copied by hand into the library, or
      elsewhere): renamed/moved to the canonical name, dated by its mtime.
  stage <id> <slot> [--force]
      copy a library state back into a slot. Refuses to overwrite a slot whose
      current content is not itself archived (sha256 not in the manifest).
  check
      sha256 every manifest row, flag missing/orphan files, bad IDs, dangling parents.
  list [--mission N] [--phase P] [--serves ID] [--context CTX] [--status S]
      query the manifest.
  show <id>            print the manifest row + sidecar.
  refresh [id]         re-read the automatic columns (after a reader fix / new decoder).
  set-status <id> <good|stale|superseded>

Layout of a library entry (nothing under the library is ever committed):
  <lib>/M01-ctrl-01__20261018__v83.sav    the state (copyrighted game RAM — never commit)
  <lib>/M01-ctrl-01__20261018__v83.json   sidecar: everything in the manifest row + header
  <lib>/M01-ctrl-01__20261018__v83.png    thumbnail pulled from the DUCCS header
  <lib>/disc_cache.json                   sha256 + EXE anchor per disc image (speed)

Environment
-----------
  AC1_STATES_DIR  library dir (default ~/dev/ac1_states; must be OUTSIDE the repo)
  AC1_DS_DIR      DuckStation data dir (default: platform Documents/DuckStation etc.)
  AC1_BIN         disc image used for the RAM anchor + disc identity (or --disc)
  AC1_DS_EXE      DuckStation executable, for the build string (default: per-user install)
"""
from __future__ import annotations
import argparse, csv, datetime as dt, hashlib, json, os, platform, re, shutil, struct, sys, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, HERE)
import savestate as SS  # noqa: E402

SERIAL = "SLUS-01323"
MANIFEST_DEFAULT = os.path.join(REPO, "disc_map", "states", "STATES.tsv")
UNKNOWN = "?"
DISC_CACHE_SCHEMA = 2        # bump when exe_anchor_from_disc() changes what it caches
AUTO_COLUMNS = ("state_version", "frame", "mission", "stage", "objobj", "flags", "result",
                "in_mission", "disc")

CONTEXTS = {f"M{i:02d}" for i in range(59)} | {"GAR", "ARN", "TTL", "DBF"}
PHASES = ("load", "ctrl", "mid", "obj", "exit", "dead", "main")
STATUSES = ("good", "stale", "superseded")
ID_RE = re.compile(r"^(?P<ctx>M[0-9]{2}|GAR|ARN|TTL|DBF)-(?P<phase>[a-z]+)-(?P<seq>[0-9]{2})$")

# Manifest columns. Automatic first, then the manual ones.
COLUMNS = [
    "id", "status", "date", "file", "sha256", "state_version", "duckstation_build",
    "frame", "mission", "stage", "objobj", "flags", "result", "in_mission", "disc",
    "purpose", "serves", "parent", "caveats", "notes",
]

# RAM fields recorded per state (PS1 address, width, formatter).
RAM_FIELDS = (
    ("mission", 0x800411F8, "u8", str),           # mission number byte
    ("stage", 0x8004121B, "u8", str),             # PA-pack / stage byte (0 = menu/garage)
    ("objobj", 0x8019F51C, "u32", lambda v: f"{v:#010x}"),  # objective-object pointer
    ("flags", 0x8019F524, "u32", lambda v: f"{v:#x}"),      # mission flags word
    ("result", 0x80048610, "u32", lambda v: f"{v:#x}"),     # result code (u32 read)
)


class StatesError(Exception):
    pass


# ---------------------------------------------------------------- id helpers
def parse_id(sid: str) -> dict:
    m = ID_RE.match(sid or "")
    if not m or m["ctx"] not in CONTEXTS or m["phase"] not in PHASES:
        raise StatesError(
            f"bad state id {sid!r}: want {{context}}-{{phase}}-{{seq}} with context "
            f"M00..M58|GAR|ARN|TTL|DBF, phase {'|'.join(PHASES)}, seq two digits "
            f"(e.g. M01-ctrl-01) — see disc_map/states/README.md")
    return {"context": m["ctx"], "phase": m["phase"], "seq": m["seq"],
            "mission": int(m["ctx"][1:]) if m["ctx"].startswith("M") else None}


# ---------------------------------------------------------------- locations
def repo_root() -> str:
    return os.path.realpath(REPO)


def library_dir(create: bool = False) -> str:
    lib = os.environ.get("AC1_STATES_DIR") or os.path.join(os.path.expanduser("~"), "dev", "ac1_states")
    lib = os.path.realpath(lib)
    root = repo_root()
    if lib == root or lib.startswith(root + os.sep):
        raise StatesError(f"library dir {lib} is inside the repo {root}; .sav files are game "
                          f"data and must live outside it (set AC1_STATES_DIR)")
    if create:
        os.makedirs(lib, exist_ok=True)
    return lib


def _windows_documents() -> list:
    out = []
    try:
        import winreg
        k = winreg.OpenKey(winreg.HKEY_CURRENT_USER,
                           r"Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders")
        v, _ = winreg.QueryValueEx(k, "Personal")
        out.append(os.path.expandvars(v))
    except Exception:
        pass
    home = os.path.expanduser("~")
    for p in (os.path.join(home, "OneDrive", "Documents"), os.path.join(home, "Documents")):
        if p not in out:
            out.append(p)
    return out


def duckstation_data_dir() -> str:
    """DuckStation's per-user data dir (holds settings.ini, savestates/, ...)."""
    env = os.environ.get("AC1_DS_DIR")
    if env:
        return env
    cands = []
    if sys.platform == "win32":
        cands += [os.path.join(d, "DuckStation") for d in _windows_documents()]
    elif sys.platform == "darwin":
        cands.append(os.path.expanduser("~/Library/Application Support/DuckStation"))
    else:
        cands.append(os.path.expanduser("~/.local/share/duckstation"))
        cands.append(os.path.expanduser("~/.var/app/org.duckstation.DuckStation/data/duckstation"))
    for c in cands:
        if os.path.isfile(os.path.join(c, "settings.ini")) or os.path.isdir(os.path.join(c, "savestates")):
            return c
    raise StatesError("DuckStation data dir not found (tried: " + ", ".join(cands) +
                      "); set AC1_DS_DIR")


def _ini_value(path: str, section: str, key: str):
    """Minimal settings.ini reader (DuckStation writes plain `Key = Value` lines)."""
    if not os.path.isfile(path):
        return None
    cur = None
    for line in open(path, encoding="utf-8", errors="replace"):
        s = line.strip()
        if s.startswith("[") and s.endswith("]"):
            cur = s[1:-1]
        elif cur == section and "=" in s:
            k, v = s.split("=", 1)
            if k.strip() == key:
                return v.strip()
    return None


def savestates_dir(data_dir=None) -> str:
    data_dir = data_dir or duckstation_data_dir()
    custom = _ini_value(os.path.join(data_dir, "settings.ini"), "Folders", "SaveStates")
    if custom:
        return custom if os.path.isabs(custom) else os.path.join(data_dir, custom)
    return os.path.join(data_dir, "savestates")


def slot_path(slot, data_dir=None, serial: str = SERIAL) -> str:
    s = str(slot)
    if s.isdigit():
        n = int(s)
        if not 1 <= n <= 10:
            raise StatesError("slot must be 1..10 (or 'resume')")
        name = f"{serial}_{n}.sav"
    elif s == "resume":
        name = f"{serial}_resume.sav"
    else:
        raise StatesError(f"bad slot {slot!r}: 1..10 or 'resume'")
    return os.path.join(savestates_dir(data_dir), name)


def duckstation_exe() -> str | None:
    env = os.environ.get("AC1_DS_EXE")
    if env and os.path.isfile(env):
        return env
    cands = []
    if sys.platform == "win32":
        la = os.environ.get("LOCALAPPDATA", "")
        cands.append(os.path.join(la, "Programs", "DuckStation", "duckstation-qt-x64-ReleaseLTCG.exe"))
        cands.append(os.path.join(la, "Programs", "DuckStation", "duckstation-qt-x64.exe"))
    for c in cands:
        if os.path.isfile(c):
            return c
    return shutil.which("duckstation-qt") or shutil.which("duckstation-qt-x64-ReleaseLTCG")


def _win_file_version(path: str):
    """'0.1.12000.0' via version.dll (no PowerShell spawn)."""
    try:
        import ctypes
        from ctypes import wintypes
        ver = ctypes.WinDLL("version")
        size = ver.GetFileVersionInfoSizeW(path, None)
        if not size:
            return None
        buf = ctypes.create_string_buffer(size)
        if not ver.GetFileVersionInfoW(path, 0, size, buf):
            return None
        p = ctypes.c_void_p()
        n = wintypes.UINT()
        if not ver.VerQueryValueW(buf, "\\", ctypes.byref(p), ctypes.byref(n)):
            return None
        ffi = ctypes.string_at(p.value, n.value)
        ms, ls = struct.unpack_from("<II", ffi, 8)   # dwFileVersionMS / LS
        return f"{ms >> 16}.{ms & 0xffff}.{ls >> 16}.{ls & 0xffff}"
    except Exception:
        return None


def duckstation_build(data_dir=None) -> str:
    """e.g. '0.1-12000 9ed01fa1' (exe file version + settings.ini LastVersion hash)."""
    parts = []
    exe = duckstation_exe()
    if exe and sys.platform == "win32":
        v = _win_file_version(exe)
        if v:
            a = v.split(".")
            parts.append(f"{a[0]}.{a[1]}-{a[2]}" if len(a) >= 3 else v)
    try:
        ini = os.path.join(data_dir or duckstation_data_dir(), "settings.ini")
        h = _ini_value(ini, "AutoUpdater", "LastVersion")
        if h:
            parts.append(h[:8])
    except StatesError:
        pass
    return " ".join(parts) if parts else UNKNOWN


# ---------------------------------------------------------------- disc identity
def sha256_file(path: str) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 22), b""):
            h.update(chunk)
    return h.hexdigest()


def disc_info(disc: str, lib: str) -> dict:
    """{'sha256','sha12','anchor'(hex),'t_addr'} for a disc image, cached per (path,size,mtime)."""
    cache_path = os.path.join(lib, "disc_cache.json")
    cache = {}
    if os.path.isfile(cache_path):
        try:
            cache = json.load(open(cache_path, encoding="utf-8"))
        except Exception:
            cache = {}
    st = os.stat(disc)
    key = os.path.realpath(disc)
    ent = cache.get(key)
    if (ent and ent.get("size") == st.st_size and ent.get("mtime") == int(st.st_mtime)
            and ent.get("schema") == DISC_CACHE_SCHEMA):
        return ent
    sha = sha256_file(disc)
    anchor, t_addr = SS.exe_anchor_from_disc(disc)
    ent = {"schema": DISC_CACHE_SCHEMA, "size": st.st_size, "mtime": int(st.st_mtime),
           "sha256": sha, "sha12": sha[:12], "anchor": anchor.hex(), "t_addr": t_addr}
    cache[key] = ent
    os.makedirs(lib, exist_ok=True)
    json.dump(cache, open(cache_path, "w", encoding="utf-8"), indent=1)
    return ent


# ---------------------------------------------------------------- png
def write_png(path: str, w: int, h: int, rgba: bytes) -> None:
    raw = b"".join(b"\x00" + rgba[y * w * 4:(y + 1) * w * 4] for y in range(h))

    def chunk(tag, data):
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xffffffff)

    png = (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
           + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))
    with open(path, "wb") as f:
        f.write(png)


# ---------------------------------------------------------------- reading a state
def read_fields(state_path: str, disc: str | None, lib: str, warn=print) -> tuple:
    """Return (fields_for_manifest, sidecar_extra). Never raises for missing
    decoders/disc: unreadable fields become '?' and a warning is printed."""
    fields = {k: UNKNOWN for k in ("state_version", "frame", "mission", "stage", "objobj",
                                   "flags", "result", "in_mission", "disc")}
    extra = {"warnings": []}

    def w(msg):
        extra["warnings"].append(msg)
        warn("warning: " + msg)

    hdr = SS.read_header(state_path)
    fields["state_version"] = str(hdr["version"])
    extra["header"] = {k: v for k, v in hdr.items() if k != "magic"}
    if hdr["serial"] and hdr["serial"] != SERIAL:
        w(f"state serial is {hdr['serial']!r}, expected {SERIAL!r}")

    dinfo = None
    if disc and os.path.isfile(disc):
        try:
            dinfo = disc_info(disc, lib)
            fields["disc"] = dinfo["sha12"]
            extra["disc_path"] = disc
        except Exception as ex:  # unreadable/odd disc image
            w(f"disc identity not recorded ({ex})")
    else:
        w("no disc image (--disc or $AC1_BIN): disc identity and RAM fields unknown")

    backend = SS.zstd_backend()
    ctype = hdr["data_compression_type"]
    if ctype == SS.COMPRESSION_ZSTD and backend is None:
        w("no zstd decoder (pip install zstandard): frame and RAM fields unknown")
        return fields, extra
    extra["zstd_backend"] = backend

    try:
        blob = SS.read_main_state(state_path)
    except Exception as ex:
        w(f"could not decompress state data ({ex}); frame and RAM fields unknown")
        return fields, extra

    fn = SS.frame_numbers(blob)
    if fn:
        fields["frame"] = str(fn[0])
        extra["internal_frame_number"] = fn[1]
    else:
        w("System/CPU markers not where expected; frame number unknown")

    if dinfo:
        try:
            ram_base = SS.find_ram_base(blob, bytes.fromhex(dinfo["anchor"]), dinfo["t_addr"])
        except Exception as ex:
            w(f"RAM anchor not found ({ex}); RAM fields unknown")
            return fields, extra
        extra["ram_base"] = ram_base
        bus = SS.bus_marker_offset(blob)
        extra["bus_marker"] = bus
        if bus < 0 or not (0 < ram_base - bus < 0x200):
            w(f"ram_base {ram_base:#x} is not just after the Bus marker ({bus:#x}); the EXE "
              f"anchor may have matched a copy elsewhere — treat RAM fields with suspicion")
        mission_live = False
        for name, addr, width, fmt in RAM_FIELDS:
            o = ram_base + (addr - SS.PS1_RAM_BASE)
            v = blob[o] if width == "u8" else struct.unpack_from("<I", blob, o)[0]
            fields[name] = fmt(v)
            extra.setdefault("ram_raw", {})[name] = v
        raw = extra["ram_raw"]
        mission_live = raw["objobj"] != 0 or raw["stage"] != 0   # == SaveState.in_mission()
        fields["in_mission"] = "yes" if mission_live else "no"
    return fields, extra


# ---------------------------------------------------------------- manifest
def load_manifest(path: str) -> list:
    if not os.path.isfile(path):
        return []
    with open(path, encoding="utf-8", newline="") as f:
        rd = csv.DictReader(f, delimiter="\t")
        if rd.fieldnames is None:
            return []
        missing = [c for c in COLUMNS if c not in rd.fieldnames]
        if missing:
            raise StatesError(f"{path}: manifest is missing columns {missing}")
        return [dict(r) for r in rd]


def save_manifest(path: str, rows: list) -> None:
    os.makedirs(os.path.dirname(path), exist_ok=True)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8", newline="") as f:
        wr = csv.DictWriter(f, fieldnames=COLUMNS, delimiter="\t", lineterminator="\n",
                            extrasaction="ignore")
        wr.writeheader()
        for r in rows:
            wr.writerow({c: (r.get(c, "") or "").replace("\t", " ").replace("\n", " ") for c in COLUMNS})
    os.replace(tmp, path)


def find_row(rows: list, sid: str) -> dict:
    """The live row for an id: the last one not marked superseded (--replace keeps
    the old row under the same id as history), else the last row with that id."""
    hits = [r for r in rows if r["id"] == sid]
    if not hits:
        raise StatesError(f"{sid}: not in the manifest")
    live = [r for r in hits if r["status"] != "superseded"]
    return (live or hits)[-1]


# ---------------------------------------------------------------- commands
def cmd_archive(a) -> int:
    parse_id(a.id)
    lib = library_dir(create=True)
    rows = load_manifest(a.manifest)
    existing = [r for r in rows if r["id"] == a.id]
    if existing and not a.replace:
        raise StatesError(f"{a.id} is already in the manifest ({existing[0]['file']}); use the "
                          f"next seq, or --replace to mark the old row superseded")
    if a.parent:
        parse_id(a.parent)
        if not any(r["id"] == a.parent for r in rows):
            raise StatesError(f"--parent {a.parent} is not in the manifest")
    if a.status not in STATUSES:
        raise StatesError(f"--status must be one of {STATUSES}")

    src = a.file or slot_path(a.slot, a.ds_dir)
    if not os.path.isfile(src):
        raise StatesError(f"no state at {src} (save slot {a.slot} in DuckStation first)")
    hdr = SS.read_header(src)
    today = dt.datetime.now()
    dst = _unique_target(lib, a.id, f"{today:%Y%m%d}", hdr["version"])
    shutil.copy2(src, dst)
    sha = sha256_file(dst)
    if sha != sha256_file(src):
        os.remove(dst)
        raise StatesError("copy verification failed (slot file changed while copying?)")
    print(f"archived {src} -> {dst}")
    return _register(a, rows, existing, lib, dst, src, a.slot, today, f"{today:%Y-%m-%d}")


def cmd_adopt(a) -> int:
    """Register an EXISTING .sav (already copied by hand into the library, or anywhere
    else) under an id: renamed/moved to the canonical name, dated by its mtime."""
    parse_id(a.id)
    lib = library_dir(create=True)
    rows = load_manifest(a.manifest)
    existing = [r for r in rows if r["id"] == a.id]
    if existing and not a.replace:
        raise StatesError(f"{a.id} is already in the manifest ({existing[0]['file']}); use the "
                          f"next seq, or --replace to mark the old row superseded")
    if a.parent:
        parse_id(a.parent)
        if not any(r["id"] == a.parent for r in rows):
            raise StatesError(f"--parent {a.parent} is not in the manifest")
    if a.status not in STATUSES:
        raise StatesError(f"--status must be one of {STATUSES}")
    src = os.path.abspath(a.path)
    if not os.path.isfile(src):
        raise StatesError(f"no such file: {src}")
    hdr = SS.read_header(src)
    sha = sha256_file(src)
    for r in rows:
        if r["sha256"] == sha:
            raise StatesError(f"this file is already archived as {r['id']} ({r['file']})")
    mtime = dt.datetime.fromtimestamp(os.stat(src).st_mtime)
    datestr = a.date or f"{mtime:%Y%m%d}"
    in_lib = os.path.dirname(os.path.realpath(src)) == lib
    if a.keep_name and in_lib:
        dst = src
    else:
        dst = _unique_target(lib, a.id, datestr, hdr["version"])
        if in_lib:
            os.rename(src, dst)
            for ext in (".json", ".png"):
                side = os.path.splitext(src)[0] + ext
                if os.path.isfile(side):
                    os.rename(side, os.path.splitext(dst)[0] + ext)
        else:
            shutil.copy2(src, dst)
            if sha256_file(dst) != sha:
                os.remove(dst)
                raise StatesError("copy verification failed")
    print(f"adopted {src} -> {dst}")
    return _register(a, rows, existing, lib, dst, src, a.slot or UNKNOWN, dt.datetime.now(),
                     f"{datestr[:4]}-{datestr[4:6]}-{datestr[6:8]}")


def _unique_target(lib: str, sid: str, datestr: str, version: int) -> str:
    stem = f"{sid}__{datestr}__v{version}"
    dst = os.path.join(lib, stem + ".sav")
    k = 2
    while os.path.exists(dst):
        dst = os.path.join(lib, f"{stem}-{k}.sav")
        k += 1
    return dst


def _register(a, rows, existing, lib, dst, src, slot, now, date_col) -> int:
    """Shared tail of archive/adopt: read fields, thumbnail, sidecar, manifest row."""
    stem = os.path.splitext(os.path.basename(dst))[0]
    sha = sha256_file(dst)
    disc = a.disc or os.environ.get("AC1_BIN")
    fields, extra = read_fields(dst, disc, lib)

    png = None
    try:
        shot = SS.read_screenshot(dst)
        if shot:
            png = os.path.join(lib, stem + ".png")
            write_png(png, *shot)
            print(f"thumbnail {png} ({shot[0]}x{shot[1]})")
        else:
            print("no embedded screenshot in this state")
    except Exception as ex:
        print(f"warning: thumbnail not extracted ({ex})")
        extra["warnings"].append(f"thumbnail not extracted ({ex})")

    row = {
        "id": a.id, "status": a.status, "date": date_col,
        "file": os.path.basename(dst), "sha256": sha,
        "duckstation_build": a.build or duckstation_build(a.ds_dir),
        "purpose": a.purpose or "", "serves": a.serves or "", "parent": a.parent or "",
        "caveats": a.caveats or "", "notes": a.note or "",
    }
    row.update(fields)

    sidecar = dict(row)
    sidecar.update({
        "archived_at": now.isoformat(timespec="seconds"),
        "source": src, "source_mtime": dt.datetime.fromtimestamp(os.stat(dst).st_mtime).isoformat(timespec="seconds"),
        "slot": slot, "thumbnail": os.path.basename(png) if png else None,
        "library": lib, "host": platform.node(),
    })
    sidecar.update(extra)
    with open(os.path.join(lib, stem + ".json"), "w", encoding="utf-8") as f:
        json.dump(sidecar, f, indent=1, default=str)

    for r in existing:
        r["status"] = "superseded"
    rows.append(row)
    save_manifest(a.manifest, rows)
    print(f"manifest {a.manifest}: +{a.id}  mission={row['mission']} stage={row['stage']} "
          f"in_mission={row['in_mission']} frame={row['frame']} v{row['state_version']} "
          f"disc={row['disc']}")
    if row["in_mission"] == "no" and parse_id(a.id)["mission"] is not None:
        print(f"note: id {a.id} names a mission but the state reads as NOT in-mission")
    return 0


def cmd_stage(a) -> int:
    rows = load_manifest(a.manifest)
    row = find_row(rows, a.id)
    lib = library_dir()
    src = os.path.join(lib, row["file"])
    if not os.path.isfile(src):
        raise StatesError(f"{a.id}: library file missing: {src}")
    if sha256_file(src) != row["sha256"]:
        raise StatesError(f"{a.id}: library file sha256 does not match the manifest — run `check`")
    dst = a.file or slot_path(a.slot, a.ds_dir)
    if os.path.isfile(dst):
        cur = sha256_file(dst)
        known = {r["sha256"] for r in rows}
        if cur not in known and not a.force:
            raise StatesError(
                f"slot {a.slot} ({dst}) holds a state that is NOT archived (sha256 {cur[:12]}…, "
                f"mtime {dt.datetime.fromtimestamp(os.stat(dst).st_mtime):%Y-%m-%d %H:%M}). "
                f"Archive it first (`states.py archive {a.slot} <id>`) or pass --force to discard it")
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copy2(src, dst)
    print(f"staged {a.id} -> {dst}  (DuckStation: load slot {a.slot}; the file is re-read on load)")
    return 0


def cmd_check(a) -> int:
    rows = load_manifest(a.manifest)
    lib = library_dir()
    problems = 0

    def bad(msg):
        nonlocal problems
        problems += 1
        print("PROBLEM:", msg)

    ids, live = {}, {}
    for r in rows:
        try:
            parse_id(r["id"])
        except StatesError as ex:
            bad(str(ex))
        if r["status"] not in STATUSES:
            bad(f"{r['id']}: status {r['status']!r} not in {STATUSES}")
        if r["status"] != "superseded":
            if r["id"] in live:
                bad(f"duplicate live id {r['id']} (older rows must be 'superseded')")
            live[r["id"]] = r
        ids[r["id"]] = r
    for r in rows:
        if r.get("parent") and r["parent"] not in ids:
            bad(f"{r['id']}: parent {r['parent']} not in the manifest")
        p = os.path.join(lib, r["file"])
        if not os.path.isfile(p):
            bad(f"{r['id']}: missing file {p}")
            continue
        sha = sha256_file(p)
        if sha != r["sha256"]:
            bad(f"{r['id']}: sha256 mismatch ({sha[:12]}… on disk vs {r['sha256'][:12]}… in manifest)")
        else:
            print(f"ok       {r['id']:14s} {r['file']}")
        if not os.path.isfile(os.path.splitext(p)[0] + ".json"):
            print(f"note     {r['id']}: no sidecar json")
    known = {r["file"] for r in rows}
    if os.path.isdir(lib):
        for name in sorted(os.listdir(lib)):
            if name.endswith(".sav") and name not in known:
                bad(f"orphan (in library, not in manifest): {name}")
    print(f"{len(rows)} manifest rows, {problems} problems  (library: {lib})")
    return 1 if problems else 0


def _matches(r: dict, a) -> bool:
    pid = ID_RE.match(r["id"])
    if a.mission is not None:
        if not (pid and pid["ctx"].startswith("M") and int(pid["ctx"][1:]) == a.mission):
            return False
    if a.phase and not (pid and pid["phase"] == a.phase):
        return False
    if a.context and not (pid and pid["ctx"] == a.context):
        return False
    if a.status and r["status"] != a.status:
        return False
    if a.serves:
        ids = {s.strip() for s in r.get("serves", "").split(",") if s.strip()}
        if a.serves not in ids:
            return False
    return True


def cmd_list(a) -> int:
    rows = [r for r in load_manifest(a.manifest) if _matches(r, a)]
    if not rows:
        print("(no matching states)")
        return 0
    cols = ["id", "status", "date", "mission", "stage", "in_mission", "frame", "state_version",
            "disc", "serves", "purpose"]
    widths = {c: max(len(c), *(len(r.get(c, "")) for r in rows)) for c in cols}
    widths["purpose"] = min(widths["purpose"], 60)
    print("  ".join(c.ljust(widths[c]) for c in cols))
    for r in rows:
        print("  ".join((r.get(c, "") or "")[:widths[c]].ljust(widths[c]) for c in cols))
    return 0


def cmd_show(a) -> int:
    row = find_row(load_manifest(a.manifest), a.id)
    for c in COLUMNS:
        print(f"{c:18s} {row.get(c, '')}")
    side = os.path.join(library_dir(), os.path.splitext(row["file"])[0] + ".json")
    if os.path.isfile(side):
        print(f"--- sidecar {side}")
        print(open(side, encoding="utf-8").read())
    return 0


def cmd_refresh(a) -> int:
    """Re-read the automatic (state-derived) columns of existing rows — after a reader
    fix, or once a disc/zstd decoder becomes available. Manual columns are kept."""
    rows = load_manifest(a.manifest)
    lib = library_dir()
    targets = [find_row(rows, a.id)] if a.id else rows
    disc = a.disc or os.environ.get("AC1_BIN")
    for r in targets:
        p = os.path.join(lib, r["file"])
        if not os.path.isfile(p):
            print(f"{r['id']}: missing file {p}")
            continue
        if sha256_file(p) != r["sha256"]:
            print(f"{r['id']}: sha256 mismatch, not refreshing (run `check`)")
            continue
        fields, extra = read_fields(p, disc, lib)
        changed = {c: (r[c], fields[c]) for c in AUTO_COLUMNS if r.get(c, "") != fields[c]}
        r.update({c: fields[c] for c in AUTO_COLUMNS})
        side_path = os.path.splitext(p)[0] + ".json"
        side = {}
        if os.path.isfile(side_path):
            try:
                side = json.load(open(side_path, encoding="utf-8"))
            except Exception:
                side = {}
        side.update(r)
        side.update(extra)
        side["refreshed_at"] = dt.datetime.now().isoformat(timespec="seconds")
        with open(side_path, "w", encoding="utf-8") as f:
            json.dump(side, f, indent=1, default=str)
        print(f"{r['id']}: " + (", ".join(f"{c} {o!r}->{n!r}" for c, (o, n) in changed.items()) or "no change"))
    save_manifest(a.manifest, rows)
    return 0


def cmd_set_status(a) -> int:
    if a.status not in STATUSES:
        raise StatesError(f"status must be one of {STATUSES}")
    rows = load_manifest(a.manifest)
    row = find_row(rows, a.id)
    row["status"] = a.status
    if a.note:
        row["notes"] = (row.get("notes", "") + " " + a.note).strip()
    save_manifest(a.manifest, rows)
    print(f"{a.id}: status -> {a.status}")
    return 0


# ---------------------------------------------------------------- cli
def build_parser() -> argparse.ArgumentParser:
    ap = argparse.ArgumentParser(prog="states.py", description=__doc__.split("Subcommands")[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--manifest", default=MANIFEST_DEFAULT, help=f"TSV manifest (default {MANIFEST_DEFAULT})")
    ap.add_argument("--ds-dir", help="DuckStation data dir (default $AC1_DS_DIR or the platform default)")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("archive", help="copy a slot into the library under a stable id")
    p.add_argument("slot", help="1..10 or 'resume'")
    p.add_argument("id", help="state id, e.g. M01-ctrl-01")
    p.add_argument("--purpose", help="what the state is for (one line)")
    p.add_argument("--serves", help="comma-separated checklist ids, e.g. S-1,LVL-1")
    p.add_argument("--parent", help="id of the state this one was played forward from")
    p.add_argument("--caveats", help="known problems (cheats on, wrong disc, ...)")
    p.add_argument("--note", help="free-text notes column")
    p.add_argument("--status", default="good", help="good|stale|superseded (default good)")
    p.add_argument("--disc", help="disc .bin for RAM anchor + identity (default $AC1_BIN)")
    p.add_argument("--file", help="archive this .sav instead of the slot file (slot is then only recorded)")
    p.add_argument("--replace", action="store_true", help="allow re-using an id; old row -> superseded")
    p.add_argument("--build", help="override the duckstation_build string")
    p.set_defaults(fn=cmd_archive)

    p = sub.add_parser("adopt", help="register an existing .sav (e.g. copied by hand into the library)")
    p.add_argument("path", help="the .sav file; renamed/moved to the canonical name unless --keep-name")
    p.add_argument("id", help="state id, e.g. M30-ctrl-01")
    p.add_argument("--slot", help="slot it was saved from, if known (recorded only)")
    p.add_argument("--date", help="YYYYMMDD for the file name/date column (default: the file's mtime)")
    p.add_argument("--keep-name", action="store_true", help="leave a file already in the library under its current name")
    for flag, hlp in (("--purpose", "what the state is for"), ("--serves", "checklist ids"),
                      ("--parent", "parent state id"), ("--caveats", "known problems"),
                      ("--note", "notes column"), ("--disc", "disc .bin (default $AC1_BIN)"),
                      ("--build", "override the duckstation_build string")):
        p.add_argument(flag, help=hlp)
    p.add_argument("--status", default="good")
    p.add_argument("--replace", action="store_true")
    p.set_defaults(fn=cmd_adopt)

    p = sub.add_parser("stage", help="copy a library state back into a slot")
    p.add_argument("id")
    p.add_argument("slot", help="1..10 or 'resume'")
    p.add_argument("--force", action="store_true", help="overwrite an un-archived slot")
    p.add_argument("--file", help="write here instead of the slot file")
    p.set_defaults(fn=cmd_stage)

    p = sub.add_parser("check", help="verify manifest vs library")
    p.set_defaults(fn=cmd_check)

    p = sub.add_parser("list", help="query the manifest")
    p.add_argument("--mission", type=int)
    p.add_argument("--phase", choices=PHASES)
    p.add_argument("--context", help="M01|GAR|ARN|TTL|DBF")
    p.add_argument("--serves", help="checklist id, e.g. S-1")
    p.add_argument("--status", choices=STATUSES)
    p.set_defaults(fn=cmd_list)

    p = sub.add_parser("show", help="print one row + sidecar")
    p.add_argument("id")
    p.set_defaults(fn=cmd_show)

    p = sub.add_parser("refresh", help="re-read the automatic columns (after a reader fix)")
    p.add_argument("id", nargs="?", help="one id (default: every row)")
    p.add_argument("--disc", help="disc .bin (default $AC1_BIN)")
    p.set_defaults(fn=cmd_refresh)

    p = sub.add_parser("set-status", help="mark a state good|stale|superseded")
    p.add_argument("id")
    p.add_argument("status")
    p.add_argument("--note")
    p.set_defaults(fn=cmd_set_status)
    return ap


def main(argv=None) -> int:
    a = build_parser().parse_args(argv)
    try:
        return a.fn(a)
    except StatesError as ex:
        print("error:", ex, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
