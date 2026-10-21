#include "ac1/ac1.h"

typedef struct {                /* 0x1C */
    s32 p00;                    /* +0x00 offsets, made absolute here */
    u8 pad04[4];
    s32 p08;                    /* +0x08 */
    u8 pad0c[2];
    u16 flags;                  /* +0x0E 0x8000 = unused, 0x4000 = no prims */
    s32 prims;                  /* +0x10 */
    u16 nprim;                  /* +0x14 */
    u16 idx;                    /* +0x16 */
    s32 verts;                  /* +0x18 */
} Rec1C;

typedef struct {
    u32 n;
    u32 unk04;
    u32 total;                  /* +0x08 */
    Rec1C rec[1];               /* +0x0C */
} RecSet;

/* Relocate a model's records and scale its primitives' screen coordinates
 * (x << 3 / y << 4 style fix-ups per GPU primitive type). */
void func_800574D8(RecSet *set) {
    Rec1C *r;
    s32 n;
    s32 k;
    u8 *prim;
    u16 *h;
    s32 len;
    u32 tag;
    s32 unused[2];              /* never touched: retail has the empty 8-byte frame (p15) */

    n = set->total;
    r = set->rec;
    while (--n != -1) {
        if (r->flags & 0x8000) {
            r->idx = 0xFFFF;
        } else {
            r->p00 += 12;
            r->p00 += (s32)set;
            r->p08 += 12;
            r->p08 += (s32)set;
            r->prims += 12;
            r->prims += (s32)set;
            r->verts += 12;
            r->verts += (s32)set;
            if (!(r->flags & 0x4000)) {
                k = r->nprim + (r->flags & 0x1FF);
                prim = (u8 *)r->prims;
                while (--k != -1) {
                    h = (u16 *)(prim + 4);
                    len = prim[1];
                    tag = *(u32 *)prim;
                    prim = (u8 *)h + len * 4;
                    switch ((tag >> 24) & 0xBC) {
            case 0x20:
                h[3] <<= 4;
                h[4] <<= 4;
                h[5] <<= 4;
                h[2] <<= 3;
                break;
            case 0x30:
                h[3] <<= 4;
                h[5] <<= 4;
                h[7] <<= 4;
                h[2] <<= 3;
                h[4] <<= 3;
                h[6] <<= 3;
                break;
            case 0x24:
                h[7] <<= 4;
                h[8] <<= 4;
                h[9] <<= 4;
                h[6] <<= 3;
                break;
            case 0x34:
                h[7] <<= 4;
                h[9] <<= 4;
                h[11] <<= 4;
                h[6] <<= 3;
                h[8] <<= 3;
                h[10] <<= 3;
                break;
            case 0x28:
                h[3] <<= 4;
                h[4] <<= 4;
                h[5] <<= 4;
                h[6] <<= 4;
                h[2] <<= 3;
                break;
            case 0x38:
                h[3] <<= 4;
                h[5] <<= 4;
                h[7] <<= 4;
                h[9] <<= 4;
                h[2] <<= 3;
                h[4] <<= 3;
                h[6] <<= 3;
                h[8] <<= 3;
                break;
            case 0x2C:
                h[9] <<= 4;
                h[10] <<= 4;
                h[11] <<= 4;
                h[12] <<= 4;
                h[8] <<= 3;
                break;
            case 0x3C:
                h[9] <<= 4;
                h[11] <<= 4;
                h[13] <<= 4;
                h[15] <<= 4;
                h[8] <<= 3;
                h[10] <<= 3;
                h[12] <<= 3;
                h[14] <<= 3;
                break;
            case 0xA0:
                h[2] <<= 4;
                h[3] <<= 4;
                h[4] <<= 4;
                break;
            case 0xB0:
                h[6] <<= 4;
                h[7] <<= 4;
                h[8] <<= 4;
                break;
            case 0xA4:
                h[8] <<= 4;
                h[9] <<= 4;
                h[10] <<= 4;
                break;
            case 0xB4:
                h[12] <<= 4;
                h[13] <<= 4;
                h[14] <<= 4;
                break;
            case 0xA8:
                h[2] <<= 4;
                h[3] <<= 4;
                h[4] <<= 4;
                h[5] <<= 4;
                break;
            case 0xB8:
                h[8] <<= 4;
                h[9] <<= 4;
                h[10] <<= 4;
                h[11] <<= 4;
                break;
            case 0xAC:
                h[10] <<= 4;
                h[11] <<= 4;
                h[12] <<= 4;
                h[13] <<= 4;
                break;
            case 0xBC:
                h[16] <<= 4;
                h[17] <<= 4;
                h[18] <<= 4;
                h[19] <<= 4;
                break;
            }
                }
            }
        }
        r++;
    }
}
