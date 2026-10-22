#include "ac1/ac1.h"
#include "ac1/ovl202_t42.h"

typedef struct {                /* libgpu POLY_G3 */
    u32 tag;
    u32 rgb0;                   /* r0 g0 b0 code */
    u32 xy0;
    u32 rgb1;
    u32 xy1;
    u32 rgb2;
    u32 xy2;
} PolyG3w;

/* Gouraud triangle from three screen points (stride 8) and three colours. */
void func_8008C95C(u32 *xy, u32 *rgb, s32 z) {
    PolyG3w *p;

    p = (PolyG3w *)SPAD->cur;
    SPAD->cur += sizeof(PolyG3w);
    p->xy0 = xy[0];
    p->xy1 = xy[2];
    p->xy2 = xy[4];
    p->rgb0 = rgb[0];
    p->rgb1 = rgb[1];
    p->rgb2 = rgb[2];
    setlen(p, 6);
    addPrim(SPAD->ot + z, p);
}
