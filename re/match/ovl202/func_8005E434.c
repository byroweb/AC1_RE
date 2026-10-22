#include "ac1/ac1.h"

typedef struct {
    SVECTOR pos;                /* +0x00 */
    u8 unk8[2];
    u16 heading;                /* +0x0A */
    u8 unkc[2];
    u16 id;                     /* +0x0E visibility bit */
    u8 unk10[0x16];
    u16 range;                  /* +0x26 */
    u16 cone;                   /* +0x28 0 = omnidirectional */
} Lamp;

typedef struct {
    u8 r, g, b, cd;
} Rgba;

typedef struct {
    Rgba c[3];
} FlareCol;

/* render globals from the visibility bits at 0x801AC860 */
typedef struct {
    u32 vis[16];                /* +0x000 */
    u8 unk40[0xE1C - 0x40];
    MATRIX cam;                 /* +0xE1C (0x801AD67C) */
} VisCam;

extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void RotTrans(SVECTOR *v0, VECTOR *v1, s32 *flag);
extern s32 ratan2(s32 y, s32 x);
extern s32 func_800148E4(s32 a, s32 b, s32 range);
extern void func_80060B54(s16 *xy, FlareCol *col, s32 a, Rgba *c2, s32 size, s32 mode);
extern FlareCol D_8004B3F8;
extern VisCam D_801AC860;

/* lens flare for a visible lamp in range (and facing the camera when it
 * has a cone) */
void func_8005E434(Lamp *l)
{
    VECTOR v;
    s16 xy[2];
    s32 flag;
    s32 sx;
    s32 sy;

    if (!(D_801AC860.vis[(s16)l->id >> 5] & (1 << (l->id & 0x1F)))) {
        return;
    }
    SetRotMatrix(&D_801AC860.cam);
    SetTransMatrix((MATRIX *)((u8 *)&D_801AC860.cam + 12));
    RotTrans(&l->pos, &v, &flag);
    if (v.vz <= 0 || v.vz >= l->range) {
        return;
    }
    {
        FlareCol col;

        col = D_8004B3F8;
        if (l->cone != 0
            && func_800148E4((s16)(2048 - l->heading),
                             (s16)ratan2(l->pos.vx - D_801AC860.cam.t[0], l->pos.vz - D_801AC860.cam.t[2]),
                             (s16)l->cone) == 0) {
            return;
        }
        sx = v.vx * 320 / v.vz;
        sy = v.vy * 320 / v.vz;
        xy[0] = sx;
        xy[1] = sy;
        if ((u16)(sx + 160) > 320 || (u16)(sy + 120) > 240) {
            return;
        }
        xy[0] = sx + 160;
        xy[1] = sy + 120;
        func_80060B54(xy, &col, 0, &col.c[2], 16, 2);
    }
}
