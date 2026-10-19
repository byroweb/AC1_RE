#!/usr/bin/env python3
"""pick_calibration.py RANK_TSV OUT_DIR - the 2026-10-18 harder calibration set.

From `rank.py --tsv -n 100000` output: exe + ovl201..204, size <= 1200, <= 2 jump
tables, no feature-identical twin anywhere (shared code would be a free second match).
Four score bands 5-6.5 / 6.5-8 / 8-9.5 / 9.5-11 (6/8/6/4 rows) with GTE and
jump-table quotas, evenly spaced by score, dealt snake-wise into queue-C1..C3.tsv.
"""
import sys, csv
rows = list(csv.DictReader(open(sys.argv[1]), delimiter='\t'))
hdr = list(rows[0].keys())
UNITS = {'exe','ovl201','ovl202','ovl203','ovl204'}
pool = [r for r in rows if r['unit'] in UNITS and int(r['size']) <= 1200 and int(r['jump_tables']) <= 2]
BANDS = [(5.0,6.5,6,0,0),(6.5,8.0,8,1,2),(8.0,9.5,6,2,1),(9.5,11.0,4,1,1)]  # lo,hi,n,n_gte,n_jt
picked = []
FEAT = ('size','instructions','branches','calls','jump_tables','gte_ops','stack_size')
def fkey(r): return tuple(r[f] for f in FEAT)
import collections
twins = collections.Counter(fkey(r) for r in rows)
pool = [r for r in pool if twins[fkey(r)] == 1]     # drop anything with a feature-identical twin anywhere
def spaced(cands, k, taken):
    cands = [c for c in cands if (c['unit'],c['vaddr']) not in taken]
    cands.sort(key=lambda r: float(r['score']))
    out = []
    if k <= 0 or not cands: return out
    for i in range(k):
        idx = int((i + 0.5) * len(cands) / k)
        c = cands[min(idx, len(cands)-1)]
        if c not in out: out.append(c)
    return out
taken = set()
for lo, hi, n, ng, nj in BANDS:
    band = [r for r in pool if lo <= float(r['score']) < hi]
    sel = []
    sel += spaced([r for r in band if int(r['gte_ops'])>0 and int(r['jump_tables'])==0], ng, taken)
    taken |= {(r['unit'],r['vaddr']) for r in sel}
    sel += spaced([r for r in band if int(r['jump_tables'])>0 and int(r['gte_ops'])==0], nj, taken)
    taken |= {(r['unit'],r['vaddr']) for r in sel}
    # rest: plain, round-robin over units
    rest = n - len(sel)
    plain = [r for r in band if int(r['gte_ops'])==0 and int(r['jump_tables'])==0]
    units = sorted(UNITS)
    per = {u: [r for r in plain if r['unit']==u] for u in units}
    i = 0
    while rest > 0:
        u = units[(i + int(lo*10)) % len(units)]; i += 1
        got = spaced(per[u], 1 + (i // len(units)), taken)
        got = [g for g in got if (g['unit'],g['vaddr']) not in taken][:1]
        if got:
            sel += got; taken.add((got[0]['unit'],got[0]['vaddr'])); rest -= 1
        if i > 50: break
    picked += sel
picked.sort(key=lambda r: float(r['score']))
queues = {'C1':[], 'C2':[], 'C3':[]}
order = ['C1','C2','C3','C3','C2','C1']   # snake deal keeps the profiles even
for k, r in enumerate(picked):
    queues[order[k % 6]].append(r)
for q, rs in queues.items():
    with open(sys.argv[2] + '/queue-%s.tsv' % q, 'w', newline='\n') as f:
        f.write('\t'.join(hdr) + '\n')
        for r in rs: f.write('\t'.join(r[h] for h in hdr) + '\n')
    print(q, ' '.join('%s:%s(%s,%sB,g%s,j%s)' % (r['unit'], r['vaddr'][-5:], r['score'], r['size'], r['gte_ops'], r['jump_tables']) for r in rs))
