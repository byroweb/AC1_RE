#include "ac1/ac1.h"

typedef struct {
    s32 pad;
    s32 (*fn)(void *, s32, s32, s32, s32);
} ObjHandler;

typedef struct {
    u8 unk00[0x14];
    ObjHandler *handler;        /* +0x14 */
} HitObj;

extern void func_80014728(s32, SVECTOR *);
extern AcEntity *func_80074620(AcEntity *, SVECTOR *, s32, s32, s32);
extern HitObj *func_800733D8(s32, SVECTOR *, s32, s32, s32);
extern void func_80071014(SVECTOR *, s32, s32, SVECTOR *, void *, s32, s32);
extern s32 func_8008A464(s32, SVECTOR *);

s32 func_8008A5CC(SVECTOR *pos, SVECTOR *rot) {
    SVECTOR p;
    SVECTOR d;
    u8 hit[8];
    SVECTOR d2;
    AcEntity *e;
    HitObj *o;
    ObjHandler *h;

    func_80014728(rot->vy, &d);
    p.vx = pos->vx + ((d.vx * 125) >> 11);
    p.vz = pos->vz + ((d.vz * 125) >> 11);
    p.vy = pos->vy;
    e = func_80074620(g_ac_entities, &p, 200, 575, 0);
    if (e != 0) {
        if (e->fn_move != 0) {
            return ((s32 (*)(AcEntity *, s32))e->fn_move)(e, 0x8100);
        }
        return 0;
    }
    o = func_800733D8(0, &p, 200, 575, 0);
    if (o != 0) {
        h = o->handler;
        if (h != 0) {
            if (h->fn != 0) {
                return h->fn(o, 0, 0, 0, 0);
            }
            return 0;
        }
        return 0;
    }
    d2 = d;
    d.vx >>= 5;
    d.vz >>= 5;
    func_80071014(&p, 200, 575, &d, hit, -1, 7);
    return func_8008A464(0x8100, &d2);
}
