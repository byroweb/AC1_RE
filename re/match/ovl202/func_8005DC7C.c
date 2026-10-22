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
extern void func_80057C44(s32 a, ModelHdr *h, s32 z);
extern MATRIX D_800B1228;
extern s32 D_801987CC;
extern RenderGlobals D_801A5F30;
extern void *D_801A6544;

/* ovl202 0x8005DC7C - place model `model` at pos/rot relative to the camera; mode 0 builds
 * the full local matrix (near: camera rot, OT shift 2; far: cam2, shift 0), else only the
 * translation is refreshed; then draws it through func_80057C44 when a4 != 0 */
void func_8005DC7C(s8 mode, u16 model, SVECTOR *pos, SVECTOR *rot, s32 a4, s32 up, s32 down)
{
    SVECTOR d;
    SVECTOR r;
    s32 z;
    ModelHdr *h;
    u8 unused[16];      /* retail frame slots, never touched */

    z = up;
    d.vx = pos->vx - D_801A5F30.cam.t[0];
    d.vy = pos->vy - D_801A5F30.cam.t[1];
    d.vz = pos->vz - D_801A5F30.cam.t[2];
    if (d.vy <= 0) {
        z = down;
    }
    h = D_801A5F30.tbl[model];
    if (mode == 0) {
        D_801A5F30.model = (ModelPart *)((u8 *)h + h->off);
        ApplyMatrix(&D_801A5F30.cam, &d, (VECTOR *)D_800B1228.t);
        r.vx = rot->vx;
        r.vz = rot->vz;
        r.vy = 2048 - rot->vy;
        RotMatrixYXZ(&r, &D_800B1228);
        if (d.vx > 4096 || d.vy > 4096 || d.vz > 4096 || d.vx < -4096 || d.vy < -4096 || d.vz < -4096) {
            MulMatrix2(&D_801A5F30.cam2, &D_800B1228);
            D_801987CC = 0;
        } else {
            MulMatrix2(&D_801A5F30.cam, &D_800B1228);
            D_801987CC = 2;
            D_800B1228.t[0] *= 4;
            D_800B1228.t[1] *= 4;
            D_800B1228.t[2] *= 4;
        }
    } else {
        ApplyMatrix(&D_801A5F30.cam, &d, (VECTOR *)D_800B1228.t);
        D_800B1228.t[0] <<= D_801987CC;
        D_800B1228.t[1] <<= D_801987CC;
        D_800B1228.t[2] <<= D_801987CC;
    }
    if (a4 != 0) {
        func_80057C44(a4, h, z);
    }
}
