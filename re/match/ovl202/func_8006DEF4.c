#include "ac1/ac1.h"
/* ovl202 0x8006DEF4 - candidate world structs for a box: OR the bucket rows it spans on
 * each axis (func_8006DE54), then AND the three axes into s->z */
typedef struct BucketScr {
    u32 z[16];                  /* +0x00 */
    s32 n;                      /* +0x40 */
    s32 skip;                   /* +0x44 */
    u8 unk48[0x38];
    u32 x[16];                  /* +0x80 */
    u32 y[16];                  /* +0xC0 */
} BucketScr;
typedef struct Buckets {
    s32 n;                      /* +0x0000 words per bucket row */
    u8 unk0004[0x1C1C];
    u32 tab[3][64][16];         /* +0x1C20 x, y, z bucket rows */
} Buckets;
extern Buckets D_801E8B70;
void func_8006DE54(u32 *tab, s16 lo, s16 hi, u32 *out);


void func_8006DEF4(CollBox *b, BucketScr *s)
{
    u32 *px;
    u32 *py;
    u32 *o;
    s32 m;
    s32 n;

    m = D_801E8B70.n;
    s->n = m;
    s->skip = 16 - m;
    func_8006DE54(D_801E8B70.tab[0][0], b->min.vx, b->max.vx, s->x);
    func_8006DE54(D_801E8B70.tab[1][0], b->min.vy, b->max.vy, s->y);
    func_8006DE54(D_801E8B70.tab[2][0], b->min.vz, b->max.vz, s->z);
    px = s->x;
    py = s->y;
    o = s->z;
    n = D_801E8B70.n;
    do {
        *o++ &= *px++ & *py++;
    } while (--n != 0);
}
