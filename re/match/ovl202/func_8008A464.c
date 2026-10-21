#include "ac1/ac1.h"

typedef struct {
    s16 unk0;
    s16 x;
    s16 unk4;
    s16 z;
    s32 unk8;
    s32 flags;
} HitShape;

typedef struct {
    s32 pad;
    s32 (*fn)(WorldStruct *, s32, void *);
} HitRec;

typedef struct {
    HitShape *shape;            /* +0x00 0 ends the table */
    WorldStruct *w;             /* +0x04 */
    u8 unk08[0x14];
} HitEntry;

extern HitEntry D_801E8B74[];
extern void func_8001380C(s32, SVECTOR *, s32);

s32 func_8008A464(s32 msg, s32 arg1) {
    SVECTOR v;
    HitEntry *ent;
    HitShape *s;
    WorldStruct *w;
    WorldStruct *prev;
    HitRec *h;
    s32 res;
    s32 acc;

    acc = 0;
    ent = D_801E8B74;
    prev = 0;
    if (ent->shape != 0) {
        do {
            w = ent->w;
            if (w != prev) {
                res = 0;
            }
            if (res == 0) {
                s = ent->shape;
                if ((u32)((s->flags & 7) - 2) < 2) {
                    func_8001380C(arg1, &v, -w->unk16 & 3);
                    if (s->x * v.vx + s->z * v.vz <= 0xDDAFFF) {
                        goto next;
                    }
                }
                h = w->on_hit;
                if (h != 0 && h->fn != 0) {
                    res = h->fn(w, msg, ent);
                } else {
                    res = 0;
                }
                acc |= res;
                prev = w;
            }
        next:
            ent++;
        } while (ent->shape != 0);
    }
    return acc;
}
