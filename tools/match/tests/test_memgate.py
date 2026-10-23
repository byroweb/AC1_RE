"""tools/loop/memgate.py: the memory gate and watchdog that keep heavy jobs inside a budget.

    python -m pytest tools/match/tests/test_memgate.py

- system() reads a sane commit limit; tree_bytes() sees a child process's allocation;
- Quote: pessimistic default without calibration, the worst seen peak x margin with it;
- Gate: a job waits while its quote would cut into the headroom and starts on release;
  the watchdog stops the newest attached job when free commit falls below the headroom.
"""
import os
import subprocess
import sys
import threading
import time

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import memgate  # noqa: E402

GB = memgate.GB


def test_system_reads_commit():
    s = memgate.system()
    assert s["commit_limit"] > 0
    assert 0 <= s["commit_free"] <= s["commit_limit"]


def test_tree_bytes_sees_a_child_allocation():
    child = subprocess.Popen([sys.executable, "-c",
                              "import sys,time; b = bytearray(200 * 2**20); sys.stdout.write('ok\\n'); "
                              "sys.stdout.flush(); time.sleep(30)"],
                             stdout=subprocess.PIPE)
    try:
        assert child.stdout.readline().strip() == b"ok"
        assert memgate.tree_bytes(child.pid) >= 180 * memgate.MB
        # the parent's tree includes the child
        assert memgate.tree_bytes(os.getpid()) >= 180 * memgate.MB
    finally:
        memgate.kill_tree(child.pid)
        child.wait(10)


def test_quote_default_then_worst_peak(tmp_path):
    log = str(tmp_path / "memquote.tsv")
    q = memgate.Quote("permute", log)
    assert not q.calibrated()
    assert q.mb(1024, 600) == memgate.DEFAULT_MB[0] + memgate.DEFAULT_MB[1]
    for size, sec, peak in [(256, 300, 30), (512, 300, 600), (1024, 600, 40), (2048, 600, 60)]:
        memgate.append_log({"tool": "permute", "unit": "ovl202", "vaddr": "0x80000000", "size": size,
                            "budget_s": sec, "seconds": sec, "peak_mb": peak}, log)
    memgate.append_log({"tool": "other", "unit": "x", "vaddr": "0x0", "size": 1, "budget_s": 1,
                        "seconds": 1, "peak_mb": 99999}, log)          # other tools' rows are ignored
    q = memgate.Quote("permute", log)
    assert q.calibrated() and len(q.rows) == 4
    assert q.mb(100, 60) == 600 * memgate.MARGIN       # the worst peak, whatever the size
    assert q.mb(4096, 1200) == 600 * memgate.MARGIN
    memgate.append_log({"tool": "permute", "unit": "ovl202", "vaddr": "0x80000004", "size": 64,
                        "budget_s": 60, "seconds": 60, "peak_mb": 10}, log)
    assert memgate.Quote("permute", log).mb(64, 60) == 600 * memgate.MARGIN


class FakeMem:
    def __init__(self, free):
        self.free = free

    def __call__(self):
        return self.free


def test_gate_waits_for_headroom_then_admits():
    mem = FakeMem(10 * GB)
    g = memgate.Gate(headroom=4 * GB, interval=0.05, sample=mem, kill=lambda pid: None)
    try:
        g.admit("a", 4 * GB)                          # 10 - 0 - 4 >= 4
        started = threading.Event()

        def second():
            g.admit("b", 4 * GB)                      # 10 - 4 reserved - 4 = 2 < 4: waits
            started.set()
        t = threading.Thread(target=second, daemon=True)
        t.start()
        time.sleep(0.3)
        assert not started.is_set()
        g.release("a")
        assert started.wait(2)
        g.release("b")
    finally:
        g.close()


def test_used_memory_counts_against_reservation():
    mem = FakeMem(10 * GB)
    g = memgate.Gate(headroom=4 * GB, interval=0.05, sample=mem, kill=lambda pid: None)
    try:
        g.admit("a", 4 * GB)
        g.jobs["a"]["used"] = 3 * GB                 # the job already holds 3 of its 4 GB ...
        mem.free = 7 * GB                            # ... so free commit fell by 3 GB
        assert g._reserved() == 1 * GB
        assert g.fits(2 * GB)                        # 7 - 1 - 2 = 4 >= 4
        assert not g.fits(2.5 * GB)
        g.release("a")
    finally:
        g.close()


def test_watchdog_stops_the_newest_job():
    mem = FakeMem(10 * GB)
    killed = []
    g = memgate.Gate(headroom=4 * GB, interval=0.05, sample=mem, kill=killed.append)
    try:
        g.admit("old", 1 * GB)
        g.attach("old", 111)
        time.sleep(0.01)
        g.admit("new", 1 * GB)
        g.attach("new", 222)
        mem.free = 3 * GB                             # below the headroom
        deadline = time.time() + 2
        while not killed and time.time() < deadline:
            time.sleep(0.02)
        assert killed and killed[0] == 222
        stopped, _peak = g.release("new")
        assert stopped
        assert g.release("old")[0] is False
    finally:
        g.close()
