#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x58];
    u8 u58[8];          /* +0x58 */
    s32 u60;            /* +0x60 */
    u16 bmin[3];        /* +0x64 */
    u16 pad6a;
    u16 bmax[3];        /* +0x6C */
    u8 pad72[0x2A];
    u16 axis;           /* +0x9C (word read: bits 2..3 = dominant axis) */
    u16 n[3];           /* +0x9E */
    u8 pada4[0x1C];
    u16 *corner[4];     /* +0xC0 */
    s32 rd0;            /* +0xD0 */
    u8 padd4[0x18];
    u16 p[4][2];        /* +0xEC */
    volatile u16 ny;             /* +0xFC */
    volatile u16 nx;             /* +0xFE */
    volatile u16 nz;             /* +0x100 */
    volatile u16 ymin;           /* +0x102 */
    volatile u16 ymax;           /* +0x104 */
} Face;

extern s32 func_8006E8D4(Face *f, s32 x, s32 z);
extern void func_8006F04C(void *a, u16 *n, s32 *out);

s32 func_8006EB10(void *unused, Face *f)
{
    s16 x0, x1, z0, z1;
    u16 ny, nx, nz, ymin, ymax;
    s32 base;
    s32 r;

    if (f->u60 != 0) {
        func_8006F04C(f->u58, f->n, &f->rd0);
        if (f->rd0 < 0) {
            return 0;
        }
    }
    switch ((s32)((*(u32 *)&f->axis >> 2) & 3)) {
    case 0:
        f->p[0][0] = f->corner[0][2];
        f->p[0][1] = f->corner[0][1];
        f->p[1][0] = f->corner[1][2];
        f->p[1][1] = f->corner[1][1];
        f->p[2][0] = f->corner[2][2];
        f->p[2][1] = f->corner[2][1];
        if ((*(s32 *)((u8 *)f + 0xAC) & 0x7FF80000) != 0x7FF80000) {
            f->p[3][0] = f->corner[3][2];
            f->p[3][1] = f->corner[3][1];
        }
        x0 = f->bmin[1];
        x1 = f->bmax[1];
        z0 = f->bmin[2];
        z1 = f->bmax[2];
        base = 0;
        f->ny = f->n[0];
        f->nx = f->n[1];
        f->nz = f->n[2];
        f->ymin = f->bmin[0];
        f->ymax = f->bmax[0];
    common:
        if ((r = func_8006E8D4(f, x0, z0)) != 0) {
            return r + base;
        }
        if ((r = func_8006E8D4(f, x1, z0)) != 0) {
            return r + base + 4;
        }
        if ((r = func_8006E8D4(f, x0, z1)) != 0) {
            return r + base + 8;
        }
        if ((r = func_8006E8D4(f, x1, z1)) != 0) {
            return r + base + 12;
        }
        break;
    case 1:
        f->p[0][0] = f->corner[0][2];
        f->p[0][1] = f->corner[0][0];
        f->p[1][0] = f->corner[1][2];
        f->p[1][1] = f->corner[1][0];
        f->p[2][0] = f->corner[2][2];
        f->p[2][1] = f->corner[2][0];
        if ((*(s32 *)((u8 *)f + 0xAC) & 0x7FF80000) != 0x7FF80000) {
            f->p[3][0] = f->corner[3][2];
            f->p[3][1] = f->corner[3][0];
        }
        x0 = f->bmin[0];
        x1 = f->bmax[0];
        z0 = f->bmin[2];
        z1 = f->bmax[2];
        base = 16;
        f->ny = f->n[1];
        f->nx = f->n[0];
        f->nz = f->n[2];
        f->ymin = f->bmin[1];
        f->ymax = f->bmax[1];
        goto common;
    case 2:
        f->p[0][0] = f->corner[0][1];
        f->p[0][1] = f->corner[0][0];
        f->p[1][0] = f->corner[1][1];
        f->p[1][1] = f->corner[1][0];
        f->p[2][0] = f->corner[2][1];
        f->p[2][1] = f->corner[2][0];
        if ((*(s32 *)((u8 *)f + 0xAC) & 0x7FF80000) != 0x7FF80000) {
            f->p[3][0] = f->corner[3][1];
            f->p[3][1] = f->corner[3][0];
        }
        x0 = f->bmin[0];
        x1 = f->bmax[0];
        z0 = f->bmin[1];
        z1 = f->bmax[1];
        base = 32;
        f->ny = f->n[2];
        f->nx = f->n[0];
        f->nz = f->n[1];
        f->ymin = f->bmin[2];
        f->ymax = f->bmax[2];
        goto common;
    default:
        return -1;
    }
    return -1;
}
