#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    u8 unk0[8];
    s32 off;                    /* +0x08 byte offset of the model body */
    void *verts;                /* +0x0C vertex list (after relocation) */
} ModelHdr;

extern void SetRotMatrix(MATRIX *m);
extern void SetLightMatrix(MATRIX *m);
extern void SetColorMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void SetBackColor(s32 r, s32 g, s32 b);
extern void RotTrans(SVECTOR *v0, VECTOR *v1, s32 *flag);
extern void func_8005A57C(s32 a, s32 b);
extern ModelHdr *volatile D_801A5F30;
extern ModelHdr *D_801A5F34[];
extern void *D_801A6544;
extern MATRIX D_801AD67C;

/* set up the GTE for one model at pos: camera rotation at double scale,
 * translation = camera-space pos / 4, the light set's matrices, then select
 * the model and its vertex list */
void func_8005E300(u16 model, SVECTOR *pos, MATRIX *light)
{
    MATRIX m;
    VECTOR v;
    s32 flag[3];        /* only flag[0] is written (RotTrans) */
    u16 *src;
    s16 *dst;
    s32 i;

    SetRotMatrix(&D_801AD67C);
    src = (u16 *)D_801AD67C.m[0];
    SetTransMatrix((MATRIX *)((u8 *)&D_801AD67C + 12));
    RotTrans(pos, &v, flag);
    v.vx >>= 2;
    v.vy >>= 2;
    v.vz >>= 2;
    SetTransMatrix((MATRIX *)((u8 *)&v - 20));
    dst = m.m[0];
    i = 9;
    do {
        *dst++ = *src++ * 2;
    } while (--i);
    SetRotMatrix(&m);
    SetColorMatrix((MATRIX *)((u8 *)light + 0x50));
    SetBackColor(M2C_FIELD(light, u8 *, 0x62), M2C_FIELD(light, u8 *, 0x63), M2C_FIELD(light, u8 *, 0x64));
    SetLightMatrix(light);
    D_801A5F30 = (ModelHdr *)((u8 *)D_801A5F34[model] + D_801A5F34[model]->off);
    D_801A6544 = D_801A5F30->verts;
    func_8005A57C(0, 0x1E00);
}
