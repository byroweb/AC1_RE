#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

extern u32 D_800411F4;
extern s32 D_80040FC4;
extern SVECTOR D_80039C78;
extern s16 func_800132CC(s16 cur, s16 goal, s32 rate);

/* Ease the view offset towards the locked target's aim point. */
void func_80082D04(void) {
    SVECTOR a;
    TargetSlot *t = g_target_head;

    if ((D_800411F4 & 0x20000) && D_80040FC4 != 0) {
        if (t == 0 || t->target == 0) {
            goto none;
        }
        a.vx = t->aim[0] - (t->aim[0] >> 2);
        a.vy = t->aim[1] - (t->aim[1] >> 2);
        D_801987D0.ox = func_800132CC(D_801987D0.ox, a.vx, 64);
        D_801987D0.oy = func_800132CC(D_801987D0.oy, a.vy, 64);
    } else {
        if (t != 0 && t->target != 0 && (t->flags & 2)) {
            a.vx = t->aim[0] >> 1;
            a.vy = t->aim[1] >> 1;
        } else {
        none:
            a = D_80039C78;
        }
        D_801987D0.ox = func_800132CC(D_801987D0.ox, a.vx, 20);
        D_801987D0.oy = func_800132CC(D_801987D0.oy, a.vy, 20);
    }
}
