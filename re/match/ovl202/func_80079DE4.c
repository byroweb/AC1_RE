#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xCC];
    SVECTOR v;          /* +0xCC */
} AcTurn;

typedef struct {
    u8 pad[0x11];
    u8 id;
} SndCtx;

extern u16 D_800411B6[];
extern SndCtx *D_80040FEC[];
extern u8 *D_801A5DC0[];
extern s32 func_8004CC30(s32 id, void *key, AcPos *pos, s32 vol, s32 range, s32 len, s32 pitch);

void func_80079DE4(AcEntity *e)
{
    if (--((u8 *)&e->mover)[2] == 0 && e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
    ((AcTurn *)e)->v.vx += ((AcTurn *)e)->v.vy;
    if (((AcTurn *)e)->v.vx < -D_800411B6[0]) {
        ((AcTurn *)e)->v.vx = -D_800411B6[0];
    }
    e->yaw += ((AcTurn *)e)->v.vx;
    func_8004CC30(D_80040FEC[0]->id, D_801A5DC0[0] + 0x1200, &e->pos, 40, 8000, 10000, 0);
}
