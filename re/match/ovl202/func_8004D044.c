#include "ac1/ac1.h"
/* ovl202 0x8004D044 - pick a sound channel for sound id: a free channel
 * (voice -1), else the one already playing id (stopped first), else the
 * idle channel with the lowest priority below the sound's own.
 * The unused vl/vr parameters are reused as the min-volume and loop-end
 * values via (u8)x >> 8 (combine folds it to 0 after flow): the entry copies
 * stay live, so i and prio avoid a1/a2 (regs_someone_prefers) as in retail.
 */

typedef struct {
    u16 busy;       /* +0x00 key | 0x8000 */
    s8 voice;       /* +0x02 */
    u8 vol;         /* +0x03 */
    u8 note;        /* +0x04 */
    u8 fine;        /* +0x05 */
    s16 id;         /* +0x06 */
    s16 prio;       /* +0x08 */
} SndChan;

typedef struct {
    s16 vab;        /* +0x00 */
    s16 prog;       /* +0x02 */
    s16 tone;       /* +0x04 */
    s16 note;       /* +0x06 */
    u16 prio;       /* +0x08 */
} SndDef;

typedef struct {
    s16 id;
    u8 pad[6];
} SndVab;

typedef struct {
    SndVab vabs[150];
    u8 pad[6];
    SndDef defs[1];
} SndBank;

extern SndChan D_80041DE0[23];
extern SndBank D_80041BD0;
extern void func_8004CF0C(SndChan *c);

SndChan *func_8004D044(s32 id, s32 vl, s32 vr)
{
    SndChan *c;
    SndChan *best;
    s32 i;
    s32 prio;

    c = D_80041DE0;
    for (i = 0; i < 23; i++) {
        if (c->voice == -1) {
            return c;
        }
        c++;
    }
    c = D_80041DE0;
    i = 22;
    do {
        if (c->id == id) {
            func_8004CF0C(c);
            return c;
        }
        c++;
    } while (--i != -1);
    prio = D_80041BD0.defs[id].prio;
    {
    vl = (u8)vl >> 8;
    best = 0;
    c = D_80041DE0;
    i = 22;
    vr = ((u8)vr >> 8) - 1;
    do {
        if (c->busy == 0) {
            if (c->prio < prio) {
                prio = c->prio;
                best = c;
            } else if (c->prio == prio && c->vol < vl) {
                vl = c->vol;
                best = c;
            }
        }
        c++;
    } while (--i != vr);
    }
    if (best == 0) {
        return 0;
    }
    func_8004CF0C(best);
    return best;
}
