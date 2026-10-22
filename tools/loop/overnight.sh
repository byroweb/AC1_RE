#!/usr/bin/env bash
# overnight.sh - unattended machine pass over every open body: no LLM, CPU only.
#
#   bash tools/loop/overnight.sh            # from the repo root, in Git Bash; ~45-90 min at -j 24
#
# 1. a branch off origin/main (BRANCH=0: stay on the current branch)
# 2. one queue of every open twin group, best credit per effort first (make_queues.py --bodies)
# 3. sweep.py with only the drafts that can have changed since the last pass: ctx-spframe and
#    ctx-fix-frame (new shared headers/names) and neighbour (new matched near-copies)
# 4. permute.py in two passes: QUICK s for every candidate within MAX_RATIO (0.2: the lanes
#    already ground everything within 0.1), then DEEP s for the DEEP_N that came closest
# 5. twins.py replay --scope all, drive.py verify, and a commit when verify says all matched;
#    skipped when no manifest row was added (they cost ~8 min even with nothing new)
#
# Every match is written as it lands, so an interrupted run keeps what it found (rerun from
# step 5 by hand: twins.py replay, drive.py verify, commit).  Knobs (environment variables):
#   J=24 VARIANTS=ctx-spframe,ctx-fix-frame,neighbour MAX_RATIO=0.2 QUICK=120 DEEP=900
#   DEEP_N=50 LIMIT= (rows of the queue, for a test) WORK=~/dev/ac1_work/<stamp> BRANCH=1 COMMIT=1
set -uo pipefail
cd "$(dirname "$0")/../.."
export PYTHONIOENCODING=utf-8       # progress.py prints block characters; the console is cp1252

J=${J:-24}
VARIANTS=${VARIANTS:-ctx-spframe,ctx-fix-frame,neighbour}
MAX_RATIO=${MAX_RATIO:-0.2}
QUICK=${QUICK:-120}
DEEP=${DEEP:-900}
DEEP_N=${DEEP_N:-50}
LIMIT=${LIMIT:-}
STAMP=$(date +%Y%m%d-%H%M)
WORK=${WORK:-$HOME/dev/ac1_work/$STAMP}
BRANCH=${BRANCH:-1}
COMMIT=${COMMIT:-1}
mkdir -p "$WORK"
LOG="$WORK/overnight.log"
exec > >(tee -a "$LOG") 2>&1
step() { echo; echo "== $(date +%T) $*"; }

step "overnight pass: work dir $WORK, -j $J"
if [ "$BRANCH" = 1 ]; then
  git fetch -q origin && git checkout -q -b "machine-pass-$STAMP" origin/main || exit 1
  echo "branch machine-pass-$STAMP"
fi
before=$(python tools/loop/progress.py 2>/dev/null | grep -o '| [0-9]* | [0-9]* |' | tail -1)

rows() { grep -c "^0x" re/match/manifest.tsv; }
rows0=$(rows)

step "queue"
python tools/loop/make_queues.py --bodies -N 1 --prefix all --out-dir "$WORK" ${LIMIT:+--limit $LIMIT} | tail -2

step "sweep ($VARIANTS)"
python tools/loop/sweep.py --queue "$WORK/all1.tsv" --max-size 100000 -j "$J" --variants "$VARIANTS" \
  --work "$WORK/sweep" --no-replay | tail -15

step "permuter, quick pass (${QUICK}s each, words/(size/4) <= $MAX_RATIO)"
python tools/loop/permute.py run --report "$WORK/sweep/sweep_report.tsv" --max-ratio "$MAX_RATIO" \
  -j "$J" --budget "$QUICK" --work "$WORK/perm1" --no-replay | tail -3

step "permuter, deep pass (${DEEP}s each for the $DEEP_N closest)"
python - "$WORK" "$DEEP_N" <<'EOF'
import csv, os, shutil, sys
work, n = sys.argv[1], int(sys.argv[2])
rep = os.path.join(work, "perm1", "permute_report.tsv")
rows = list(csv.DictReader(open(rep), delimiter="\t")) if os.path.exists(rep) else []
def score(r):
    v = [int(x) for x in (r["perm_best"], r["lever"]) if x not in ("", "None")]
    return min(v) if v else None
left = [r for r in rows if r["status"] not in ("MATCH", "crash") and score(r) is not None]
left.sort(key=lambda r: (score(r), int(r["size"])))
os.makedirs(os.path.join(work, "deep"), exist_ok=True)
picked = 0
for r in left[:n]:
    d = os.path.join(work, "perm1", "%s_%s" % (r["unit"], r["vaddr"][2:]))
    src = next((os.path.join(d, f) for f in ("lever.c", "draft.c") if os.path.exists(os.path.join(d, f))), None)
    if not src:
        continue
    dst = os.path.join(work, "deep", r["unit"], "func_%s.c" % r["vaddr"][2:])
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copyfile(src, dst)
    picked += 1
print("%d deep candidate(s)" % picked)
EOF
deep=$(find "$WORK/deep" -name 'func_*.c' 2>/dev/null)
if [ -n "$deep" ]; then
  args=(); for f in $deep; do args+=(--cfile "$f"); done
  python tools/loop/permute.py run "${args[@]}" -j "$J" --budget "$DEEP" --work "$WORK/perm2" --no-replay | tail -3
fi

new=$(( $(rows) - rows0 ))
if [ "$new" -le 0 ]; then
  step "no new matches: replay, verify and commit skipped"
  exit 0
fi

step "twins replay (all scopes): $new new manifest row(s) so far"
python tools/loop/twins.py replay --scope all -j "$J" | grep -E "^(replayed|manifest|ledger)"

step "verify"
vout=$(python tools/loop/drive.py verify -j "$J" -q 2>&1 | tail -3)
echo "$vout"
after=$(python tools/loop/progress.py 2>/dev/null | grep -o '| [0-9]* | [0-9]* |' | tail -1)
echo "progress (functions | matched): before $before, after $after"

if ! echo "$vout" | grep -qE '^([0-9]+)/ matched'; then
  step "verify did not say all matched: nothing committed, see $LOG"
  exit 1
fi
if [ "$COMMIT" = 1 ]; then
  git add re/match tools/loop/ledger.tsv
  git commit -q -m "overnight machine pass $STAMP" -m "tools/loop/overnight.sh: sweep ($VARIANTS) + permuter (${QUICK}s, then ${DEEP}s x $DEEP_N), -j $J.
Progress (functions | matched): before $before, after $after."
  step "committed on $(git branch --show-current): review and open a PR"
else
  step "verified, not committed (COMMIT=0)"
fi
