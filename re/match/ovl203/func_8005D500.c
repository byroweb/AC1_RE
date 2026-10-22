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

/* render globals from 0x8019D968 */
typedef struct {
    ModelPart *model;           /* +0x0000 (0x8019D968) */
    ModelHdr *tbl[1];           /* +0x0004 (0x801A5F34) */
    u8 unk8[0x7294];
    MATRIX cam;                 /* +0x729C (0x801A4C04) */
    u8 unk776c[0x10];
    MATRIX cam2;                /* +0x72CC (0x801A4C34) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void func_8001462C(MATRIX *m, s32 scale);
extern void func_8005D15C(s32 part, s32 a, s32 b);
extern MATRIX D_800AD190;
extern s32 D_80194734;
extern RenderGlobals D_8019D968;
extern void *D_8019DF7C;

/* place model part `part` at pos with rotation rot and scale, then draw it */
void func_8005D500(s32 model, s32 part, SVECTOR *pos, SVECTOR *rot, s32 scale, s32 a5, s32 up,
                   s32 down)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[48];      /* retail frame slots, never touched */
    s32 z;

    z = up;
    d.vx = pos->vx - D_8019D968.cam.t[0];
    d.vy = pos->vy - D_8019D968.cam.t[1];
    d.vz = pos->vz - D_8019D968.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    D_8019D968.model = (ModelPart *)((u8 *)D_8019D968.tbl[model] + D_8019D968.tbl[model]->off);
    ApplyMatrix(&D_8019D968.cam, &d, (VECTOR *)D_800AD190.t);
    r.vx = rot->vx;
    r.vz = rot->vz;
    r.vy = 2048 - rot->vy;
    RotMatrixYXZ(&r, &D_800AD190);
    func_8001462C(&D_800AD190, scale);
    MulMatrix2(&D_8019D968.cam2, &D_800AD190);
    D_80194734 = 0;
    SetRotMatrix(&D_800AD190);
    SetTransMatrix(&D_800AD190);
    D_8019DF7C = *(void **)((u8 *)D_8019D968.model + (part * 32 + 12) - part * 4);
    func_8005D15C(part, a5, z);
}
