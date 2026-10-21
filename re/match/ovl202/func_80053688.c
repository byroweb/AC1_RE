#include "ac1/ac1.h"

typedef struct {                /* 0x1C */
    u8 pad00[0x0E];
    u16 flags;                  /* +0x0E */
    u8 pad10[6];
    u16 idx;                    /* +0x16 */
    u8 pad18[4];
} Rec1C;

typedef struct {
    u32 n;                      /* records per group */
    u32 unk04;
    u32 total;                  /* +0x08 */
    Rec1C rec[1];               /* +0x0C */
} RecSet;

typedef struct {
    u8 pad00[8];
    s32 off;                    /* +0x08 offset of the RecSet */
} RecHdr;

typedef struct {
    u8 *cur;                    /* 0x801A5F30 */
    RecHdr *tbl[1];             /* 0x801A5F34 */
} TexSlots;

extern TexSlots D_801A5F30;
extern void func_800154D8(void *dst, void *src, s32 nwords);

void func_80053688(s32 dslot, s32 sslot) {
    RecSet *s;
    RecSet *d;
    Rec1C *src;
    Rec1C *t;
    Rec1C *dr;
    s32 groups;
    s32 base;
    s32 i;

    s = (RecSet *)((u8 *)D_801A5F30.tbl[sslot] + D_801A5F30.tbl[sslot]->off);
    groups = s->total / s->n;
    d = (RecSet *)((u8 *)D_801A5F30.tbl[dslot] + D_801A5F30.tbl[dslot]->off);
    base = 0;
    src = s->rec;
    while (--groups != -1) {
        t = s->rec;
        i = s->n;
        while (--i != -1) {
            dr = &d->rec[t->idx] + base;
            t++;
            func_800154D8(dr, src, 7);
            src->flags |= 0x2000;
            src++;
            dr->idx = sslot;
            dr->flags |= 0x8000;
        }
        base += d->n;
    }
}
