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

/* render globals from 0x801ADCD8 (ovl204 layout) */
typedef struct {
    u32 *model;                 /* +0x0000 (0x801ADCD8) */
    ModelHdr *tbl[0x1960];      /* +0x0004 (0x801ADCDC) */
    LightSet light[0x20];       /* +0x6584 (0x801B425C) */
    u32 fog;                    /* +0x7284 (0x801B4F5C) */
    u8 unk7288[0x72C4 - 0x7288];
    MATRIX cam;                 /* +0x72C4 (0x801B4F9C) */
    u8 unk72e4[0x10];
    MATRIX cam2;                /* +0x72F4 (0x801B4FCC) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern MATRIX *MulMatrix0(void *m0, MATRIX *m1, MATRIX *m2);
extern void SetColorMatrix(MATRIX *m);
extern void SetLightMatrix(MATRIX *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void SetFogNearFar(s32 a, s32 b, s32 h);
extern void func_8005C294(u16 *a);
extern s32 func_80066D90(SVECTOR *pos, s32 a, s32 b);
extern void func_80057CA8(s32 o, ModelHdr *m, s32 z, s32 a, s32 b, s32 n);
extern void func_80064CC8(s32 o, ModelHdr *m, s32 z, s32 v, s32 a, s32 b, s32 n);
extern MATRIX D_800AD348;
extern s32 D_801A4964;
extern RenderGlobals D_801ADCD8;
extern WStruct D_801D4488[];

void func_8005D068(u16 model, SVECTOR *pos, SVECTOR *rot, s32 o, u16 *flag, s32 b, s32 ws,
                   s32 up, s32 down, s32 c, s32 e)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[16];
    MATRIX lm;
    s32 z;
    ModelHdr *m;
    LightSet *lt;
    s32 v;
    u32 f;
    s32 n;

    z = up;
    d.vx = pos->vx - D_801ADCD8.cam.t[0];
    d.vy = pos->vy - D_801ADCD8.cam.t[1];
    d.vz = pos->vz - D_801ADCD8.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    lt = &D_801ADCD8.light[D_801D4488[ws].light];
    m = D_801ADCD8.tbl[model];
    D_801ADCD8.model = (u32 *)((u8 *)m + m->off);
    SetColorMatrix((MATRIX *)lt->col);
    SetBackColor(lt->r, lt->g, lt->b);
    ApplyMatrix(&D_801ADCD8.cam, &d, (VECTOR *)D_800AD348.t);
    if (*flag != 0) {
        func_8005C294(flag);
    }
    r.vx = rot->vx;
    r.vz = rot->vz;
    r.vy = 2048 - rot->vy;
    RotMatrixYXZ(&r, &D_800AD348);
    MulMatrix0(lt, &D_800AD348, &lm);
    SetLightMatrix(&lm);
    if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
        f = lt->fog;
        if (f != 0) {
            f *= 2;
            SetFogNearFar(f >> 4, f >> 2, 80);
        }
        D_801ADCD8.fog = lt->fog;
        MulMatrix2(&D_801ADCD8.cam2, &D_800AD348);
        D_801A4964 = 0;
    } else {
        f = lt->fog;
        if (f != 0) {
            f *= 2;
            SetFogNearFar(f >> 2, f, 80);
        }
        D_801ADCD8.fog = lt->fog;
        MulMatrix2(&D_801ADCD8.cam, &D_800AD348);
        D_801A4964 = 2;
        D_800AD348.t[0] <<= 2;
        D_800AD348.t[1] <<= 2;
        D_800AD348.t[2] <<= 2;
    }
    v = func_80066D90(pos, c, e);
    n = *D_801ADCD8.model;
    n <<= 1;
    if (v == 0) {
        func_80057CA8(o, m, z, *flag, b, n);
    } else {
        func_80064CC8(o, m, z, v, *flag, b, n);
    }
}
