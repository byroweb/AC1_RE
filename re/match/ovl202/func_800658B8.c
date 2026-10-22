#include "ac1/ac1.h"

typedef struct {
    u8 unk0[4];
    SVECTOR c;                  /* +0x04 centre (pad = off flag) */
    SVECTOR a;                  /* +0x0C */
    SVECTOR b;                  /* +0x14 */
    SVECTOR d;                  /* +0x1C */
    s32 w;                      /* +0x24 */
    s32 unk28;
    s16 r;                      /* +0x2C */
    s16 unk2e;
} Sphere48;

typedef struct {
    Sphere48 s[16];
    s32 unk300;
    s32 n;                      /* +0x304 (0x801AEB30) */
} SphereSet;

typedef struct {
    SVECTOR a;
    SVECTOR b;
    SVECTOR d;
    s32 w;
    s16 r;
    s16 pad;
} ScrSphere;

typedef struct {
    u8 unk0[0x5C];
    ScrSphere s[4];             /* +0x5C */
    s32 n;                      /* +0xDC */
} SphereScratch;

#define SCR ((SphereScratch *)0x1F800000)

extern volatile s32 D_801987CC;
extern SphereSet D_801AE82C;
extern s32 D_801AEB30;

/* copy (scaled by 1 << D_801987CC) up to four of the spheres selected by
 * mask into the scratchpad */
void func_800658B8(u32 mask)
{
    Sphere48 *src;
    ScrSphere *dst;
    s32 i;
    s32 sh;
    SphereScratch *scr;

    dst = SCR->s;
    src = D_801AE82C.s;
    scr = SCR;
    SCR->n = 0;
    i = D_801AEB30;
    while (--i != -1) {
        if (mask & 1) {
            dst->a.vx = src->a.vx << (sh = D_801987CC);
            dst->a.vy = src->a.vy << sh;
            dst->a.vz = src->a.vz << sh;
            dst->b.vx = src->b.vx << sh;
            dst->d.vx = src->d.vx << sh;
            dst->b.vy = src->b.vy << sh;
            dst->d.vy = src->d.vy << sh;
            dst->b.vz = src->b.vz << sh;
            dst->d.vz = src->d.vz << sh;
            dst->w = src->w;
            dst->r = src->r << sh;
            dst++;
            if (++scr->n >= 4) {
                return;
            }
        }
        mask >>= 1;
        src++;
    }
}
