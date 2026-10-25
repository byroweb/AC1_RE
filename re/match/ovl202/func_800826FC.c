#include "ac1/ac1.h"
/* ovl202 port of ovl203 0x80081358: per-frame AC step: engine sound, then probe the ground under four
 * feet (func_80082654), drop the highest, and tilt the body to the plane of
 * the other three. */
typedef struct {
    u8 pad0[0x20];
    u8 *parts;          /* +0x20 */
} Mdl3;
typedef struct {
    u8 pad0[8];
    AcPos pos;          /* +0x08 */
    u8 pad10[0x60 - 0x10];
    Mdl3 *mdl;          /* +0x60 */
    u8 pad64[0x108 - 0x64];
    SVECTOR tilt;       /* +0x108 (pad +0x10E = engine level) */
} TObj3;
typedef struct {
    u8 pad[1];
    u8 snd;             /* +0x01 */
} Rec20b;
extern Rec20b *D_80040FEC;
extern u8 D_8004124E;
extern u8 *D_801A5DC0;
extern SVECTOR D_8008EC68, D_8008EC70, D_8008EC78, D_8008EC80;
extern void func_8007DF98();
extern void func_8004CC30(s32 id, u8 *buf, AcPos *pos, s32 vol, s32 a, s32 b, s32 c);
extern s16 func_80082654();
extern void func_8001467C(SVECTOR *a, SVECTOR *b, SVECTOR *c, SVECTOR *out);
extern s16 func_80013390(s32 cur, s32 target, s32 step);
extern void func_80013CC0(SVECTOR *ang, void *m);

void func_800826FC(TObj3 *o)
{
    SVECTOR v;
    SVECTOR a;
    SVECTOR b;
    SVECTOR c;
    Mdl3 *m;
    u8 *part;
    s32 idx;
    s32 min;
    s16 t;
    s32 x;
    s32 z;
    s16 u;

    func_8007DF98(o);
    func_8004CC30(D_80040FEC->snd, D_801A5DC0 + 0x1300, &o->pos, (o->tilt.pad >> 7) + 50, 8000, 10000, o->tilt.pad >> 7);
    o->tilt.pad -= 256;
    if (o->tilt.pad < 0) {
        o->tilt.pad = 0;
    }
    m = o->mdl;
    if (m == 0) {
        return;
    }
    part = m->parts + D_8004124E * 124;
    D_8008EC68.vy = func_80082654(o, &D_8008EC68, m);
    D_8008EC70.vy = func_80082654(o, &D_8008EC70);
    D_8008EC78.vy = func_80082654(o, &D_8008EC78);
    D_8008EC80.vy = func_80082654(o, &D_8008EC80);
    z = D_8008EC68.vy;
    if (D_8008EC70.vy < z) {
        idx = 0;
    } else {
        z = D_8008EC70.vy;
        idx = 1;
    }
    if (z < D_8008EC78.vy) {
        z = D_8008EC78.vy;
        idx = 2;
    }
    do {
    if (z < D_8008EC80.vy) {
        idx = 3;
    }
    } while (0);
    if (z == 0x7FFF) {
        v.vz = 0;
        v.vx = 0;
        v.vy = o->tilt.vy;
    } else {
        switch (idx) {
        case 0:
            a = D_8008EC70;
            b = D_8008EC78;
            c = D_8008EC80;
            break;
        case 1:
            a = D_8008EC68;
            b = D_8008EC78;
            c = D_8008EC80;
            break;
        case 2:
            a = D_8008EC68;
            b = D_8008EC80;
            c = D_8008EC70;
            break;
        case 3:
            a = D_8008EC68;
            b = D_8008EC78;
            c = D_8008EC70;
            break;
        }
        if (a.vy < b.vy) {
            min = a.vy;
        } else {
            min = b.vy;
        }
        t = c.vy;
        if (t < min) {
            min = t;
        }
        a.vy -= min;
        if (a.vy > 120) {
            a.vy = 120;
        }
        b.vy -= min;
        if (b.vy > 120) {
            b.vy = 120;
        }
        c.vy = t - min;
        if (c.vy > 120) {
            c.vy = 120;
        }
        func_8001467C(&a, &b, &c, &v);
        v.vy = -v.vy;
        x = abs(v.vx);
        u = x + abs(v.vz);
        if (v.vy < u) {
            v.vy = u;
        }
    }
    o->tilt.vx = func_80013390(o->tilt.vx, v.vx, 32);
    o->tilt.vy = func_80013390(o->tilt.vy, v.vy, 32);
    o->tilt.vz = func_80013390(o->tilt.vz, v.vz, 32);
    func_80013CC0(&o->tilt, part + 20);
    *(s16 *)(part + 38) = 58;
}
