#include "ac1/ac1.h"

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the part table */
} ModelHdr;

typedef struct {
    u8 lm[0x50];                /* +0x00 light matrix */
    s16 col[9];                 /* +0x50 colour matrix (3x3) */
    u8 r, g, b;                 /* +0x62 back colour */
    u8 pad65;
    u16 fog;                    /* +0x66 */
} LightSet;                     /* 0x68 */

typedef struct {
    u8 unk0[0x30];
    u8 light;                   /* +0x30 */
    u8 unk31[0x0F];
} WStruct;                      /* 0x40 */

typedef struct {
    s32 w[5];                   /* 3x3 s16 rotation + pad, copied as 5 words */
} RotPart;

/* render globals from 0x8019D968 (ovl203 layout, = ovl204 minus 0x28 before cam) */
typedef struct {
    u32 *model;                 /* +0x0000 (0x801ADCD8) */
    ModelHdr *tbl[0x1960];      /* +0x0004 (0x801ADCDC) */
    LightSet light[0x20];       /* +0x6584 (0x801B425C) */
    u32 fog;                    /* +0x7284 (0x801B4F5C) */
    u8 unk7288[0x729C - 0x7288];
    MATRIX cam;                 /* +0x72C4 (0x801B4F9C) */
    u8 unk72e4[0x10];
    MATRIX cam2;                /* +0x72F4 (0x801B4FCC) */
    u8 unk7314[0x7434 - 0x7314];
    RotPart rot2;               /* +0x7434 (0x801B510C) */
} RenderGlobals;


extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern MATRIX *MulMatrix0(void *m0, MATRIX *m1, MATRIX *m2);
extern void SetColorMatrix(MATRIX *m);
extern void SetLightMatrix(void *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void func_8005CAE0(u16 *a);
extern s32 func_80066F54(SVECTOR *pos, s32 a, s32 b);
extern s32 func_8005414C(s32 *o, s32 model, s32 part, s32 a, s32 b, s32 c, s32 d, s32 e);
extern void func_800584F4(s32 o, ModelHdr *m, s32 z, s32 a, s32 b, s32 n);
extern void func_80064E8C(s32 o, ModelHdr *m, s32 z, s32 v, s32 a, s32 b, s32 n);
extern void func_80057A3C(s32 part, s32 z, s32 mask);
extern void func_80058B88(s32 part, s32 z, s32 flag, s32 mask);
extern void func_800640DC(s32 part, s32 z, s32 v, s32 flag, s32 mask);
extern MATRIX D_800AD190;
extern s32 D_80194734;
extern RenderGlobals D_8019D968;
extern MATRIX D_801A4C04;
extern RotPart D_801A4D54;
extern void *D_8019DF7C;
extern WStruct D_801C40F0[];

/* place model `model` (lit by the world structure's light set) and draw it, or
 * with part bit 0x80 draw one part with the stored rotation */
s32 func_8005CB8C(u16 model, SVECTOR *pos, SVECTOR *rot, s32 *o, s32 part, s32 a5, s32 a6, s32 a7,
                  s32 a8, s32 a9, u16 *flag, s32 mask, s32 ws, s32 up, s32 down, s32 c, s32 e)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[48];
    MATRIX lm;
    s32 z;
    ModelHdr *m;
    LightSet *lt;
    s32 v;
    u32 f;
    s32 ret;
    s32 i;
    s16 *p;
    s32 *src;
    s32 *dst;

    z = up;
    d.vx = pos->vx - D_8019D968.cam.t[0];
    d.vy = pos->vy - D_8019D968.cam.t[1];
    d.vz = pos->vz - D_8019D968.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    lt = &D_8019D968.light[D_801C40F0[ws].light];
    m = D_8019D968.tbl[model];
    D_8019D968.model = (u32 *)((u8 *)m + m->off);
    SetColorMatrix((MATRIX *)lt->col);
    SetBackColor(lt->r, lt->g, lt->b);
    ApplyMatrix(&D_801A4C04, &d, (VECTOR *)D_800AD190.t);
    if (part & 0x80) {
        part &= 0x7F;
        SetLightMatrix(lt);
        if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
            {
                s32 *s1;
                s32 *d1;
                s32 n1;

                s1 = D_801A4D54.w;
                d1 = (s32 *)&D_800AD190;
                for (n1 = 5; n1 != 0; n1--) {
                    *d1++ = *s1++;
                }
            }
            D_80194734 = 1;
            D_800AD190.t[0] <<= 1;
            D_800AD190.t[1] <<= 1;
            D_800AD190.t[2] <<= 1;
            ret = 1;
        } else {
            {
                s32 *s2;
                s32 *d2;
                s32 n2;

                s2 = D_8019D968.rot2.w;
                d2 = (s32 *)&D_800AD190;
                for (n2 = 5; n2 != 0; n2--) {
                    *d2++ = *s2++;
                }
            }
            p = &D_800AD190.m[0][0];
            for (i = 9; i != 0; i--) {
                *p = *p >> 1;
                p++;
            }
            D_80194734 = 2;
            D_800AD190.t[0] <<= 2;
            D_800AD190.t[1] <<= 2;
            D_800AD190.t[2] <<= 2;
            ret = 2;
        }
    } else {
        if (*flag != 0) {
            func_8005CAE0(flag);
        }
        r.vx = rot->vx;
        r.vz = rot->vz;
        r.vy = 2048 - rot->vy;
        RotMatrixYXZ(&r, &D_800AD190);
        MulMatrix0(lt, &D_800AD190, &lm);
        SetLightMatrix(&lm);
        if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
            f = lt->fog;
            if (f != 0) {
                f *= 2;
                SetFogNearFar(f >> 4, f >> 2, 160);
            }
            D_8019D968.fog = lt->fog;
            MulMatrix2(&D_8019D968.cam2, &D_800AD190);
            ret = 1;
            D_80194734 = 0;
        } else {
            f = lt->fog;
            if (f != 0) {
                f *= 2;
                SetFogNearFar(f >> 2, f, 160);
            }
            D_8019D968.fog = lt->fog;
            MulMatrix2(&D_801A4C04, &D_800AD190);
            ret = 1;
            D_80194734 = 2;
            D_800AD190.t[0] <<= 2;
            D_800AD190.t[1] <<= 2;
            D_800AD190.t[2] <<= 2;
        }
    }
    if (m->unk0[5] != 0) {
        v = func_80066F54(pos, c, e);
        if (func_8005414C(o, model, part, a5, a6, a7, a8, a9) != 0) {
            if (v == 0) {
                func_800584F4(*o, m, z, *flag, mask, 0);
            } else {
                func_80064E8C(*o, m, z, v, *flag, mask, 0);
            }
        }
    } else {
        v = func_80066F54(pos, c, e);
        SetRotMatrix(&D_800AD190);
        SetTransMatrix(&D_800AD190);
        D_8019DF7C = *(void **)((u8 *)D_8019D968.model + (part * 32 + 12) - part * 4);
        if (v == 0) {
            if (*flag == 0) {
                func_80057A3C((u16)part, z, mask & (1 << part));
            } else {
                func_80058B88((u16)part, z, *flag, mask & (1 << part));
            }
        } else {
            func_800640DC((u16)part, z, v, *flag, mask & (1 << part));
        }
    }
    return ret;
}
