#include "ac1/ac1.h"
/* ovl204 func_80051878 - the ovl204 copy of ovl202 func_80051878 (debug
 * marker drawer); here the view position and angles come from the record
 * that D_80039D18 points to instead of fixed globals.  Sets the GTE transform
 * from the camera matrix D_801B4F9C and that view, then (flag 1)
 * draws a box whose half-sizes follow rsin of two angles in o, and (flag 2,
 * when t is given) a short line repeated three times along a direction:
 * t's angles through func_80014774, or straight ahead when t->active is 0. */

typedef struct {
    u8 unk00[12];
    u8 flags;                   /* +0x0C bit 0: box, bit 1: direction marker */
    u8 unk0D[15];
    s16 ang_b;                  /* +0x1C */
    s16 ang_a;                  /* +0x1E */
} Func80051878Obj;

typedef struct {
    s32 active;                 /* +0x00 */
    u8 unk04[20];
    s16 yaw;                    /* +0x18 */
    s16 pitch;                  /* +0x1A */
} Func80051878Target;

extern MATRIX D_801B4F9C;
typedef struct {
    u8 unk000[8];
    SVECTOR pos;                /* +0x008 view position */
    s16 unk010;
    s16 yaw;                    /* +0x012 */
} Func80051878View;

typedef struct {
    u8 unk000[0x220];
    s16 pitch;                  /* +0x220 */
    u8 unk222[0x394 - 0x222];
    Func80051878View *view;     /* +0x394 */
    struct {
        u8 unk00[0x34];
        u32 frame;              /* +0x34 */
    } *clock;                   /* +0x398 */
} Func80051878Game;

extern Func80051878Game *D_80039D18;
extern u8 D_800894E8[];
extern u8 D_800894F0[];
extern void func_80051858(SVECTOR *v, DVECTOR *sxy);
extern void func_80060050(DVECTOR *pts, void *col, s32 n, s32 arg3, s32 arg4);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);

void func_80051878(Func80051878Obj *o, Func80051878Target *t)
{
    Func80051878View *view;
    SVECTOR rot;
    SVECTOR rel;
    SVECTOR box[8];
    DVECTOR sxy[9];
    MATRIX m;
    SVECTOR dir;
    SVECTOR ang;
    s32 a, b;
    s32 i;

    view = D_80039D18->view;
    if (o->flags == 0) {
        return;
    }
    rel.vx = view->pos.vx - D_801B4F9C.t[0];
    rel.vy = view->pos.vy - D_801B4F9C.t[1];
    rel.vz = view->pos.vz - D_801B4F9C.t[2];
    ApplyMatrix(&D_801B4F9C, &rel, (VECTOR *)m.t);
    rot.vy = -view->yaw;
    rot.vx = -D_80039D18->pitch;
    rot.vz = 0;
    RotMatrixYXZ(&rot, &m);
    MulMatrix2(&D_801B4F9C, &m);
    SetRotMatrix(&m);
    SetTransMatrix(&m);

    if (o->flags & 1) {
        a = rsin(o->ang_a) << 2;
        b = rsin(o->ang_b) << 2;
        box[0].vx = box[5].vx = -a + 512;
        box[1].vx = box[4].vx = a - 512;
        box[2].vx = box[3].vx = a;
        box[6].vx = box[7].vx = -a;
        box[0].vy = box[1].vy = -b;
        box[2].vy = box[7].vy = -b + 512;
        box[3].vy = box[6].vy = b - 512;
        box[4].vy = box[5].vy = b;
        box[0].vz = box[1].vz = box[2].vz = box[3].vz = box[4].vz = box[5].vz = box[6].vz = box[7].vz = 16384;
        func_80051858(&box[0], &sxy[0]);
        func_80051858(&box[1], &sxy[1]);
        func_80051858(&box[2], &sxy[2]);
        func_80051858(&box[3], &sxy[3]);
        func_80051858(&box[4], &sxy[4]);
        func_80051858(&box[5], &sxy[5]);
        func_80051858(&box[6], &sxy[6]);
        func_80051858(&box[7], &sxy[7]);
        sxy[8] = sxy[0];
        func_80060050(sxy, D_800894E8, 8, 16, -1);
    }

    if ((o->flags & 2) && t != 0) {
        i = (D_80039D18->clock->frame & 3) + 1;
        if (t->active != 0) {
            rot.vy = -view->yaw;
            rot.vx = 0;
            RotMatrixYXZ(&rot, &m);
            MulMatrix2(&D_801B4F9C, &m);
            SetRotMatrix(&m);
            ang.vy = t->pitch;
            ang.vx = -t->yaw - D_80039D18->pitch;
            func_80014774(&ang, &dir);
        } else {
            dir.vy = 0;
            dir.vx = 0;
            dir.vz = 4096;
        }
        box[0].vx = box[1].vx = dir.vx * i >> 2;
        box[0].vy = box[1].vy = dir.vy * i >> 2;
        box[0].vz = box[1].vz = (dir.vz * i >> 2) + 1024;
        dir.vx >>= 1;
        dir.vy >>= 1;
        dir.vz >>= 1;
        box[0].vx -= 150;
        box[1].vx += 150;
        for (i = 0; i < 3; i++) {
            func_80051858(&box[0], &sxy[0]);
            func_80051858(&box[1], &sxy[1]);
            func_80060050(sxy, D_800894F0, 1, 16, 1);
            box[0].vx += dir.vx;
            box[0].vy += dir.vy;
            box[0].vz += dir.vz;
            box[1].vx += dir.vx;
            box[1].vy += dir.vy;
            box[1].vz += dir.vz;
        }
        while (--i);
    }
}
