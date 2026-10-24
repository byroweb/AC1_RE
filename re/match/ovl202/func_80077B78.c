/* ovl202: weapon fire dispatcher.  For each of the n shots it takes a muzzle
 * segment (func_800745D0, offsets from the varargs pairs), builds the aim
 * vector and fires by the weapon record's class bits (0x7000) and sub-type
 * (0xF00).  Retail quirks: the record pointer is not advanced, and the
 * effect slot `save` (s5) starts uninitialised. */
#include "ac1/ac1.h"
typedef struct {
    u16 flags;          /* +0x00 */
    u16 h2;             /* +0x02 */
    s16 h4;             /* +0x04 */
    s16 h6;             /* +0x06 */
    s16 h8;             /* +0x08 */
    s16 hA;             /* +0x0A */
    s16 hC;             /* +0x0C */
    s16 hE;             /* +0x0E */
    s16 h10;            /* +0x10 */
    u8 h12[6];          /* +0x12 */
    u8 h18[4];          /* +0x18 */
    u8 h1c[4];          /* +0x1C */
} Func80077B78Wpn;
typedef struct {
    u8 kind;            /* +0x00 */
    u8 pad1;
    s16 dmg;            /* +0x02 */
    u8 b4;              /* +0x04 */
    u8 pad5[3];
} Func80077B78Blast;
typedef struct {
    SVECTOR from;       /* +0x00 */
    SVECTOR to;         /* +0x08 */
} Func80077B78Seg;
typedef struct {
    u8 pad0[0x12];
    s16 yaw;            /* +0x12 */
    u8 pad14[2];
    u16 sect;           /* +0x16 */
    u8 pad18[0x60 - 0x18];
    s32 model;          /* +0x60 */
    u8 pad64[0x76 - 0x64];
    s16 aim_x;          /* +0x76 */
    s16 aim_y;          /* +0x78 */
    s16 aim_y2;         /* +0x7A */
} Func80077B78Ent;
extern u32 D_801A5DD8[];
extern void VectorNormalSS(SVECTOR *in, SVECTOR *out);
extern void func_80014338(SVECTOR *in, s32 a, SVECTOR *out);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern s32 func_800745D0();
extern void func_80085E98(Func80077B78Ent *e, Func80077B78Blast *b, Func80077B78Seg *s, SVECTOR *d, s32 a, s32 c, s32 f, void *g, s32 h, s32 i, s32 j, void *k, void *l, s32 m, s32 n);
extern void func_800860EC(Func80077B78Ent *e, Func80077B78Blast *b, Func80077B78Seg *s, SVECTOR *d, s32 a, s32 c, s32 f, s32 h, s32 i, s32 j, void *k, void *l, s32 m, s32 n);
extern void func_800862E8(Func80077B78Ent *e, Func80077B78Blast *b, Func80077B78Seg *s, SVECTOR *ang, SVECTOR *d, s32 a, s32 c, s32 f, s32 g);
extern void func_80086448(Func80077B78Ent *e, Func80077B78Blast *b, Func80077B78Seg *s, SVECTOR *ang, SVECTOR *d, s32 a, s32 c, s32 f);
extern void func_80087DEC(Func80077B78Ent *e, Func80077B78Seg *s, SVECTOR *d, Func80077B78Blast *b, s32 a, s32 c, s32 f);
extern void func_8008840C(Func80077B78Ent *e, Func80077B78Seg *s, SVECTOR *d, Func80077B78Blast *b, s32 a, s32 c);
extern void func_800889B4(Func80077B78Ent *e, Func80077B78Seg *s, SVECTOR *d, Func80077B78Blast *b, s32 a, s32 c);
extern void func_8008915C(Func80077B78Ent *e, Func80077B78Seg *s, SVECTOR *d, Func80077B78Blast *b, s32 a, s32 c, s32 f, SVECTOR *ang);

void func_80077B78(Func80077B78Ent *e, u8 kind, s16 dmg, u8 b4, Func80077B78Wpn *w, s32 n, ...)
{
    Func80077B78Seg seg;
    s16 pad1[4];
    SVECTOR dir;
    SVECTOR ang;
    Func80077B78Blast b;
    s32 *ap;
    s32 save;
    s32 a;
    s32 c;
    s32 t;

    if (!(D_801A5DD8[(s16)e->sect >> 5] & (1 << (e->sect & 0x1F))))
        return;
    ap = &n;
    b.kind = kind;
    b.dmg = dmg;
    b.b4 = b4;
    while (--n != -1) {
        if (!(w->flags & 0x8000) && e->model != 0) {
            a = *++ap;
            c = *++ap;
            if (!func_800745D0(e, &seg, w->flags & 0xFF, w->h2, 2, a, c))
                continue;
            dir.vx = seg.to.vx - seg.from.vx;
            dir.vy = seg.to.vy - seg.from.vy;
            dir.vz = seg.to.vz - seg.from.vz;
        } else {
            goto alt;
        }
    fire:
        switch (w->flags & 0x7000) {
        case 0:
            if (save != 255)
                save = w->h10;
            switch (w->flags & 0xF00) {
            case 0:
                func_80085E98(e, &b, &seg, &dir, w->h4, w->h6, w->hC, w->h12, w->h8, w->hA, w->hE,
                              w->h18, w->h1c, 0, save);
                break;
            case 0x100:
                func_800860EC(e, &b, &seg, &dir, w->h4, w->h6, w->hC, w->h8, w->hA, w->hE,
                              w->h18, w->h1c, 0, save);
                break;
            case 0x800:
                func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
                ang.vx += *++ap;
                ang.vy += *++ap;
                func_800862E8(e, &b, &seg, &ang, &dir, w->hC, w->hA, w->hE, *++ap);
                break;
            case 0x900:
                func_80087DEC(e, &seg, &dir, &b, w->hA, 52, 0);
                break;
            case 0xA00:
                func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
                func_80014338(&dir, w->h4, &dir);
                func_80086448(e, &b, &seg, &ang, &dir, w->hC, w->hA, w->hE);
                break;
            }
            save = 255;
            break;
        case 0x2000:
            func_800889B4(e, &seg, &dir, &b, w->h6, (w->flags & 0xF00) >> 8);
            break;
        case 0x4000:
            func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
            func_8008915C(e, &seg, &dir, &b, w->h6, (w->flags & 0xF00) >> 8, 0, &ang);
            break;
        case 0x6000:
            func_8008840C(e, &seg, &dir, &b, w->h6, (w->flags & 0xF00) >> 8);
            break;
        case 0x7000:
            t = (s16)(w->h6 / 600);
            VectorNormalSS(&dir, &seg.to);
            seg.to.vx = (seg.to.vx * 75) >> 9;
            seg.to.vy = (seg.to.vy * 75) >> 9;
            seg.to.vz = (seg.to.vz * 75) >> 9;
            func_80014AE4(dir.vx, dir.vy, dir.vz, &ang);
            func_80086448(e, &b, &seg, &ang, &seg.to, 49, 0, t);
            break;
        }
        continue;
    alt:
        a = *++ap;
        ++ap;
        if (!func_800745D0(e, &seg, (u8)w->flags, w->h2, 1, a))
            continue;
        ang.vx = -e->aim_x;
        if (e->aim_y2 != 0)
            ang.vy = e->yaw - e->aim_y2;
        else
            ang.vy = e->yaw - e->aim_y;
        func_80014774(&ang, &dir);
        goto fire;
    }
}
