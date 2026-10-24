#include "ac1/ac1.h"

typedef struct {
    s16 min[3];         /* +0x00 */
    s16 id;             /* +0x06 */
    s16 max[3];         /* +0x08 */
    s16 pad0e;
    s16 org[3];         /* +0x10 */
    u8 pad16[0x10];
    s16 n_parts;        /* +0x26 */
    u16 *parts;         /* +0x28 */
    u8 pad2c[4];
    u32 flags;          /* +0x30 */
    u8 pad34[0x0C];
} Ws;

#define ABS(x) ((x) >= 0 ? (x) : -(x))

extern Ws D_801D0B68[];
extern s32 D_801E8B70;
extern void func_8006DDC4(SVECTOR *v, u32 *out);
extern s32 func_8006FB6C(void *bits, ...);

Ws *func_8006E380(Ws *cur, SVECTOR *p)
{
    Ws *best;
    Ws *w;
    s32 bestv;
    s32 idx;
    s32 score;
    u16 *parts;
    s32 i;
    s32 n;
    s32 d;
    s32 bestd;
    s32 dx;
    s32 dx_b;
    s32 dy;
    s32 dz;

    func_8006DDC4(p, (u32 *)0x1F800000);
    idx = func_8006FB6C((void *)0x1F800000, D_801E8B70);
    best = 0;
    bestv = 0x10000;
    for (; idx != -1; idx = func_8006FB6C(0)) {
        w = &D_801D0B68[idx];
        if (w->flags & 0x200) {
            continue;
        }
        if (p->vx < w->min[0] || p->vx > w->max[0]) {
            continue;
        }
        if (p->vy < w->min[1] || p->vy > w->max[1]) {
            continue;
        }
        if (p->vz < w->min[2] || p->vz > w->max[2]) {
            continue;
        }
        if (cur == 0) {
            return w;
        }
        if (!(w->flags & 0x4000)) {
            score = 0;
            if (w == cur) {
                return w;
            }
        } else {
            score = 1024;
        }
        parts = cur->parts;
        for (i = 1; i <= cur->n_parts; i++) {
            if ((*parts & 0x1FF) == idx) {
                score += i;
                goto found;
            }
            parts++;
        }
        score = 16384;
    found:
        if (score < bestv) {
            bestv = score;
            best = w;
        }
    }
    if (best != 0) {
        return best;
    }
    if (cur != 0) {
        w = cur;
        bestd = 0x7FFFFFFF;
        i = cur->n_parts;
        parts = cur->parts;
        do {
            if (!(w->flags & 0x200)) {
                dx = w->org[0] - p->vx;
                if (dx < 0) {
                    dx = -dx;
                }
                dy = w->org[1] - p->vy;
                if (dy < 0) {
                    dy = -dy;
                }
                dz = w->org[2] - p->vz;
                if (dz < 0) {
                    dz = -dz;
                }
                score = dx + dy + dz;
                if (score < bestd) {
                    best = w;
                    bestd = score;
                }
            }
            w = &D_801D0B68[*parts++ & 0x1FF];
        } while (--i != -1);
        return best;
    }
    i = g_world_struct_count;
    d = 0x7FFFFFFF;
    w = D_801D0B68;
    do {
        if (w->id != -1 && !(w->flags & 0x200)) {
            dx_b = w->org[0] - p->vx;
            if (dx_b < 0) {
                dx_b = -dx_b;
            }
            dy = w->org[1] - p->vy;
            if (dy < 0) {
                dy = -dy;
            }
            dz = w->org[2] - p->vz;
            if (dz < 0) {
                dz = -dz;
            }
            score = dx_b + dy + dz;
            if (score < d) {
                best = w;
                d = score;
            }
        }
        w++;
    } while (--i);
    return best;
}
