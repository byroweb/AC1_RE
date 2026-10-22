#include "ac1/ac1.h"

typedef struct {
    u8 pad0;
    u8 kind;            /* +0x01 */
    u8 pad2[0x0E];
    u16 count;          /* +0x10 rounds left */
    u8 pad12[0x12];
    u8 x24[4];          /* +0x24 */
} Ammo;

typedef struct {
    u8 b0;              /* +0x00 */
    u8 b1;              /* +0x01 */
    u8 pad2[2];
    u16 spread_x;       /* +0x04 */
    u16 spread_y;       /* +0x06 */
} WSpread;

typedef struct {
    void *target;
    u8 pad04[0x12];
    u16 flags;          /* +0x16 */
    u16 aim[2];         /* +0x18 */
    u8 pad1c[0x0A];
    s16 h26;            /* +0x26 */
    u8 pad28[8];
} TSlot;

extern TSlot D_80041024[8];
extern TSlot *D_80041020;
extern s32 D_80040FC4;
extern s32 func_800745D0(AcEntity *e, SVECTOR *v, s32 a, s32 b, s32 c, s32 d, s32 f);
extern void func_80014AE4(s32 x, s32 y, s32 z, SVECTOR *out);
extern s32 func_80014D78(s32 a);
extern void func_80014774(SVECTOR *rot, SVECTOR *dir);
extern void func_80014338(SVECTOR *v, s32 r, SVECTOR *out);
extern s32 func_800862E8(AcEntity *e, void *a, SVECTOR *v, SVECTOR *rot, SVECTOR *vel, s32 sp, s32 b0, s32 b1, void *tgt);

TSlot *func_8007CD74(AcEntity *e, Ammo *am, WSpread *w, s32 n, s32 a, s32 b)
{
    SVECTOR v[2];
    SVECTOR rel;
    SVECTOR ang;
    SVECTOR rot;
    SVECTOR dir;
    SVECTOR vel;
    TSlot *t;
    s32 i;
    s32 first;
    s32 r;

    first = 1;
    if (n == 0) {
        return 0;
    }
    i = 8;
    t = D_80041024;
    for (; i != 0; t++, i--) {
        if (t->target != 0 && (t->flags & 8) && t->h26 != 0) {
            goto found;
        }
    }
    return 0;
found:
    while (--n != -1) {
        if (am->count == 0) {
            continue;
        }
        am->count--;
        if (func_800745D0(e, v, a, b, 2, n * 2, n * 2 + 1) != 0) {
            func_80014AE4(v[1].vx - v[0].vx, v[1].vy - v[0].vy, v[1].vz - v[0].vz, &ang);
            rel.vx = e->pos.x - e->prev_pos.x;
            rel.vy = (e->pos.y - e->prev_pos.y) >> 2;
            rel.vz = e->pos.z - e->prev_pos.z;
            r = func_80014D78(w->spread_x);
            rot.vx = (w->spread_x >> 1) - (ang.vx + ((s16)t->aim[0] >> 2) + r);
            r = func_80014D78(w->spread_y);
            rot.vy = ang.vy + ((s16)t->aim[1] >> 2) + r - (w->spread_y >> 1);
            rot.vz = 0;
            if (am->kind == 6) {
                rot.vx += 192;
            }
            func_80014774(&rot, &dir);
            func_80014338(&dir, 100, &vel);
            rot.pad = first;
            vel.vx += rel.vx;
            vel.vy += rel.vy;
            vel.vz += rel.vz;
            rot.vx = -rot.vx;
            r = func_800862E8(e, am->x24, v, &rot, &vel, 48, w->b0, w->b1, t->target);
            first = 0;
            if (D_80041020 == t) {
                D_80040FC4 = r;
            }
        } else {
            return 0;
        }
    }
    return t;
}
