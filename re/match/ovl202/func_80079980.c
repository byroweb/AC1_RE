#include "ac1/ac1.h"

typedef struct {
    u8 b0;
    u8 pad[3];
    u8 b4;
} SndCtx;

extern u16 D_800411B4;
extern u16 D_800411B6;
extern SndCtx *D_80040FEC;
extern u8 D_80041250;
extern u8 D_80041251;
extern u8 D_80041252;
extern u8 D_80041253;
extern void func_80079798(AcEntity *e, s32 acc, s32 max);
extern void func_8007847C(AcEntity *e, s32 a, s32 b, s32 c, s32 d, s32 f, s32 g, s32 h);
extern void func_80078250(AcEntity *e, s32 a, s32 b, s32 c, s32 p, s32 q, s32 r, s32 s);
extern s32 func_8004F024(AcMover *m, AcPos *pos);

void func_80079980(AcEntity *e)
{
    s16 *h = (s16 *)e;
    s32 top;
    s32 y;
    SndCtx *c;

    func_80079798(e, D_800411B4, D_800411B6);
    top = h[0x2E / 2];
    y = e->pos.y;
    if (y >= top && top >= y - *(u16 *)((u8 *)e + 0x2A)) {
        if (((u8 *)&e->mover)[0] == 22) {
            if (h[0x6E / 2] == 6) {
                func_8007847C(e, 100, 5, 300, 700, 100, 300, 64);
            }
        } else {
            if (h[0x6E / 2] == D_80040FEC->b4 + 1) {
                func_80078250(e, D_80041250, D_80041251, 0, 80, 200, 65, 0);
            } else if (h[0x6E / 2] == D_80040FEC->b0 + 1) {
                func_80078250(e, D_80041252, D_80041253, 0, 80, 200, 65, 0);
            }
        }
    }
    if (func_8004F024(&e->mover, &e->pos) == 0 && e->fn_state != 0) {
        e->fn_state(e, e->state);
    }
}
