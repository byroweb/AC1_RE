#include "ac1/ac1.h"

#define gte_ldlvl(r0) __asm__ volatile ( \
    "lwc2 $9, 0(%0)\n\t" \
    "lwc2 $10, 4(%0)\n\t" \
    "lwc2 $11, 8(%0)" : : "r"(r0))
#define gte_sqr0() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4AA00428")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25, 0(%0)\n\t" \
    "swc2 $26, 4(%0)\n\t" \
    "swc2 $27, 8(%0)" : : "r"(r0) : "memory")

typedef struct {
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} Pos16;

extern s16 D_80041B90;
extern s16 D_80041B92;
extern s16 D_80041B94;
extern s16 D_80041B9A;
extern long SquareRoot0(long a);
extern int rsin(int a);
extern int rcos(int a);
extern long ratan2(long y, long x);
extern void func_8004D178(s32 id, s32 key, s32 vl, s32 vr, s32 pan);

s32 func_8004CC30(s32 id, s32 key, Pos16 *pos, s32 vol, s32 range, s32 len, s32 pitch)
{
    VECTOR *d = (VECTOR *)0x1F8003F0;
    VECTOR *sq = (VECTOR *)0x1F8003E0;
    s32 dist;
    s32 ang;
    s32 vl;
    s32 vr;
    s32 v;

    id &= 0xFFF;
    d->vx = (pos->x - D_80041B90) >> 1;
    d->vy = (pos->y - D_80041B92) >> 1;
    d->vz = (pos->z - D_80041B94) >> 1;
    gte_ldlvl(d);
    gte_sqr0();
    gte_stlvnl(sq);
    dist = SquareRoot0(sq->vx + sq->vy + sq->vz) * 2;
    if (dist >= range) {
        return 0;
    }
    dist = ((len - dist) << 7) / len;
    v = (dist * vol) >> 7;
    if (v < 30) {
        return 0;
    }
    if (v >= 128) {
        v = 127;
    }
    ang = (1024 - ratan2(d->vx, d->vz) - D_80041B9A) & 0xFFF;
    if (ang >= 0x800) {
        ang = 0x1000 - ang;
    }
    ang >>= 1;
    v += (v << 6) + (v << 3) + (v << 2);
    vl = (v * rsin(ang)) >> 18;
    if (vl >= 128) {
        vl = 127;
    }
    vr = (v * rcos(ang)) >> 18;
    if (vr >= 128) {
        vr = 127;
    }
    func_8004D178(id, key, vl, vr, pitch);
    return 1;
}
