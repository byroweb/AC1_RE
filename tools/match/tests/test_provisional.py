"""tools/loop/provisional.py: which C counts as asm-steered.

    python -m pytest tools/match/tests/test_provisional.py

- a block-local register pin and an empty asm statement count;
- a gp base pin (asm("$28")), PsyQ GTE macros (non-empty asm) and comments do not;
- the provisional.tsv in the repo is up to date with the manifest.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import provisional  # noqa: E402


def reasons_of(tmp_path, text):
    p = tmp_path / "f.c"
    p.write_text(text, encoding="utf-8")
    return provisional.reasons(str(p))


def test_pins_and_empty_asm_count(tmp_path):
    assert reasons_of(tmp_path, 'void f(void) { register s32 v asm("$5"); v = 1; }') == "register pin x1"
    assert reasons_of(tmp_path, 'void f(int *p) { __asm__("" : "=r"(p) : "0"(p)); }') == "empty asm x1"
    assert reasons_of(tmp_path, 'void f(void) { __asm__ volatile ("" ::: "memory"); }') == "empty asm x1"
    assert reasons_of(tmp_path, 'void f(void) { asm volatile(""); register int a __asm__("$2"); }') \
        == "register pin x1, empty asm x1"


def test_gp_gte_and_comments_do_not(tmp_path):
    assert reasons_of(tmp_path, 'register u8 *gp_base asm("$28");\nvoid f(void) {}') == ""
    assert reasons_of(tmp_path, 'void f(void) { __asm__ volatile ("lwc2 $0, 0(%0)" :: "r"(p)); }') == ""
    assert reasons_of(tmp_path, '/* register s32 v asm("$5"); */ // __asm__("")\nvoid f(void) {}') == ""


def test_repo_list_is_current():
    rows = provisional.compute()
    cur = open(provisional.OUT, encoding="utf-8").read().replace("\r\n", "\n")
    assert cur == provisional.render(rows)
