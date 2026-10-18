#!/usr/bin/env python3
"""
states_test.py — end-to-end test for tools/duckstation/states.py on SYNTHETIC data.

No real save state or disc is needed (none exists on a fresh PC): the test builds
  * a fake MODE2/2352 "disc" carrying a PS-X EXE header + payload (RAM anchor),
  * DUCCS states with the real header layout, a System/CPU marker prefix, a 2 MB
    RAM image with chosen bytes at the manifest's RAM addresses, and an RGBA8
    thumbnail — compressed with zstd when a decoder is present, deflate otherwise,
and drives archive / stage / check / list / set-status through a temp library,
temp DuckStation dir and temp manifest. Nothing touches the real library or the
committed STATES.tsv. Exit code is nonzero on any failure.

Values written into the fake RAM are arbitrary test inputs, not RE findings.
"""
import contextlib, io, json, os, struct, sys, tempfile, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import savestate as SS   # noqa: E402
import states as ST      # noqa: E402

T_ADDR = 0x80011000      # PS-X EXE header +0x18 (load address); +0x10 is the entry PC
PC0 = 0x80011e6c
FAILS = []


def check(cond, msg):
    print(("ok   " if cond else "FAIL ") + msg)
    if not cond:
        FAILS.append(msg)


def compress(data: bytes, ctype: int) -> bytes:
    if ctype == SS.COMPRESSION_ZSTD:
        import zstandard
        return zstandard.ZstdCompressor(level=3).compress(data)
    if ctype == SS.COMPRESSION_DEFLATE:
        return zlib.compress(data, 6)
    return data


def fake_disc(path: str) -> bytes:
    """Minimal raw-sector image: sector 1 = PS-X EXE header, payload starts +0x800."""
    RAW, OFF, DS = 2352, 24, 2048
    payload = bytes((i * 37 + 11) & 0xff for i in range(0x800))
    head = bytearray(DS * 2)
    head[0:8] = b"PS-X EXE"
    struct.pack_into("<IIII", head, 0x10, PC0, 0, T_ADDR, len(payload))
    head[0x800:0x800 + len(payload)] = payload
    data = bytearray(RAW * 4)
    for k in range(2):
        s = 1 + k
        data[s * RAW + OFF: s * RAW + OFF + DS] = head[k * DS:(k + 1) * DS]
    open(path, "wb").write(data)
    return payload[:256]


def fake_state(path: str, anchor: bytes, *, mission=0, stage=0, objobj=0, flags=0, result=0,
               frame=1234, version=83, ctype=None, shot=(8, 6)):
    ctype = ctype if ctype is not None else (SS.COMPRESSION_ZSTD if SS.zstd_backend() else SS.COMPRESSION_DEFLATE)
    sysblk = (b"\x06\x00\x00\x00System" + bytes([2]) + struct.pack("<I", 0)
              + struct.pack("<II", frame, frame + 7) + bytes(16)
              + b"\x03\x00\x00\x00CPU" + bytes(64) + b"\x03\x00\x00\x00Bus" + bytes(0x40))
    ram = bytearray(SS.RAM_SIZE)
    o = T_ADDR - SS.PS1_RAM_BASE
    ram[o:o + len(anchor)] = anchor
    ram[0x800411F8 - SS.PS1_RAM_BASE] = mission
    ram[0x8004121B - SS.PS1_RAM_BASE] = stage
    struct.pack_into("<I", ram, 0x8019F51C - SS.PS1_RAM_BASE, objobj)
    struct.pack_into("<I", ram, 0x8019F524 - SS.PS1_RAM_BASE, flags)
    struct.pack_into("<I", ram, 0x80048610 - SS.PS1_RAM_BASE, result)
    blob = sysblk + bytes(ram) + bytes(1024)
    cdata = compress(blob, ctype)

    w, h = shot
    pix = bytes(b for y in range(h) for x in range(w) for b in (x * 30, y * 40, 128, 255))
    cshot = compress(pix, ctype)
    media = b"C:\\fake\\Armored Core (USA) (v1.1).bin"

    hdr = bytearray(SS.HEADER_SIZE)
    struct.pack_into("<I", hdr, 0x00, SS.MAGIC)
    struct.pack_into("<I", hdr, 0x04, version)
    hdr[0x08:0x08 + 22] = b"Armored Core (Reprint)"
    hdr[0x88:0x88 + 10] = b"SLUS-01323"
    pos = SS.HEADER_SIZE
    struct.pack_into("<II", hdr, 0xa8, len(media), pos); pos += len(media)
    struct.pack_into("<I", hdr, 0xb0, 0)
    struct.pack_into("<IIIII", hdr, 0xb4, ctype, w, h, len(cshot), pos); pos += len(cshot)
    struct.pack_into("<IIII", hdr, 0xc8, ctype, len(cdata), len(blob), pos)
    open(path, "wb").write(bytes(hdr) + media + cshot + cdata)
    return pix


def run(*argv) -> tuple:
    buf = io.StringIO()
    with contextlib.redirect_stdout(buf), contextlib.redirect_stderr(buf):
        rc = ST.main(list(argv))
    return rc, buf.getvalue()


def decode_png(path: str):
    d = open(path, "rb").read()
    assert d[:8] == b"\x89PNG\r\n\x1a\n"
    w, h = struct.unpack_from(">II", d, 16)
    pos, idat = 8, b""
    while pos < len(d):
        n, tag = struct.unpack_from(">I4s", d, pos)
        if tag == b"IDAT":
            idat += d[pos + 8: pos + 8 + n]
        pos += 12 + n
    raw = zlib.decompress(idat)
    rows = [raw[y * (w * 4 + 1) + 1: (y + 1) * (w * 4 + 1)] for y in range(h)]
    return w, h, b"".join(rows)


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        lib = os.path.join(td, "lib")
        ds = os.path.join(td, "ds")
        os.makedirs(os.path.join(ds, "savestates"))
        with open(os.path.join(ds, "settings.ini"), "w") as f:
            f.write("[AutoUpdater]\nLastVersion = abcdef0123456789\n")
        disc = os.path.join(td, "disc.bin")
        anchor = fake_disc(disc)
        man = os.path.join(td, "STATES.tsv")
        os.environ["AC1_STATES_DIR"] = lib
        os.environ["AC1_DS_DIR"] = ds
        os.environ["AC1_BIN"] = disc
        M = ["--manifest", man]

        def slot(n):
            return os.path.join(ds, "savestates", f"SLUS-01323_{n}.sav")

        print(f"zstd backend: {SS.zstd_backend()}")

        # --- archive -------------------------------------------------------
        pix = fake_state(slot(3), anchor, mission=1, stage=5, objobj=0x801C4B40, flags=0x80,
                         result=0, frame=4321)
        rc, out = run(*M, "archive", "3", "M01-ctrl-01", "--purpose", "first frame of control",
                      "--serves", "S-1,LVL-1", "--note", "synthetic")
        check(rc == 0, f"archive rc=0\n{out if rc else ''}")
        rows = ST.load_manifest(man)
        check(len(rows) == 1 and rows[0]["id"] == "M01-ctrl-01", "manifest has the row")
        r = rows[0]
        check(r["file"].startswith("M01-ctrl-01__") and r["file"].endswith("__v83.sav"),
              f"file name convention ({r['file']})")
        check(r["mission"] == "1" and r["stage"] == "5", f"mission/stage read from RAM ({r['mission']},{r['stage']})")
        check(r["objobj"] == "0x801c4b40" and r["flags"] == "0x80" and r["result"] == "0x0",
              f"objobj/flags/result ({r['objobj']},{r['flags']},{r['result']})")
        check(r["in_mission"] == "yes", "in_mission yes")
        check(r["frame"] == "4321", f"frame number ({r['frame']})")
        check(r["state_version"] == "83", "state_version from +0x04")
        check(r["disc"] == ST.sha256_file(disc)[:12], "disc identity = sha256 prefix")
        check("abcdef01" in r["duckstation_build"], f"duckstation build ({r['duckstation_build']})")
        check(r["sha256"] == ST.sha256_file(os.path.join(lib, r["file"])), "sha256 matches library file")
        check(r["serves"] == "S-1,LVL-1" and r["purpose"] == "first frame of control", "manual fields")
        stem = os.path.join(lib, os.path.splitext(r["file"])[0])
        check(os.path.isfile(stem + ".json"), "sidecar json written")
        side = json.load(open(stem + ".json"))
        check(side["header"]["media_path"].endswith(".bin") and side["ram_base"] > 0, "sidecar has header + ram_base")
        check(0 < side["ram_base"] - side["bus_marker"] < 0x200 and not side["warnings"],
              f"ram_base sits just after the Bus marker, no warnings ({side['warnings']})")
        check(os.path.isfile(stem + ".png"), "thumbnail png written")
        w, h, got = decode_png(stem + ".png")
        check((w, h) == (8, 6) and got == pix, "thumbnail pixels round-trip")

        # refusals
        rc, out = run(*M, "archive", "3", "M01-ctrl-01")
        check(rc == 2 and "already" in out, "duplicate id refused")
        rc, out = run(*M, "archive", "3", "M99-ctrl-01")
        check(rc == 2 and "bad state id" in out, "M99 refused")
        rc, out = run(*M, "archive", "3", "M01-boss-01")
        check(rc == 2, "unknown phase refused")
        rc, out = run(*M, "archive", "3", "M01-ctrl-02", "--parent", "M01-mid-09")
        check(rc == 2 and "parent" in out, "dangling --parent refused")
        rc, out = run(*M, "archive", "4", "M01-ctrl-02")
        check(rc == 2 and "no state" in out, "empty slot refused")

        # second state, same slot, with parent
        fake_state(slot(3), anchor, mission=1, stage=5, objobj=0x801C4B40, flags=0x480, frame=9000)
        rc, out = run(*M, "archive", "3", "M01-exit-01", "--parent", "M01-ctrl-01", "--serves", "MIS-16")
        check(rc == 0, "second archive")
        rows = ST.load_manifest(man)
        check(len(rows) == 2 and rows[1]["parent"] == "M01-ctrl-01" and rows[1]["flags"] == "0x480", "second row")

        # --- list ---------------------------------------------------------
        rc, out = run(*M, "list", "--mission", "1")
        check(rc == 0 and "M01-ctrl-01" in out and "M01-exit-01" in out, "list --mission 1")
        rc, out = run(*M, "list", "--serves", "S-1")
        check("M01-ctrl-01" in out and "M01-exit-01" not in out, "list --serves S-1")
        rc, out = run(*M, "list", "--phase", "load")
        check("no matching" in out, "list --phase load empty")
        rc, out = run(*M, "show", "M01-exit-01")
        check(rc == 0 and "sidecar" in out, "show")

        # --- check ---------------------------------------------------------
        rc, out = run(*M, "check")
        check(rc == 0 and "0 problems" in out, "check clean")
        p = os.path.join(lib, rows[0]["file"])
        orig = open(p, "rb").read()
        open(p, "wb").write(orig[:-1] + bytes([orig[-1] ^ 1]))
        rc, out = run(*M, "check")
        check(rc == 1 and "sha256 mismatch" in out, "check catches corruption")
        open(p, "wb").write(orig)
        open(os.path.join(lib, "GAR-main-77__20000101__v1.sav"), "wb").write(b"x")
        rc, out = run(*M, "check")
        check(rc == 1 and "orphan" in out, "check catches orphans")
        os.remove(os.path.join(lib, "GAR-main-77__20000101__v1.sav"))

        # --- stage ---------------------------------------------------------
        rc, out = run(*M, "stage", "M01-ctrl-01", "7")
        check(rc == 0 and ST.sha256_file(slot(7)) == rows[0]["sha256"], "stage into empty slot")
        rc, out = run(*M, "stage", "M01-exit-01", "7")
        check(rc == 0 and ST.sha256_file(slot(7)) == rows[1]["sha256"], "stage over an archived slot (no --force needed)")
        fake_state(slot(8), anchor, mission=2, stage=9, frame=1)      # un-archived scratch
        rc, out = run(*M, "stage", "M01-ctrl-01", "8")
        check(rc == 2 and "NOT archived" in out, "stage refuses to clobber an un-archived slot")
        rc, out = run(*M, "stage", "M01-ctrl-01", "8", "--force")
        check(rc == 0 and ST.sha256_file(slot(8)) == rows[0]["sha256"], "stage --force")

        # --- set-status ----------------------------------------------------
        rc, out = run(*M, "set-status", "M01-ctrl-01", "stale", "--note", "disc swapped")
        rows = ST.load_manifest(man)
        check(rc == 0 and rows[0]["status"] == "stale" and "disc swapped" in rows[0]["notes"], "set-status")
        rc, out = run(*M, "set-status", "M01-ctrl-01", "bogus")
        check(rc == 2, "bad status refused")

        # --- degraded: no disc -----------------------------------------------
        del os.environ["AC1_BIN"]
        fake_state(slot(1), anchor, mission=0, stage=0, frame=77)
        rc, out = run(*M, "archive", "1", "GAR-main-01", "--purpose", "garage")
        rows = ST.load_manifest(man)
        g = rows[-1]
        check(rc == 0 and g["id"] == "GAR-main-01", "archive without a disc still succeeds")
        check(g["mission"] == "?" and g["disc"] == "?" and g["in_mission"] == "?", "RAM/disc fields unknown without a disc")
        check(g["frame"] == "77" and g["state_version"] == "83", "frame/version still read without a disc")
        os.environ["AC1_BIN"] = disc

        # --- degraded: no zstd decoder ------------------------------------------
        if SS.zstd_backend():
            fake_state(slot(2), anchor, mission=3, stage=2, frame=5, ctype=SS.COMPRESSION_ZSTD)
            real = SS.zstd_backend
            SS.zstd_backend = lambda: None
            try:
                rc, out = run(*M, "archive", "2", "M03-load-01")
            finally:
                SS.zstd_backend = real
            rows = ST.load_manifest(man)
            z = rows[-1]
            check(rc == 0 and z["id"] == "M03-load-01" and "no zstd decoder" in out, "archive without zstd decoder succeeds with a warning")
            check(z["mission"] == "?" and z["frame"] == "?" and z["state_version"] == "83" and z["disc"] != "?",
                  "without zstd: header + disc known, blob fields unknown")
            check(not os.path.isfile(os.path.join(lib, os.path.splitext(z["file"])[0] + ".png")),
                  "without zstd: no thumbnail (zstd-compressed)")
        else:
            print("skip zstd-degradation case (no zstd backend installed; deflate path exercised instead)")

        # --- deflate-compressed state is readable regardless of zstd -------------
        fake_state(slot(5), anchor, mission=7, stage=1, objobj=1, frame=12, ctype=SS.COMPRESSION_DEFLATE)
        rc, out = run(*M, "archive", "5", "M07-mid-01")
        rows = ST.load_manifest(man)
        check(rc == 0 and rows[-1]["mission"] == "7" and rows[-1]["frame"] == "12", "deflate-compressed state read via stdlib")

        # --- replace ------------------------------------------------------------
        rc, out = run(*M, "archive", "5", "M07-mid-01", "--replace")
        rows = ST.load_manifest(man)
        same = [r for r in rows if r["id"] == "M07-mid-01"]
        check(rc == 0 and len(same) == 2 and same[0]["status"] == "superseded" and same[1]["status"] == "good",
              "--replace supersedes the old row")
        check(same[0]["file"] != same[1]["file"], "replacement gets a distinct file name")
        rc, out = run(*M, "check")
        check(rc == 0, f"check still clean (superseded duplicate allowed)\n{out if rc else ''}")
        rc, out = run(*M, "stage", "M07-mid-01", "9")
        check(rc == 0 and ST.sha256_file(slot(9)) == same[1]["sha256"], "stage picks the live (non-superseded) row")
        same[1]["status"] = "good"; same[0]["status"] = "good"
        ST.save_manifest(man, rows)
        rc, out = run(*M, "check")
        check(rc == 1 and "duplicate live id" in out, "check flags two live rows with one id")
        same[0]["status"] = "superseded"
        ST.save_manifest(man, rows)

        # --- adopt ------------------------------------------------------------
        hand = os.path.join(lib, "M05-load-01__20261001__v94.sav")      # hand-copied, mis-versioned name
        fake_state(hand, anchor, mission=5, stage=3, frame=3)
        os.utime(hand, (1759300000, 1759300000))
        import datetime as _dt
        mt = _dt.datetime.fromtimestamp(1759300000)                      # local date of that mtime
        rc, out = run(*M, "adopt", hand, "M05-load-01", "--slot", "2", "--purpose", "hand copy")
        rows = ST.load_manifest(man)
        ad = rows[-1]
        check(rc == 0 and ad["id"] == "M05-load-01" and ad["mission"] == "5", "adopt registers a hand-copied file")
        check(ad["file"] == f"M05-load-01__{mt:%Y%m%d}__v83.sav" and not os.path.exists(hand)
              and os.path.isfile(os.path.join(lib, ad["file"])), f"adopt renames to the canonical name ({ad['file']})")
        check(ad["date"] == f"{mt:%Y-%m-%d}", "adopt dates the row by file mtime")
        outside = os.path.join(td, "elsewhere.sav")
        fake_state(outside, anchor, mission=6, frame=4)
        rc, out = run(*M, "adopt", outside, "M06-load-01", "--date", "20260102")
        rows = ST.load_manifest(man)
        check(rc == 0 and rows[-1]["file"] == "M06-load-01__20260102__v83.sav" and os.path.isfile(outside),
              "adopt copies a file from outside the library")
        rc, out = run(*M, "adopt", outside, "M06-load-02")
        check(rc == 2 and "already archived" in out, "adopt refuses a file whose sha256 is already archived")
        rc, out = run(*M, "check")
        check(rc == 0, f"check clean after adopt\n{out if rc else ''}")

        # --- refresh ------------------------------------------------------------
        rows = ST.load_manifest(man)
        rows[0]["mission"] = "?"; rows[0]["frame"] = "?"
        ST.save_manifest(man, rows)
        rc, out = run(*M, "refresh", "M01-ctrl-01")
        rows = ST.load_manifest(man)
        check(rc == 0 and rows[0]["mission"] == "1" and rows[0]["frame"] == "4321" and "mission '?'->'1'" in out,
              "refresh re-reads automatic columns")
        check(rows[0]["purpose"] == "first frame of control" and rows[0]["status"] == "stale", "refresh keeps manual columns")
        rc, out = run(*M, "refresh")
        check(rc == 0 and out.count("no change") >= 3, "refresh all: no spurious changes")

        # --- library-in-repo guard --------------------------------------------
        os.environ["AC1_STATES_DIR"] = os.path.join(ST.REPO, "disc_map", "states")
        try:
            ST.library_dir()
            check(False, "library inside repo refused")
        except ST.StatesError:
            check(True, "library inside repo refused")
        os.environ["AC1_STATES_DIR"] = lib

        # --- savestate.py CLI --header on the synthetic file -----------------
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            SS._main([slot(3), "--header"])
        check("SLUS-01323" in buf.getvalue() and "version" in buf.getvalue(), "savestate.py --header")

    print(f"\n{len(FAILS)} failure(s)")
    return 1 if FAILS else 0


if __name__ == "__main__":
    sys.exit(main())
