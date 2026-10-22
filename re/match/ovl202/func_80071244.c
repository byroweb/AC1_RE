#include "ac1/ac1.h"

typedef struct {
    s32 w0;             /* +0x00 */
    void *src;          /* +0x04 */
    s32 w8;             /* +0x08 */
    s16 x;              /* +0x0C */
    s16 y;              /* +0x0E */
    s16 z;              /* +0x10 */
    u8 pad12[0x0A];
} HitRec;

typedef struct {
    u8 pad[0x12];
    u16 y;              /* +0x12 */
} SrcObj;

typedef struct {
    u8 pad[0x48];
    HitRec *out;        /* +0x48 */
    u8 pad4c[0x44];
    SrcObj *src;        /* +0x90 */
    s32 w94;            /* +0x94 */
    u8 pad98[0x0C];
    s32 d;              /* +0xA4 plane constant */
    s32 kind;           /* +0xA8 */
    s32 shape;          /* +0xAC */
    s32 wb0;            /* +0xB0 */
    u8 padb4[0x20];
    s32 e[4];           /* +0xD4 */
    u8 pade4[0x18];
    s32 p[4];           /* +0xFC */
    s16 ny;             /* +0x10C */
    s16 nx;             /* +0x10E */
    s16 nz;             /* +0x110 */
    s16 ymin;           /* +0x112 */
    s16 ymax;           /* +0x114 */
} Face;

extern void func_80071220(s32 a, s32 b, s32 xz, s32 *out);

void func_80071244(Face *f, s32 x, s32 z)
{
    s32 y;
    s32 xz;
    SVECTOR unused;

    y = -(f->nx * x + f->nz * z + f->d) / f->ny;
    xz = z & 0xFFFF;
    xz |= x << 16;
    if (y < f->ymin) {
        return;
    }
    if (y > f->ymax) {
        return;
    }
    if ((f->kind & 7) == 1) {
        func_80071220(f->p[0], f->p[1], xz, &f->e[0]);
        func_80071220(f->p[2], f->p[0], xz, &f->e[1]);
        if ((f->shape & 0x7FF80000) == 0x7FF80000) {
            func_80071220(f->p[1], f->p[2], xz, &f->e[2]);
            if (f->e[0] >= 0 && f->e[1] >= 0 && f->e[2] >= 0) {
                goto ok;
            }
            if (f->e[0] <= 0 && f->e[1] <= 0 && f->e[2] <= 0) {
                goto ok;
            }
            return;
        } else {
            func_80071220(f->p[1], f->p[3], xz, &f->e[2]);
            func_80071220(f->p[3], f->p[2], xz, &f->e[3]);
            if (f->e[0] < 0 || f->e[1] < 0 || f->e[2] < 0 || f->e[3] < 0) {
                if (f->e[0] > 0 || f->e[1] > 0 || f->e[2] > 0 || f->e[3] > 0) {
                    return;
                }
            }
        }
    }
ok:
    f->out->y = f->src->y + (y >> 2);
    f->out->x = x;
    f->out->z = z;
    f->out->src = f->src;
    f->out->w0 = f->wb0;
    f->out->w8 = f->w94;
    f->out++;
}
