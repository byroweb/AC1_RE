"""tools/loop/router.py: labels, group-safe splits and the pure-Python learners.

    python -m pytest tools/match/tests/test_router.py

- classify_reason maps every ledger reason shape to its tier;
- group_split never puts two members of one twin group on both sides, in the
  holdout and in k-fold mode, and stratifies by size bucket;
- the boosted model and the CART tree learn a planted rule on synthetic rows;
- the committed labels / features TSVs load, agree on keys, and exclude twins.
"""
import os
import random
import sys

import pytest

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(os.path.dirname(HERE)))
sys.path.insert(0, os.path.join(REPO, "tools", "loop"))
import router  # noqa: E402


def test_classify_reason():
    assert router.classify_reason("zero-agent sweep (m2c ctx)") == (1, "sweep")
    assert router.classify_reason("zero-agent sweep (neighbour+imm of ovl202 0x1, similarity 0.98)") == (1, "sweep")
    assert router.classify_reason("permuter (levers bytes var_v1 += 0x40, ctx draft)") == (1, "levers")
    assert router.classify_reason("permuter (decomp-permuter after levers etype X, ctx draft)") == (2, "permuter")
    assert router.classify_reason("golem (qwen2.5-coder-7b-instruct, tweak)") == (3, "golem")
    assert router.classify_reason("") == (None, "claude")
    assert router.classify_reason("twin of fdat004 0x801C6734 (fdat004/func_801C6734.c)") == (None, "twin")


def _rows(n, seed=3):
    rnd = random.Random(seed)
    rows = []
    for i in range(n):
        g = "g%03d" % (i // 3)                   # groups of three functions
        rows.append({"group": g, "size": rnd.choice([64, 200, 600, 2000]) + (i % 3)})
    return rows


def test_group_split_never_straddles():
    rows = _rows(300)
    fold = router.group_split(rows, 0.2, seed=1)
    sides = {}
    for r in rows:
        sides.setdefault(r["group"], set()).add(fold[r["group"]])
    assert all(len(s) == 1 for s in sides.values())
    test = [r for r in rows if fold[r["group"]] == 0]
    assert 0.1 < len(test) / len(rows) < 0.3
    for name, lo, hi in router.SIZE_BUCKETS:
        b = [r for r in rows if lo <= r["size"] < hi]
        t = [r for r in b if fold[r["group"]] == 0]
        if b:
            assert 0.1 < len(t) / len(b) < 0.35
    kf = router.group_split(rows, 0, seed=2, folds=5)
    for r in rows:
        assert kf[r["group"]] == kf[rows[(rows.index(r) // 3) * 3]["group"]]
    assert set(kf.values()) == {0, 1, 2, 3, 4}


def _planted(n, seed=5):
    rnd = random.Random(seed)
    X, y = [], []
    for _ in range(n):
        a, b, c = rnd.random(), rnd.random(), rnd.random()
        X.append([a, b, c])
        y.append(0 if (a < 0.4 and b > 0.3) else 1 if c < 0.5 else 2)
    return X, y


def test_gbm_and_tree_learn_a_rule():
    X, y = _planted(600)
    Xt, yt = _planted(300, seed=9)
    gbm = router.GBM(3, rounds=40, depth=3).fit(X, y)
    pred = router.decide(gbm.predict_proba(Xt), 3, 0.5)
    assert sum(1 for a, b in zip(yt, pred) if a == b) / len(yt) > 0.9
    tree = router.Tree(3, depth=4, min_leaf=5).fit(X, y)
    pred = router.decide(tree.predict_proba(Xt), 3, 0.5)
    assert sum(1 for a, b in zip(yt, pred) if a == b) / len(yt) > 0.85
    rules = tree.rules(["a", "b", "c"])
    assert any("a <=" in r or "a >" in r for r in rules)


def test_rules_baseline_fits_thresholds():
    names = ["instructions", "jump_tables", "score", "has_draft", "closeness"]
    X = [[i, 0, i / 10.0, 1, 1.0 - i / 400.0] for i in range(10, 200, 2)]
    y = [0 if i <= 40 else 1 for i, *_ in X]
    m = router.Rules(2, names, use_draft=False).fit(X, y)
    assert m.I == 40
    assert router.decide(m.predict_proba([[30, 0, 1, 0, -1], [90, 0, 5, 0, -1]]), 2, 0.5) == [0, 1]


@pytest.mark.skipif(not os.path.isfile(router.LABELS) or not os.path.isfile(router.FEATURES),
                    reason="committed router TSVs missing")
def test_committed_tsvs_agree():
    rows = router.load_dataset(router.LABELS, router.FEATURES)
    assert len(rows) > 1000
    assert all(r["tier"] != "twin" for r in rows)
    labels = router.read_tsv(router.LABELS)
    tiers = {r["tier"] for r in labels}
    assert {"1", "4", "twin"} <= tiers
    assert all(r["label_source"] for r in labels)
    # every labelled, non-twin function has a feature row with words
    assert all(r["instructions"] > 0 for r in rows)
