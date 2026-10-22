#include "ac1/ac1.h"

typedef struct {
    u8 unk0[0x0C];
    void *verts;                /* +0x0C */
    u8 unk10[0x0C];
} ModelPart;

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the part table */
} ModelHdr;

/* render globals from 0x801A5F30 */
typedef struct {
    ModelPart *model;           /* +0x0000 (0x801A5F30) */
    ModelHdr *tbl[1];           /* +0x0004 (0x801A5F34) */
    u8 unk8[0x7744];
    MATRIX cam;                 /* +0x774C (0x801AD67C) */
    u8 unk776c[0x10];
    MATRIX cam2;                /* +0x777C (0x801AD6AC) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void func_8001462C(MATRIX *m, s32 scale);
extern void func_8005D76C(s32 part, s32 a, s32 b);
extern MATRIX D_800B1228;
extern s32 D_801987CC;
extern RenderGlobals D_801A5F30;
extern void *D_801A6544;

/* place model part `part` at pos with rotation rot and scale, then draw it */
void func_8005DB10(s32 model, s32 part, SVECTOR *pos, SVECTOR *rot, s32 scale, s32 a5, s32 up,
                   s32 down)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[48];      /* retail frame slots, never touched */
    s32 z;

    z = up;
    d.vx = pos->vx - D_801A5F30.cam.t[0];
    d.vy = pos->vy - D_801A5F30.cam.t[1];
    d.vz = pos->vz - D_801A5F30.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    D_801A5F30.model = (ModelPart *)((u8 *)D_801A5F30.tbl[model] + D_801A5F30.tbl[model]->off);
    ApplyMatrix(&D_801A5F30.cam, &d, (VECTOR *)D_800B1228.t);
    r.vx = rot->vx;
    r.vz = rot->vz;
    r.vy = 2048 - rot->vy;
    RotMatrixYXZ(&r, &D_800B1228);
    func_8001462C(&D_800B1228, scale);
    MulMatrix2(&D_801A5F30.cam2, &D_800B1228);
    D_801987CC = 0;
    SetRotMatrix(&D_800B1228);
    SetTransMatrix(&D_800B1228);
    D_801A6544 = *(void **)((u8 *)D_801A5F30.model + (part * 32 + 12) - part * 4);
    func_8005D76C(part, a5, z);
}
