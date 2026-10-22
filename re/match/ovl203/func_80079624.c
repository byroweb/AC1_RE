#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

extern s32 func_800732FC(void *, SVECTOR *, s32, s32, s32, s32, s32);
extern void func_80078E24(void *, SVECTOR *, s32);

/* Like func_80078FE4, but the pair is first set to the object's mid height and
 * its second point pushed out to twice the distance. */
typedef struct {
    u8 unk00[0x0A];
    s16 top;                    /* +0x0A */
    u8 unk0c[0x2A - 0x0C];
    u16 height;                 /* +0x2A */
} Ovl203Body;

void func_80079624(Ovl203Body *o, s32 arg) {
    SVECTOR cur[2];
    SVECTOR mid[2];
    u16 y;

    if (func_800732FC(o, cur, D_80039D18->aim_arg[0], D_80039D18->aim_arg[1], 2, 0, 1) != 0) {
        y = o->top - (o->height >> 1);
        ((u16 *)cur)[5] = y;
        ((u16 *)cur)[1] = y;
        ((u16 *)cur)[4] = ((u16 *)cur)[0] + (((u16 *)cur)[4] - ((u16 *)cur)[0]) * 2;
        ((u16 *)cur)[6] = ((u16 *)cur)[2] + (((u16 *)cur)[6] - ((u16 *)cur)[2]) * 2;
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
