#include "ac1/ac1.h"
/* ovl202 0x8006F3E0 - reflect velocity v off surface normal n: out = v - 2 (v.n) n.
 * The retail gte_lddp is handed &o->d (an address), not the value. */
#define gte_lddp(r0) __asm__ volatile ( \
    "mtc2 %0, $8" : : "r"(r0))
#define gte_ldsv(r0) __asm__ volatile ( \
    "lhu $12, 0(%0)\n\t" \
    "lhu $13, 2(%0)\n\t" \
    "lhu $14, 4(%0)\n\t" \
    "mtc2 $12, $9\n\t" \
    "mtc2 $13, $10\n\t" \
    "mtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_gpf12() __asm__ volatile ( \
    "nop\n\t" \
    "nop\n\t" \
    ".word 0x4B98003D")
#define gte_stsv(r0) __asm__ volatile ( \
    "mfc2 $12, $9\n\t" \
    "mfc2 $13, $10\n\t" \
    "mfc2 $14, $11\n\t" \
    "sh $12, 0(%0)\n\t" \
    "sh $13, 2(%0)\n\t" \
    "sh $14, 4(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")
typedef struct Bouncer {
    u8 unk00[0x50];
    SVECTOR out;                /* +0x50 */
    SVECTOR v;                  /* +0x58 */
    s32 hit;                    /* +0x60 */
    u8 unk64[0x3A];
    SVECTOR n;                  /* +0x9E surface normal (4.12) */
    u8 unkA6[0x2A];
    s32 d;                      /* +0xD0 */
} Bouncer;
void func_8006F04C(MATRIX *m, SVECTOR *v, s32 *out);


void func_8006F3E0(Bouncer *o)
{
    if (o->hit == 0) {
        o->out.vx = 0;
        o->out.vy = 0;
        o->out.vz = 0;
        return;
    }
    func_8006F04C((MATRIX *)&o->v, &o->n, &o->d);
    o->d >>= 12;
    gte_lddp(&o->d);
    gte_ldsv(&o->n);
    gte_gpf12();
    gte_stsv(&o->out);
    o->out.vx = o->v.vx - (o->out.vx << 1);
    o->out.vy = o->v.vy - (o->out.vy << 1);
    o->out.vz = o->v.vz - (o->out.vz << 1);
}
