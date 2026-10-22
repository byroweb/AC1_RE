#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x1C];
    u16 flags;          /* +0x1C bit 13: solid, restore the probe */
    u16 w;              /* +0x1E */
    u16 h;              /* +0x20 */
} HitObj;

extern HitObj *D_8019F308[];
extern s16 D_8019F508;
extern s32 func_80012794(SVECTOR *a, s32 b, s32 c, SVECTOR *d, HitObj *o, s32 w, s32 h);

HitObj *func_800733D8(HitObj *self, SVECTOR *a, s32 b, s32 c, SVECTOR *d)
{
    HitObj **pp;
    HitObj *o;
    HitObj *hit;
    SVECTOR sa;
    SVECTOR sd;
    s32 i;

    hit = 0;
    pp = D_8019F308;
    sa = *a;
    sd = *d;
    i = D_8019F508;
    hit = 0;
    while (--i != -1) {
        o = *pp++;
        if (o == self) {
            continue;
        }
        if (func_80012794(a, b, c, d, o, o->w, o->h) == 0) {
            continue;
        }
        if (d == 0) {
            return o;
        }
        if (o->flags & 0x2000) {
            *a = sa;
            *d = sd;
        } else {
            sa = *a;
            sd = *d;
        }
        hit = o;
    }
    return hit;
}
