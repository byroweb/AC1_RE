#include "ac1/ac1.h"
/* ovl202 func_8005156C - debug marker drawer.  Sets the GTE transform from
 * the camera matrix D_801AD67C and the view position/angle, then (flag 1)
 * draws a box whose half-sizes follow rsin of two angles in o, and (flag 2,
 * when t is given) a short line repeated three times along a direction:
 * t's angles through func_80014774, or straight ahead when t->active is 0. */

typedef struct {
    u8 unk00[12];
    u8 flags;                   /* +0x0C bit 0: box, bit 1: direction marker */
    u8 unk0D[15];
    s16 ang_b;                  /* +0x1C */
    s16 ang_a;                  /* +0x1E */
} Func8005156CObj;

typedef struct {
    s32 active;                 /* +0x00 */
    u8 unk04[20];
    s16 yaw;                    /* +0x18 */
    s16 pitch;                  /* +0x1A */
} Func8005156CTarget;

extern MATRIX D_801AD67C;
extern SVECTOR D_801A26C0;
extern s16 D_801A26CA;
typedef struct {
    s16 pitch;
} Func8005156CView;
extern Func8005156CView D_800411C0;
extern u32 D_80198804;
extern u8 D_8008DA44[];
extern u8 D_8008DA4C[];
extern void func_8005154C(SVECTOR *v, DVECTOR *sxy);
extern void func_800610A4(DVECTOR *pts, void *col, s32 n, s32 arg3, s32 arg4);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);

void func_8005156C(Func8005156CObj *o, Func8005156CTarget *t)
{
    SVECTOR rot;
    SVECTOR rel;
    SVECTOR box[8];
    DVECTOR sxy[9];
    MATRIX m;
    SVECTOR dir;
    SVECTOR ang;
    s32 a, b;
    s32 i;

    if (o->flags == 0) {
        return;
    }
    rel.vx = D_801A26C0.vx - D_801AD67C.t[0];
    rel.vy = D_801A26C0.vy - D_801AD67C.t[1];
    rel.vz = D_801A26C0.vz - D_801AD67C.t[2];
    ApplyMatrix(&D_801AD67C, &rel, (VECTOR *)m.t);
    rot.vy = -D_801A26CA;
    rot.vx = -D_800411C0.pitch;
    rot.vz = 0;
    RotMatrixYXZ(&rot, &m);
    MulMatrix2(&D_801AD67C, &m);
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
        func_8005154C(&box[0], &sxy[0]);
        func_8005154C(&box[1], &sxy[1]);
        func_8005154C(&box[2], &sxy[2]);
        func_8005154C(&box[3], &sxy[3]);
        func_8005154C(&box[4], &sxy[4]);
        func_8005154C(&box[5], &sxy[5]);
        func_8005154C(&box[6], &sxy[6]);
        func_8005154C(&box[7], &sxy[7]);
        sxy[8] = sxy[0];
        func_800610A4(sxy, D_8008DA44, 8, 16, -1);
    }

    if ((o->flags & 2) && t != 0) {
        i = (D_80198804 & 3) + 1;
        if (t->active != 0) {
            rot.vx = 0;
            rot.vy = -D_801A26CA;
            RotMatrixYXZ(&rot, &m);
            MulMatrix2(&D_801AD67C, &m);
            SetRotMatrix(&m);
            ang.vy = t->pitch;
            ang.vx = -t->yaw - D_800411C0.pitch;
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
            func_8005154C(&box[0], &sxy[0]);
            func_8005154C(&box[1], &sxy[1]);
            func_800610A4(sxy, D_8008DA4C, 1, 16, 1);
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
