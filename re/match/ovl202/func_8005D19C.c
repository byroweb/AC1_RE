#include "ac1/ac1.h"
/* matched (wave4 a3): far-arm copy loop uses its own s1/d1/k (last use inside the arm, so loop.c adds no biv final values), ret = 1 after MulMatrix2 */
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
    u8 unk0[5];
    u8 parts;                   /* +0x05 */
    u8 unk6[2];
    s32 off;                    /* +0x08 byte offset of the model body */
} ModelHdr;

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
    u8 unk779c[0x100];
    MATRIX cam3;                /* +0x789C (0x801AD7CC) */
    MATRIX cam4;                /* +0x78BC (0x801AD7EC) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern MATRIX *MulMatrix0(MATRIX *m0, MATRIX *m1, MATRIX *m2);
extern void SetColorMatrix(void *m);
extern void SetLightMatrix(MATRIX *m);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern void func_8005D0F0(u16 *p);
extern s32 func_80067AB4(SVECTOR *pos, s32 a, s32 b);
extern s32 func_80054750(s32 *out, u16 model, s32 flags, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_80058B04(s32 a, ModelHdr *h, s32 z, s32 b, s32 c, s32 n);
extern void func_800659EC(s32 a, ModelHdr *h, s32 z, s32 v, s32 b, s32 c, s32 n);
extern void func_8005804C(u16 part, s32 z, s32 lights);
extern void func_80059198(u16 part, s32 z, u16 b, s32 lights);
extern void func_80064C3C(u16 part, s32 z, s32 v, u16 b, s32 lights);
extern MATRIX D_800B1228;
extern s32 D_801987CC;
extern RenderGlobals D_801A5F30;
extern void *D_801A6544;
extern LightSel D_801D0B98[];

/* ovl202 0x8005D19C - multi-part sibling of func_8005DEC4: place model at pos/rot with
 * light set D_801D0B98[sel]; flags bit 7 = keep the pre-built camera matrices
 * (cam3 far / cam4 halved near); then draw the parts (func_80054750 + func_80058B04 /
 * func_800659EC) or the single part `flags` (func_8005804C / func_80059198 /
 * func_80064C3C).  Returns the OT shift class (1 far, 2 near). */
s32 func_8005D19C(u16 model, SVECTOR *pos, SVECTOR *rot, s32 *out, s32 flags, s32 a5, s32 a6,
                  s32 a7, s32 a8, s32 a9, u16 *a10, s32 lights, s32 sel, s32 up, s32 down,
                  s32 a15, s32 a16)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[48];
    MATRIX lm;
    s32 z;
    s32 v;
    s16 ret;
    u32 f;
    ModelHdr *h;
    LightSet *ls;
    u32 *src;
    u32 *dst;
    s16 *ms;
    s32 i;
    s32 i_b;

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
    if (flags & 0x80) {
        flags &= 0x7F;
        SetLightMatrix(&ls->light);
        if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
            u32 *s1;
            u32 *d1;
            s32 k;
            s1 = (u32 *)&D_801A5F30.cam3;
            d1 = (u32 *)&D_800B1228;
            k = 5;
            do {
                *d1++ = *s1++;
            } while (--k);
            ret = 1;
            D_801987CC = 1;
            D_800B1228.t[0] *= 2;
            D_800B1228.t[1] *= 2;
            D_800B1228.t[2] *= 2;
        } else {
            src = (u32 *)&D_801A5F30.cam4;
            dst = (u32 *)&D_800B1228;
            i = 5;
            do {
                *dst++ = *src++;
            } while (--i);
            ms = D_800B1228.m[0];
            i_b = 9;
            do {
                *ms = *ms >> 1;
                ms++;
            } while (--i_b);
            ret = 2;
            D_801987CC = 2;
            D_800B1228.t[0] *= 4;
            D_800B1228.t[1] *= 4;
            D_800B1228.t[2] *= 4;
        }
    } else {
        if (*a10 != 0) {
            func_8005D0F0(a10);
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
            ret = 1;
            D_801987CC = 0;
        } else {
            f = ls->fog;
            if (f != 0) {
                f *= 2;
                SetFogNearFar(f >> 2, f, 160);
            }
            D_801A5F30.fog = ls->fog;
            MulMatrix2(&D_801A5F30.cam, &D_800B1228);
            ret = 1;
            D_801987CC = 2;
            D_800B1228.t[0] *= 4;
            D_800B1228.t[1] *= 4;
            D_800B1228.t[2] *= 4;
        }
    }
    if (h->parts != 0) {
        v = func_80067AB4(pos, a15, a16);
        if (func_80054750(out, model, flags, a5, a6, a7, a8, a9) != 0) {
            if (v == 0) {
                func_80058B04(*out, h, z, *a10, lights, 0);
            } else {
                func_800659EC(*out, h, z, v, *a10, lights, 0);
            }
        }
    } else {
        v = func_80067AB4(pos, a15, a16);
        SetRotMatrix(&D_800B1228);
        SetTransMatrix(&D_800B1228);
        D_801A6544 = *(void **)((u8 *)D_801A5F30.model + (flags * 32 + 12) - flags * 4);
        if (v == 0) {
            if (*a10 == 0) {
                func_8005804C(flags, z, lights & (1 << flags));
            } else {
                func_80059198(flags, z, *a10, lights & (1 << flags));
            }
        } else {
            func_80064C3C(flags, z, v, *a10, lights & (1 << flags));
        }
    }
    return ret;
}
