#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"

/* per-player record (0x39C, array at 0x80040FA0); D_80039D18 = current */
typedef struct {
    u8 unk0[0x24];
    s32 unk24;                  /* +0x24 (0x80040FC4 for player 0) */
    u8 unk28[0x58];
    TargetSlot *target;         /* +0x80 lock list head */
    u8 unk84[0x254 - 0x84];
    u32 flags;                  /* +0x254 (0x800411F4) */
    u8 unk258[0x398 - 0x258];
    CamParm *cam;               /* +0x398 */
} Player;

extern Player *D_80039D18;
extern SVECTOR D_80039C78;
extern s16 func_800132CC(s16 cur, s16 goal, s32 rate);

/* ease the view offset towards the locked target's aim point (twin of ovl202 0x80082D04) */
void func_8008166C(void) {
    SVECTOR a;
    TargetSlot *t = D_80039D18->target;

    if ((D_80039D18->flags & 0x20000) && D_80039D18->unk24 != 0) {
        if (t == 0 || t->target == 0) {
            goto none;
        }
        a.vx = t->aim[0] - (t->aim[0] >> 2);
        a.vy = t->aim[1] - (t->aim[1] >> 2);
        D_80039D18->cam->ox = func_800132CC(D_80039D18->cam->ox, a.vx, 64);
        D_80039D18->cam->oy = func_800132CC(D_80039D18->cam->oy, a.vy, 64);
    } else {
        if (t != 0 && t->target != 0 && (t->flags & 2)) {
            a.vx = t->aim[0] >> 1;
            a.vy = t->aim[1] >> 1;
        } else {
        none:
            a = D_80039C78;
        }
        D_80039D18->cam->ox = func_800132CC(D_80039D18->cam->ox, a.vx, 20);
        D_80039D18->cam->oy = func_800132CC(D_80039D18->cam->oy, a.vy, 20);
    }
}
