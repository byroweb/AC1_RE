#include "ac1/ac1.h"

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the part table */
} ModelHdr;

/* render globals from 0x8019D968 (ovl203 layout) */
typedef struct {
    void *model;                /* +0x0000 (0x8019D968) */
    ModelHdr *tbl[0x1CA6];      /* +0x0004 (0x801ADCDC) */
    MATRIX cam;                 /* +0x729C (0x801A4C04) */
    u8 unk72e4[0x10];
    MATRIX cam2;                /* +0x72CC (0x801A4C34) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void func_80057640(s32 obj, ModelHdr *m, s32 z);
extern MATRIX D_800AD190;
extern s32 D_80194734;
extern RenderGlobals D_8019D968;

/* place model `model` at pos/rot (mode 0) or only re-translate it (mode != 0),
 * then hand it to func_80057640 */
void func_8005D66C(s8 mode, u16 model, SVECTOR *pos, SVECTOR *rot, s32 obj, s32 up, s32 down)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[16];      /* retail frame slots, never touched */
    s32 z;
    ModelHdr *m;

    z = up;
    d.vx = pos->vx - D_8019D968.cam.t[0];
    d.vy = pos->vy - D_8019D968.cam.t[1];
    d.vz = pos->vz - D_8019D968.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    m = D_8019D968.tbl[model];
    if (mode == 0) {
        D_8019D968.model = (u8 *)m + m->off;
        ApplyMatrix(&D_8019D968.cam, &d, (VECTOR *)D_800AD190.t);
        r.vx = rot->vx;
        r.vz = rot->vz;
        r.vy = 2048 - rot->vy;
        RotMatrixYXZ(&r, &D_800AD190);
        if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
            MulMatrix2(&D_8019D968.cam2, &D_800AD190);
            D_80194734 = 0;
        } else {
            MulMatrix2(&D_8019D968.cam, &D_800AD190);
            D_80194734 = 2;
            D_800AD190.t[0] <<= 2;
            D_800AD190.t[1] <<= 2;
            D_800AD190.t[2] <<= 2;
        }
    } else {
        ApplyMatrix(&D_8019D968.cam, &d, (VECTOR *)D_800AD190.t);
        D_800AD190.t[0] <<= D_80194734;
        D_800AD190.t[1] <<= D_80194734;
        D_800AD190.t[2] <<= D_80194734;
    }
    if (obj != 0) {
        func_80057640(obj, m, z);
    }
}
