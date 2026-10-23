#!/usr/bin/env bash
# overnight.sh - unattended machine pass over every open body: CPU only (no LLM unless GOLEM=1).
#
#   bash tools/loop/overnight.sh            # from the repo root, in Git Bash; a few hours at -j 4
#                                           # (~20-30 min at J=24, but that leaves the PC unusable)
#
# 1. a branch off origin/main (BRANCH=0: stay on the current branch)
# 2. one queue of every open twin group, programs then missions, best credit per effort first
#    (make_queues.py --bodies with twins_programs.tsv, then with twins.tsv)
# 3. sweep.py with only the drafts that can have changed since the last pass: ctx-spframe and
#    ctx-fix-frame (new shared headers/names) and neighbour (new matched near-copies)
# 4. permute.py in two passes: QUICK s for every candidate within MAX_RATIO (0.2: the lanes
#    already ground everything within 0.1), then DEEP s for up to DEEP_N within DEEP_WORDS
#    words (on the first night every gain landed inside 300 s, and those further out stayed
#    hundreds of words off); near misses of 1-3 words are listed in $WORK/nearmiss.tsv for hand matching
#    GOLEM=1: then golem.py tweak (a local LLM, LM Studio at GOLEM_URL) on what the deep pass left
#    within DEEP_WORDS words; ~40 s per body on the GPU, skipped when no server answers
# 5. twins.py replay --scope all, drive.py verify, and a commit when verify says all matched;
#    skipped when no manifest row was added (they cost ~8 min even with nothing new)
#
# Every match is written as it lands, so an interrupted run keeps what it found (rerun from
# step 5 by hand: twins.py replay, drive.py verify, commit).  Knobs (environment variables):
#   J=4 VARIANTS=ctx-spframe,ctx-fix-frame,neighbour MAX_RATIO=0.2 QUICK=120 DEEP=300
#   DEEP_N=50 DEEP_WORDS=12 LIMIT= (rows of the queue, for a test) WORK=~/dev/ac1_work/<stamp> BRANCH=1 COMMIT=1
#   GOLEM=0 GOLEM_MODEL=qwen2.5-coder-7b-instruct GOLEM_URL=http://localhost:1234/v1
set -uo pipefail
cd "$(dirname "$0")/../.."
export PYTHONIOENCODING=utf-8       # progress.py prints block characters; the console is cp1252

J=${J:-4}                           # the permuter stages also pass memgate.py's memory gate
VARIANTS=${VARIANTS:-ctx-spframe,ctx-fix-frame,neighbour}
MAX_RATIO=${MAX_RATIO:-0.2}
QUICK=${QUICK:-120}
DEEP=${DEEP:-300}
DEEP_N=${DEEP_N:-50}
DEEP_WORDS=${DEEP_WORDS:-12}
GOLEM=${GOLEM:-0}
GOLEM_MODEL=${GOLEM_MODEL:-qwen2.5-coder-7b-instruct}
GOLEM_URL=${GOLEM_URL:-http://localhost:1234/v1}
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
python tools/loop/make_queues.py --bodies -N 1 --prefix prog --out-dir "$WORK" ${LIMIT:+--limit $LIMIT} | tail -2
python tools/loop/make_queues.py --bodies -N 1 --prefix mis --twins tools/loop/twins.tsv --out-dir "$WORK" \
  ${LIMIT:+--limit $LIMIT} | tail -2
{ cat "$WORK/prog1.tsv"; tail -n +2 "$WORK/mis1.tsv"; } > "$WORK/all1.tsv"

step "sweep ($VARIANTS)"
python tools/loop/sweep.py --queue "$WORK/all1.tsv" --max-size 100000 -j "$J" --variants "$VARIANTS" \
  --work "$WORK/sweep" --no-replay | tail -15

step "permuter, quick pass (${QUICK}s each, words/(size/4) <= $MAX_RATIO)"
python tools/loop/permute.py run --report "$WORK/sweep/sweep_report.tsv" --max-ratio "$MAX_RATIO" \
  -j "$J" --budget "$QUICK" --work "$WORK/perm1" --no-replay | tail -3

step "permuter, deep pass (${DEEP}s each for up to $DEEP_N within $DEEP_WORDS words)"
python - "$WORK" "$DEEP_N" "$DEEP_WORDS" <<'EOF'
import csv, os, shutil, sys
sys.path.insert(0, "tools/loop")
import hints
work, n, max_words = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
rep = os.path.join(work, "perm1", "permute_report.tsv")
rows = list(csv.DictReader(open(rep), delimiter="\t")) if os.path.exists(rep) else []
def score(r):
    v = [int(x) for x in (r["perm_best"], r["lever"]) if x not in ("", "None")]
    return min(v) if v else None
left = [r for r in rows if r["status"] not in ("MATCH", "crash") and score(r) is not None]
left.sort(key=lambda r: (score(r), int(r["size"])))
# near misses for hand matching: the 1-3 word ones are usually one C-level tweak away
with open(os.path.join(work, "nearmiss.tsv"), "w", newline="\n") as fh:
    fh.write("unit\tvaddr\tsize\twords\n")
    for r in left:
        if score(r) <= 3:
            fh.write("%s\t%s\t%s\t%d\n" % (r["unit"], r["vaddr"], r["size"], score(r)))
left = [r for r in left if score(r) <= max_words]
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
# bodies the viewer's "more deep-pass time" asked for (steer deep), on top of the closest
asked = 0
for unit, vaddr in hints.steer(hints.load(), "deep"):
    dst = os.path.join(work, "deep", unit, "func_%s.c" % vaddr[2:])
    d = os.path.join(work, "perm1", "%s_%s" % (unit, vaddr[2:]))
    src = next((os.path.join(d, f) for f in ("lever.c", "draft.c") if os.path.exists(os.path.join(d, f))), None)
    if not src:
        sd = os.path.join(work, "sweep")
        pre = "%s_%s_" % (unit, vaddr[2:])
        drafts = sorted(f for f in os.listdir(sd) if f.startswith(pre) and f.endswith(".c")) if os.path.isdir(sd) else []
        src = os.path.join(sd, drafts[0]) if drafts else None
    if src and not os.path.exists(dst) and not os.path.exists(os.path.join("re", "match", unit, "func_%s.c" % vaddr[2:])):
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        shutil.copyfile(src, dst)
        asked += 1
print("%d deep candidate(s), %d of them asked for in the viewer" % (picked + asked, asked))
EOF
deep=$(find "$WORK/deep" -name 'func_*.c' 2>/dev/null)
if [ -n "$deep" ]; then
  args=(); for f in $deep; do args+=(--cfile "$f"); done
  python tools/loop/permute.py run "${args[@]}" -j "$J" --budget "$DEEP" --work "$WORK/perm2" --no-replay | tail -3
fi

if [ "$GOLEM" = 1 ] && [ -f "$WORK/perm2/permute_report.tsv" ]; then
  step "golem: $GOLEM_MODEL tweaks the deep pass's leftovers within $DEEP_WORDS words"
  if curl -sf -m 5 "$GOLEM_URL/models" >/dev/null; then
    python tools/loop/golem.py run --report "$WORK/perm2/permute_report.tsv" --job tweak --max-words "$DEEP_WORDS" \
      --model "$GOLEM_MODEL" --url "$GOLEM_URL" --log "$WORK/golem_log.tsv" | tail -3
  else
    echo "no model server at $GOLEM_URL: skipped"
  fi
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

if ! echo "$vout" | grep -qE '^([0-9]+)/\1 matched'; then
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
