"""The codegen rules quoted in the match-function skill table still hold.

    python -m pytest tools/match/tests/test_probes.py

Runs tools/match/probes/run_probes.py (26 A/B pairs through match_fn.py).
Skipped without the toolchain from tools/match/setup.sh.
"""
import os
import subprocess
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
MATCH_DIR = os.path.dirname(HERE)
RUNNER = os.path.join(MATCH_DIR, "probes", "run_probes.py")


def test_all_probes_hold():
    if not os.path.isfile(os.path.join(MATCH_DIR, "vendor", "toolchain.json")):
        pytest.skip("tools/match/vendor/toolchain.json missing: run tools/match/setup.sh")
    p = subprocess.run([sys.executable, RUNNER], capture_output=True, text=True)
    assert p.returncode == 0, p.stdout + p.stderr
