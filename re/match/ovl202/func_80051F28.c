#include "ac1/ac1.h"

typedef struct {
    s16 on;                     /* +0x00 */
    SVECTOR pos;                /* +0x02 */
} Marker;

typedef struct {
    u8 unk0[12];
} HudShape;

extern Marker D_80040FCE;
extern HudShape D_8008EFFC;
extern s32 D_8008DA20;
extern void RotTrans(SVECTOR *v0, VECTOR *v1, long *flag);
extern void func_80060B54(DVECTOR *, HudShape *, s32, s32 *, s32, s32);

void func_80051F28(s32 *arg0) {
    SVECTOR unused[1];
    VECTOR v;
    long flag;
    s32 sx;
    s32 sy;

    if (*arg0 == 0) {
        return;
    }
    if (D_80040FCE.on == 0) {
        return;
    }
    RotTrans(&D_80040FCE.pos, &v, &flag);
    if (v.vz <= 0) {
        return;
    }
    sx = (v.vx * 320) / v.vz;
    sy = (v.vy * 320) / v.vz;
    if ((u32)(sx + 168) < 337 && (u32)(sy + 128) < 257) {
        DVECTOR p;

        p.vx = sx + 152;
        p.vy = sy + 112;
        func_80060B54(&p, &D_8008EFFC, 0, &D_8008DA20, 16, 2);
    }
}
