#include "ac1/ac1.h"

typedef struct {
    u8 pad[0xA4];
    s32 d;              /* +0xA4 plane constant */
    s32 kind;           /* +0xA8 low 3 bits: 1 = bounded polygon */
    s32 shape;          /* +0xAC 0x7FF80000 = triangle */
    u8 padb0[0x24];
    s32 e[4];           /* +0xD4 edge tests */
    u8 pade4[8];
    s32 p[4];           /* +0xEC corner records */
    s16 ny;             /* +0xFC */
    s16 nx;             /* +0xFE */
    s16 nz;             /* +0x100 */
    s16 ymin;           /* +0x102 */
    s16 ymax;           /* +0x104 */
} Face;

extern void func_8006F028(s32 a, s32 b, s32 xz, s32 *out);

s32 func_8006E8D4(Face *f, s32 x, s32 z)
{
    s32 y;
    s32 side;
    s32 xz;
    SVECTOR unused;

    y = -(f->nx * x + f->nz * z + f->d) / f->ny;
    xz = z & 0xFFFF;
    xz |= x << 16;
    if (f->ny > 0) {
        goto pos;
    }
    if (y < f->ymin) {
        return 0;
    }
    if (y > f->ymax) {
        return 0;
    }
    side = 1;
common:
    if ((f->kind & 7) != 1) {
        return side;
    }
    func_8006F028(f->p[0], f->p[1], xz, &f->e[0]);
    func_8006F028(f->p[2], f->p[0], xz, &f->e[1]);
    if ((f->shape & 0x7FF80000) == 0x7FF80000) {
        func_8006F028(f->p[1], f->p[2], xz, &f->e[2]);
        if (!(f->e[0] >= 0 && f->e[1] >= 0 && f->e[2] >= 0) && !(f->e[0] <= 0 && f->e[1] <= 0 && f->e[2] <= 0)) {
            return 0;
        }
        return side;
    }
    func_8006F028(f->p[1], f->p[3], xz, &f->e[2]);
    func_8006F028(f->p[3], f->p[2], xz, &f->e[3]);
    if ((f->e[0] >= 0 && f->e[1] >= 0 && f->e[2] >= 0 && f->e[3] >= 0) || (f->e[0] <= 0 && f->e[1] <= 0 && f->e[2] <= 0 && f->e[3] <= 0)) {
        return side;
    }
    return 0;
pos:
    if (y < f->ymin) {
        return 0;
    }
    if (y > f->ymax) {
        return 0;
    }
    side = 2;
    goto common;
}
