#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

extern s32 func_800732FC(void *, SVECTOR *, s32, s32, s32, s32, s32);
extern void func_80078E24(void *, SVECTOR *, s32);

/* Aim: fetch the new aim pair, apply the midpoint with the last one and then
 * the new one, and remember it in the current player block. */
void func_80078FE4(void *o, s32 arg) {
    SVECTOR cur[2];
    SVECTOR mid[2];

    if (func_800732FC(o, cur, D_80039D18->aim_arg[0], D_80039D18->aim_arg[1], 2, 0, 1) != 0) {
        mid[0].vx = (D_80039D18->aim[0].vx + cur[0].vx) >> 1;
        mid[0].vy = (D_80039D18->aim[0].vy + cur[0].vy) >> 1;
        mid[0].vz = (D_80039D18->aim[0].vz + cur[0].vz) >> 1;
        mid[1].vx = (D_80039D18->aim[1].vx + cur[1].vx) >> 1;
        mid[1].vy = (D_80039D18->aim[1].vy + cur[1].vy) >> 1;
        mid[1].vz = (D_80039D18->aim[1].vz + cur[1].vz) >> 1;
        func_80078E24(o, mid, arg);
        func_80078E24(o, cur, arg);
        D_80039D18->aim[0] = cur[0];
        D_80039D18->aim[1] = cur[1];
    }
}
