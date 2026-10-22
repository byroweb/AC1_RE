#include "ac1/ac1.h"
/* ovl202 0x8005DEC4 - lit variant of func_8005DC7C: place model `model` at pos/rot
 * relative to the camera with light set D_801D0B98[ls].idx (colour/back colour,
 * light matrix, fog), then draw it (func_80058B04, or func_800659EC when
 * func_80067AB4 finds something for it). */

typedef struct {
    s32 n;                      /* +0x00 */
    s32 flags;                  /* +0x04 bit1: extra prims, bit0: twice */
    u8 unk8[4];
    void *verts;                /* +0x0C */
} ModelBody;

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the model body */
} ModelHdr;

typedef struct {                /* 0x68 */
    MATRIX light;               /* +0x00 */
    u8 unk20[0x30];
    MATRIX color;               /* +0x50 */
} LightSetHead;

typedef struct {                /* 0x68 */
    MATRIX light;               /* +0x00 */
    u8 unk20[0x30];
    s16 color[3][3];            /* +0x50 */
    u8 unk62pad[0];
    u8 r;                       /* +0x62 */
    u8 g;                       /* +0x63 */
    u8 b;                       /* +0x64 */
    u8 unk65;
    u16 fog;                    /* +0x66 */
} LightSet;

typedef struct {
    u8 idx;                     /* +0x00 light set */
    u8 unk1[0x3F];
} LightSel;                     /* 0x40, at 0x801D0B98 */

/* render globals from 0x801A5F30 */
typedef struct {
    ModelBody *model;           /* +0x0000 (0x801A5F30) */
    ModelHdr *tbl[1];           /* +0x0004 (0x801A5F34) */
    u8 unk8[0x6A2C];
    LightSet lights[1];         /* +0x6A34 (0x801AC964) */
    u8 unk6a9c[0x7734 - 0x6A9C];
    s32 fog;                    /* +0x7734 (0x801AD664) */
    u8 unk7738[0x14];
    MATRIX cam;                 /* +0x774C (0x801AD67C) */
    u8 unk776c[0x10];
    MATRIX cam2;                /* +0x777C (0x801AD6AC) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern MATRIX *MulMatrix0(MATRIX *m0, MATRIX *m1, MATRIX *m2);
extern void SetColorMatrix(void *m);
extern void SetLightMatrix(MATRIX *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern void func_8005D0F0(u16 *p);
extern s32 func_80067AB4(SVECTOR *pos, s32 a, s32 b);
extern void func_80058B04(s32 a, ModelHdr *h, s32 z, s32 b, s32 c, s32 n);
extern void func_800659EC(s32 a, ModelHdr *h, s32 z, s32 v, s32 b, s32 c, s32 n);
extern MATRIX D_800B1228;
extern s32 D_801987CC;
extern RenderGlobals D_801A5F30;
extern LightSel D_801D0B98[];

void func_8005DEC4(u16 model, SVECTOR *pos, SVECTOR *rot, s32 a3, u16 *a4, s32 a5, s32 sel,
                   s32 up, s32 down, s32 a9, s32 a10)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[16];
    MATRIX lm;
    s32 z;
    s32 n;
    s32 v;
    u32 f;
    ModelHdr *h;
    LightSet *ls;

    z = up;
    d.vx = pos->vx - D_801A5F30.cam.t[0];
    d.vy = pos->vy - D_801A5F30.cam.t[1];
    d.vz = pos->vz - D_801A5F30.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    ls = &D_801A5F30.lights[D_801D0B98[sel].idx];
    h = D_801A5F30.tbl[model];
    D_801A5F30.model = (ModelBody *)((u8 *)h + h->off);
    SetColorMatrix(ls->color);
    SetBackColor(ls->r, ls->g, ls->b);
    ApplyMatrix(&D_801A5F30.cam, &d, (VECTOR *)D_800B1228.t);
    if (*a4 != 0) {
        func_8005D0F0(a4);
    }
    r.vx = rot->vx;
    r.vz = rot->vz;
    r.vy = 2048 - rot->vy;
    RotMatrixYXZ(&r, &D_800B1228);
    MulMatrix0(&ls->light, &D_800B1228, &lm);
    SetLightMatrix(&lm);
    if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
        f = ls->fog;
        if (f != 0) {
            f *= 2;
            SetFogNearFar(f >> 4, f >> 2, 160);
        }
        D_801A5F30.fog = ls->fog;
        MulMatrix2(&D_801A5F30.cam2, &D_800B1228);
        D_801987CC = 0;
        if (D_801A5F30.model->flags & 2) {
            if (D_801A5F30.model->flags & 1) {
                n = D_801A5F30.model->n * 2;
            } else {
                n = D_801A5F30.model->n;
            }
        } else {
            n = 0;
        }
    } else {
        f = ls->fog;
        if (f != 0) {
            f *= 2;
            SetFogNearFar(f >> 2, f, 160);
        }
        D_801A5F30.fog = ls->fog;
        MulMatrix2(&D_801A5F30.cam, &D_800B1228);
        n = 0;
        D_801987CC = 2;
        D_800B1228.t[0] *= 4;
        D_800B1228.t[1] *= 4;
        D_800B1228.t[2] *= 4;
    }
    v = func_80067AB4(pos, a9, a10);
    if (v == 0) {
        func_80058B04(a3, h, z, *a4, a5, n);
    } else {
        func_800659EC(a3, h, z, v, *a4, a5, n);
    }
}
