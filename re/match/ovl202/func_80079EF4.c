#include "ac1/ac1.h"

#define ABS(x) ((x) >= 0 ? (x) : -(x))

typedef struct {
    u8 pad[0xCC];
    SVECTOR v;          /* +0xCC */
} AcPadMove;

typedef struct {
    u8 pad[0x11];
    u8 id;
} SndCtx;

extern u16 D_800411B8;
extern u16 D_800411BA;
extern s32 D_80040FA8;
extern u16 D_80041234;
extern u16 D_80041236;
extern SndCtx *D_80040FEC;
extern u8 *D_801A5DC0;
extern void func_80079798(AcEntity *e, s32 acc, s32 max);
extern s32 func_8004CC30(s32 id, void *key, AcPos *pos, s32 vol, s32 range, s32 len, s32 pitch);
extern s32 func_8004F080(AcMover *m, AcPos *pos);

void func_80079EF4(AcEntity *e)
{
    func_80079798(e, D_800411B8, D_800411BA);
    if (D_80040FA8 & (D_80041234 | D_80041236)) {
        func_8004CC30(D_80040FEC->id, D_801A5DC0 + 0x1200, &e->pos, 40, 8000, 10000, 0);
    }
    ((AcPadMove *)e)->v.vy = 0;
    if (ABS(((AcPadMove *)e)->v.vx) <= 512) {
        ((AcPadMove *)e)->v.vx = 0;
    } else {
        ((AcPadMove *)e)->v.vx -= ((AcPadMove *)e)->v.vx >> 3;
    }
    if (ABS(((AcPadMove *)e)->v.vz) <= 512) {
        ((AcPadMove *)e)->v.vz = 0;
    } else {
        ((AcPadMove *)e)->v.vz -= ((AcPadMove *)e)->v.vz >> 3;
    }
    if (func_8004F080(&e->mover, &e->pos) == 0 && e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
}
