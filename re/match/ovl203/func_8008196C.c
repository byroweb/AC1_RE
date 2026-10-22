#include "ac1/ac1.h"

/* ovl203 port of ovl202 0x80082D04: the player record is reached through D_80039D18 */
typedef struct {
    u8 pad00[0x24];
    s32 lock;                   /* +0x24 (ovl202 D_80040FC4) */
    u8 pad28[0x80 - 0x28];
    TargetSlot *target_head;    /* +0x80 (ovl202 g_target_head) */
    u8 pad84[0x254 - 0x84];
    u32 flags;                  /* +0x254 (ovl202 D_800411F4) */
} Pl3;

typedef struct {                /* 0x80194738: ovl203 view camera (CamParm layout) */
    u8 pad00[0x2C];
    s16 ox;                     /* +0x2C */
    s16 oy;                     /* +0x2E */
} Cam3;

extern Pl3 *D_80039D18;
extern Cam3 D_80194738;
extern SVECTOR D_80039C78;
extern s16 func_800132CC(s16 cur, s16 goal, s32 rate);

/* Ease the view offset towards the locked target's aim point. */
void func_8008196C(void) {
    SVECTOR a;
    Pl3 *p = D_80039D18;
    TargetSlot *t = p->target_head;

    if ((p->flags & 0x20000) && p->lock != 0) {
        if (t == 0 || t->target == 0) {
            goto none;
        }
        a.vx = t->aim[0] - (t->aim[0] >> 2);
        a.vy = t->aim[1] - (t->aim[1] >> 2);
        D_80194738.ox = func_800132CC(D_80194738.ox, a.vx, 64);
        D_80194738.oy = func_800132CC(D_80194738.oy, a.vy, 64);
    } else {
        if (t != 0 && t->target != 0 && (t->flags & 2)) {
            a.vx = t->aim[0] >> 1;
            a.vy = t->aim[1] >> 1;
        } else {
        none:
            a = D_80039C78;
        }
        D_80194738.ox = func_800132CC(D_80194738.ox, a.vx, 20);
        D_80194738.oy = func_800132CC(D_80194738.oy, a.vy, 20);
    }
}
