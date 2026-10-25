#include "ac1/ac1.h"

typedef struct {
    u16 frame;
    s16 vx, vy, vz;
} AnimKey;

typedef struct {
    s16 total;          /* +4 */
    s16 n;              /* +6 */
    AnimKey keys[1];    /* +8 */
} AnimSub;

typedef struct {
    u8 pad0[4];
    AnimSub s;          /* +4 */
} AnimHdr;

typedef struct AnimBank {
    u8 pad0[0xC];
    u16 ofs[1];         /* +0xC clip offsets from +0xC */
} AnimBank;

typedef struct {
    u8 idx;             /* +0x00 clip */
    u8 pad1[9];
    s16 frame;          /* +0x0A */
    s16 dir;            /* +0x0C */
    u8 padE[0x1C - 0xE];
    AnimBank *bank;     /* +0x1C */
} AnimSt;

s32 func_8004EADC(AnimSt *a)
{
    u8 spare[1];
    AnimHdr *h;
    AnimKey *k;
    AnimSub *s;
    s32 total;
    s32 half;
    s32 n;
    s32 i;
    s32 prev;
    s32 f;

    h = (AnimHdr *)((u8 *)a->bank + (a->bank->ofs[a->idx] + 12));
    s = &h->s;
    total = s->total;
    half = (total + 1) >> 1;
    n = s->n;
    if (a->dir > 0) {
        k = s->keys;
        prev = 0;
        for (i = 0; i < n; i++, k++) {
            f = k->frame;
            if (f >= half) {
                return prev <= a->frame && a->frame <= f;
            }
            prev = f;
        }
    } else if (a->dir < 0) {
        k = &s->keys[n] - 1;
        prev = total;
        for (i = 0; i < n; i++, k--) {
            f = k->frame;
            if (f <= total) {
                return a->frame <= prev && f <= a->frame;
            }
            prev = f;
        }
    }
    return 0;
}
