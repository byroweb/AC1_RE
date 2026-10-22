#include "ac1/ac1.h"

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the part table */
} ModelHdr;

/* render globals from 0x801ADCD8 (ovl204 layout) */
typedef struct {
    void *model;                /* +0x0000 (0x801ADCD8) */
    ModelHdr *tbl[0x1CB0];      /* +0x0004 (0x801ADCDC) */
    MATRIX cam;                 /* +0x72C4 (0x801B4F9C) */
    u8 unk72e4[0x10];
    MATRIX cam2;                /* +0x72F4 (0x801B4FCC) */
} RenderGlobals;

extern void ApplyMatrix(MATRIX *m, SVECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrixYXZ(SVECTOR *r, MATRIX *m);
extern MATRIX *MulMatrix2(MATRIX *m0, MATRIX *m1);
extern void func_80056DF4(s32 obj, ModelHdr *m, s32 z);
extern MATRIX D_800AD348;
extern s32 D_801A4964;
extern RenderGlobals D_801ADCD8;

/* place model `model` at pos/rot (mode 0) or only re-translate it (mode != 0),
 * then hand it to func_80056DF4 */
void func_8005CE20(s8 mode, u16 model, SVECTOR *pos, SVECTOR *rot, s32 obj, s32 up, s32 down)
{
    SVECTOR d;
    SVECTOR r;
    u8 unused[16];      /* retail frame slots, never touched */
    s32 z;
    ModelHdr *m;

    z = up;
    d.vx = pos->vx - D_801ADCD8.cam.t[0];
    d.vy = pos->vy - D_801ADCD8.cam.t[1];
    d.vz = pos->vz - D_801ADCD8.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    m = D_801ADCD8.tbl[model];
    if (mode == 0) {
        D_801ADCD8.model = (u8 *)m + m->off;
        ApplyMatrix(&D_801ADCD8.cam, &d, (VECTOR *)D_800AD348.t);
        r.vx = rot->vx;
        r.vz = rot->vz;
        r.vy = 2048 - rot->vy;
        RotMatrixYXZ(&r, &D_800AD348);
        if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
            MulMatrix2(&D_801ADCD8.cam2, &D_800AD348);
            D_801A4964 = 0;
        } else {
            MulMatrix2(&D_801ADCD8.cam, &D_800AD348);
            D_801A4964 = 2;
            D_800AD348.t[0] <<= 2;
            D_800AD348.t[1] <<= 2;
            D_800AD348.t[2] <<= 2;
        }
    } else {
        ApplyMatrix(&D_801ADCD8.cam, &d, (VECTOR *)D_800AD348.t);
        D_800AD348.t[0] <<= D_801A4964;
        D_800AD348.t[1] <<= D_801A4964;
        D_800AD348.t[2] <<= D_801A4964;
    }
    if (obj != 0) {
        func_80056DF4(obj, m, z);
    }
}
