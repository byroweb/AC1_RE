#!/usr/bin/env python3
"""memgate.py - keep heavy local jobs inside a memory budget.

    memgate.py status                  free commit, headroom, the quote model
    memgate.py quote SIZE [BUDGET]     the quote for one body (bytes, seconds)

Three parts, used by permute.py run (and meant for any tool that starts heavy jobs):

quote     how much commit one job may need: the worst peak in the calibration log,
          plus a margin (peaks did not follow body size or run time); before any
          calibration, a deliberately pessimistic default by body size.
gate      Gate.admit() starts a job only when
              free commit - reserved - quote >= headroom
          where reserved is the part of every running job's quote it has not used
          yet.  Otherwise the job waits.  The worker count (-j) still applies on top.
watchdog  a thread that samples free commit and each job's process tree every
          --interval seconds.  If free commit falls below the headroom, it stops the
          newest job (its kill callback) and marks it, so the caller can requeue it.
          It also records each job's peak, which goes to the calibration log.

Free commit, not free RAM: Windows refuses allocations once the commit charge
reaches the commit limit (RAM + page file), even with physical memory left.  That is
the WinError 1450 / MemoryError that ended two permuter runs at -j 12-20.

The calibration log ($AC1_MEMQUOTE, default ~/dev/ac1_work/memquote.tsv) has one row
per finished job: tool, unit, vaddr, size, budget_s, seconds, peak_mb.  It is local
data (per machine), not part of the repo.

Environment: AC1_MEM_HEADROOM_GB (default 4), AC1_MEMQUOTE.
"""
import argparse
import ctypes
import os
import sys
import threading
import time

GB = 1 << 30
MB = 1 << 20
HEADROOM = float(os.environ.get("AC1_MEM_HEADROOM_GB", "4")) * GB
LOG = os.environ.get("AC1_MEMQUOTE") or os.path.join(os.path.expanduser("~"), "dev", "ac1_work", "memquote.tsv")
LOG_COLS = ["tool", "unit", "vaddr", "size", "budget_s", "seconds", "peak_mb"]
DEFAULT_MB = (1024, 1024)       # no calibration yet: 1 GB + 1 GB per KB of body (pessimistic)
MARGIN = 1.25                   # quote = worst calibrated peak x MARGIN ...
FLOOR_MB = 256                  # ... and at least this
MIN_ROWS = 4                    # fewer calibration rows than this: use the default

IS_WIN = sys.platform == "win32"


# ---------------------------------------------------------------------------
# measuring
# ---------------------------------------------------------------------------
if IS_WIN:
    from ctypes import wintypes

    class _PerfInfo(ctypes.Structure):
        _fields_ = [("cb", wintypes.DWORD), ("CommitTotal", ctypes.c_size_t), ("CommitLimit", ctypes.c_size_t),
                    ("CommitPeak", ctypes.c_size_t), ("PhysicalTotal", ctypes.c_size_t),
                    ("PhysicalAvailable", ctypes.c_size_t), ("SystemCache", ctypes.c_size_t),
                    ("KernelTotal", ctypes.c_size_t), ("KernelPaged", ctypes.c_size_t),
                    ("KernelNonpaged", ctypes.c_size_t), ("PageSize", ctypes.c_size_t),
                    ("HandleCount", wintypes.DWORD), ("ProcessCount", wintypes.DWORD),
                    ("ThreadCount", wintypes.DWORD)]

    class _ProcMem(ctypes.Structure):
        _fields_ = [("cb", wintypes.DWORD), ("PageFaultCount", wintypes.DWORD),
                    ("PeakWorkingSetSize", ctypes.c_size_t), ("WorkingSetSize", ctypes.c_size_t),
                    ("QuotaPeakPagedPoolUsage", ctypes.c_size_t), ("QuotaPagedPoolUsage", ctypes.c_size_t),
                    ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t), ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                    ("PagefileUsage", ctypes.c_size_t), ("PeakPagefileUsage", ctypes.c_size_t),
                    ("PrivateUsage", ctypes.c_size_t)]

    class _ProcEntry(ctypes.Structure):
        _fields_ = [("dwSize", wintypes.DWORD), ("cntUsage", wintypes.DWORD), ("th32ProcessID", wintypes.DWORD),
                    ("th32DefaultHeapID", ctypes.c_size_t), ("th32ModuleID", wintypes.DWORD),
                    ("cntThreads", wintypes.DWORD), ("th32ParentProcessID", wintypes.DWORD),
                    ("pcPriClassBase", ctypes.c_long), ("dwFlags", wintypes.DWORD),
                    ("szExeFile", ctypes.c_wchar * 260)]

    _k32 = ctypes.WinDLL("kernel32", use_last_error=True)
    _k32.OpenProcess.restype = wintypes.HANDLE
    _k32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
    _k32.K32GetPerformanceInfo.argtypes = [ctypes.POINTER(_PerfInfo), wintypes.DWORD]
    _k32.K32GetProcessMemoryInfo.argtypes = [wintypes.HANDLE, ctypes.POINTER(_ProcMem), wintypes.DWORD]
    _k32.Process32FirstW.argtypes = [wintypes.HANDLE, ctypes.POINTER(_ProcEntry)]
    _k32.Process32NextW.argtypes = [wintypes.HANDLE, ctypes.POINTER(_ProcEntry)]
    _k32.CloseHandle.argtypes = [wintypes.HANDLE]


def system():
    """{commit_free, commit_limit, phys_free, phys_total} in bytes."""
    if IS_WIN:
        pi = _PerfInfo()
        pi.cb = ctypes.sizeof(pi)
        if not _k32.K32GetPerformanceInfo(ctypes.byref(pi), pi.cb):
            raise OSError(ctypes.get_last_error(), "GetPerformanceInfo failed")
        pg = pi.PageSize
        return {"commit_free": (pi.CommitLimit - pi.CommitTotal) * pg, "commit_limit": pi.CommitLimit * pg,
                "phys_free": pi.PhysicalAvailable * pg, "phys_total": pi.PhysicalTotal * pg}
    info = {}
    with open("/proc/meminfo") as fh:
        for line in fh:
            k, v = line.split(":", 1)
            info[k] = int(v.split()[0]) * 1024
    # Linux overcommits, so available RAM is the honest limit there
    return {"commit_free": info.get("MemAvailable", 0), "commit_limit": info.get("MemTotal", 0),
            "phys_free": info.get("MemAvailable", 0), "phys_total": info.get("MemTotal", 0)}


def free_commit():
    return system()["commit_free"]


def _children_map():
    """{pid: [child pids]} of every process."""
    kids = {}
    if IS_WIN:
        snap = _k32.CreateToolhelp32Snapshot(2, 0)            # TH32CS_SNAPPROCESS
        if snap in (None, wintypes.HANDLE(-1).value):
            return kids
        try:
            e = _ProcEntry()
            e.dwSize = ctypes.sizeof(e)
            ok = _k32.Process32FirstW(snap, ctypes.byref(e))
            while ok:
                kids.setdefault(e.th32ParentProcessID, []).append(e.th32ProcessID)
                ok = _k32.Process32NextW(snap, ctypes.byref(e))
        finally:
            _k32.CloseHandle(snap)
        return kids
    for d in os.listdir("/proc"):
        if d.isdigit():
            try:
                with open("/proc/%s/stat" % d) as fh:
                    ppid = int(fh.read().rsplit(")", 1)[1].split()[1])
                kids.setdefault(ppid, []).append(int(d))
            except (OSError, ValueError, IndexError):
                pass
    return kids


def _private(pid):
    if IS_WIN:
        h = _k32.OpenProcess(0x1000, False, pid)              # PROCESS_QUERY_LIMITED_INFORMATION
        if not h:
            return 0
        try:
            pm = _ProcMem()
            pm.cb = ctypes.sizeof(pm)
            return pm.PrivateUsage if _k32.K32GetProcessMemoryInfo(h, ctypes.byref(pm), pm.cb) else 0
        finally:
            _k32.CloseHandle(h)
    try:
        with open("/proc/%d/status" % pid) as fh:
            for line in fh:
                if line.startswith("VmRSS:"):
                    return int(line.split()[1]) * 1024
    except OSError:
        pass
    return 0


def tree_bytes(pid, kids=None):
    """Committed (private) bytes of pid and all its descendants."""
    kids = _children_map() if kids is None else kids
    total, todo, seen = 0, [pid], set()
    while todo:
        p = todo.pop()
        if p in seen:
            continue
        seen.add(p)
        total += _private(p)
        todo.extend(kids.get(p, []))
    return total


def kill_tree(pid):
    """Stop pid and its descendants (best effort)."""
    import subprocess
    if IS_WIN:
        subprocess.run(["taskkill", "/T", "/F", "/PID", str(pid)], stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL)
        return
    import signal
    for p in [pid] + _children_map().get(pid, []):
        try:
            os.kill(p, signal.SIGKILL)
        except OSError:
            pass


# ---------------------------------------------------------------------------
# quoting
# ---------------------------------------------------------------------------
def read_log(path=LOG, tool=None):
    rows = []
    if not os.path.isfile(path):
        return rows
    with open(path, encoding="utf-8") as fh:
        head = fh.readline().rstrip("\n").split("\t")
        for line in fh:
            r = dict(zip(head, line.rstrip("\n").split("\t")))
            try:
                if tool and r["tool"] != tool:
                    continue
                rows.append((int(r["size"]), float(r["budget_s"]), float(r["seconds"]), float(r["peak_mb"])))
            except (KeyError, ValueError):
                continue
    return rows


def append_log(row, path=LOG):
    """row: dict with LOG_COLS keys."""
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    new = not os.path.isfile(path)
    with open(path, "a", encoding="utf-8", newline="\n") as fh:
        if new:
            fh.write("\t".join(LOG_COLS) + "\n")
        fh.write("\t".join(str(row[c]) for c in LOG_COLS) + "\n")


class Quote:
    """A job's commit quote from the calibration log: the largest peak any job of this
    tool has reached, x MARGIN (at least FLOOR_MB).  The first calibration (2026-10-23,
    10 permuter jobs, 368 B - 3.4 KB, 90-1200 s) showed no trend with body size or run
    time: most peaks were 31-64 MB, two were 552 and 633 MB.  So a fit would understate
    the outliers; the worst case seen is the honest quote."""

    def __init__(self, tool="permute", path=LOG):
        self.rows = read_log(path, tool)
        self.worst = max((p for *_x, p in self.rows), default=None)

    def calibrated(self):
        return len(self.rows) >= MIN_ROWS

    def mb(self, size, budget_s):
        if not self.calibrated():
            return DEFAULT_MB[0] + DEFAULT_MB[1] * size / 1024.0
        return max(FLOOR_MB, self.worst * MARGIN)

    def bytes(self, size, budget_s):
        return int(self.mb(size, budget_s) * MB)

    def describe(self):
        if not self.calibrated():
            return "default (no calibration: %d rows < %d): %d MB + %d MB per KB" % (
                len(self.rows), MIN_ROWS, DEFAULT_MB[0], DEFAULT_MB[1])
        return "worst peak of %d rows (%.0f MB) x %.2f = %.0f MB per job" % (
            len(self.rows), self.worst, MARGIN, self.mb(0, 0))


# ---------------------------------------------------------------------------
# gate + watchdog
# ---------------------------------------------------------------------------
class Gate:
    """Admission control + watchdog for jobs that each run one process tree.

        g = Gate(log=print)
        g.admit(key, quote_bytes)          # blocks until it fits
        g.attach(key, pid)                 # the job's process (watched, killable)
        ... run ...
        stopped = g.release(key)           # True if the watchdog stopped it
        g.close()
    """

    def __init__(self, headroom=HEADROOM, interval=2.0, log=None, sample=None, kill=kill_tree):
        self.headroom = headroom
        self.interval = interval
        self.log = log or (lambda msg: None)
        self.sample = sample or free_commit           # injectable for tests
        self.kill = kill
        self.cond = threading.Condition()
        self.jobs = {}                                # key -> dict(quote, pid, used, peak, t0, stopped)
        self.closed = False
        self.waiting_logged = set()
        self.thread = threading.Thread(target=self._watch, name="memgate", daemon=True)
        self.thread.start()

    def _reserved(self):
        return sum(max(0, j["quote"] - j["used"]) for j in self.jobs.values())

    def fits(self, quote):
        return self.sample() - self._reserved() - quote >= self.headroom

    def admit(self, key, quote):
        with self.cond:
            while not self.closed and not self.fits(quote):
                if key not in self.waiting_logged:
                    self.waiting_logged.add(key)
                    self.log("memgate: %s waits (quote %.1f GB, free commit %.1f GB, reserved %.1f GB, "
                             "headroom %.1f GB)" % (key, quote / GB, self.sample() / GB, self._reserved() / GB,
                                                    self.headroom / GB))
                self.cond.wait(self.interval)
            self.jobs[key] = {"quote": quote, "pid": None, "used": 0, "peak": 0, "t0": time.time(),
                              "stopped": False}

    def attach(self, key, pid):
        with self.cond:
            if key in self.jobs:
                self.jobs[key]["pid"] = pid

    def detach(self, key):
        """The job's process ended; it keeps its reservation until release()."""
        with self.cond:
            if key in self.jobs:
                self.jobs[key]["pid"] = None

    def peak(self, key):
        with self.cond:
            j = self.jobs.get(key)
            return j["peak"] if j else 0

    def release(self, key):
        """-> (stopped by the watchdog, peak bytes)."""
        with self.cond:
            j = self.jobs.pop(key, None)
            self.waiting_logged.discard(key)
            self.cond.notify_all()
        return (j["stopped"], j["peak"]) if j else (False, 0)

    def _watch(self):
        while not self.closed:
            time.sleep(self.interval)
            try:
                self.tick()
            except Exception as e:  # noqa: BLE001 - the watchdog must keep running
                self.log("memgate: watchdog error %s" % e)

    def tick(self):
        with self.cond:
            live = [(k, j) for k, j in self.jobs.items() if j["pid"]]
        if live:
            kids = _children_map()
            for k, j in live:
                used = tree_bytes(j["pid"], kids)
                j["used"] = used
                j["peak"] = max(j["peak"], used)
        free = self.sample()
        if free < self.headroom:
            with self.cond:
                cands = sorted(((j["t0"], k, j) for k, j in self.jobs.items() if j["pid"] and not j["stopped"]),
                               reverse=True)
            if cands:
                _t0, k, j = cands[0]
                j["stopped"] = True
                self.log("memgate: free commit %.1f GB < headroom %.1f GB: stopping %s (using %.1f GB)"
                         % (free / GB, self.headroom / GB, k, j["used"] / GB))
                self.kill(j["pid"])
            else:
                self.log("memgate: free commit %.1f GB < headroom %.1f GB and none of our jobs is running"
                         % (free / GB, self.headroom / GB))
        with self.cond:
            self.cond.notify_all()

    def close(self):
        self.closed = True
        with self.cond:
            self.cond.notify_all()


# ---------------------------------------------------------------------------
def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("status")
    p = sub.add_parser("quote")
    p.add_argument("size", type=int)
    p.add_argument("budget", type=float, nargs="?", default=300)
    args = ap.parse_args(argv)
    q = Quote()
    if args.cmd == "status":
        s = system()
        print("commit: %.1f GB free of %.1f GB limit; RAM: %.1f GB free of %.1f GB"
              % (s["commit_free"] / GB, s["commit_limit"] / GB, s["phys_free"] / GB, s["phys_total"] / GB))
        print("headroom %.1f GB -> room for new jobs: %.1f GB" % (HEADROOM / GB, max(0, s["commit_free"] - HEADROOM) / GB))
        print("quote model (%s): %s" % (LOG, q.describe()))
        return 0
    print("%d B body, %.0f s budget: quote %.0f MB (%s)" % (args.size, args.budget, q.mb(args.size, args.budget),
                                                            q.describe()))
    return 0


if __name__ == "__main__":
    sys.exit(main())
