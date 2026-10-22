#include "ac1/ac1.h"

/* PsyQ inline_c.h GTE macros */
#define gte_lddp(r0) __asm__ volatile ("mtc2 %0, $8" : : "r"(r0))
#define gte_ldcv(r0) __asm__ volatile ("lbu $12, 0(%0)\n\tlbu $13, 1(%0)\n\tlbu $14, 2(%0)\n\tmtc2 $12, $9\n\tmtc2 $13, $10\n\tmtc2 $14, $11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_gpf12() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B98003D")
#define gte_stcv(r0) __asm__ volatile ("mfc2 $12, $9\n\tmfc2 $13, $10\n\tmfc2 $14, $11\n\tsb $12, 0(%0)\n\tsb $13, 1(%0)\n\tsb $14, 2(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")

typedef struct {
    u8 c[4];
} Col4;

extern Col4 D_80090928[4][16];

/* rebuild the four 16-entry fade ramps whose bit in the mask nibble is set:
 * entries 1..15 are the base colour scaled by a falling factor */
void func_8005D0F0(u16 *src)
{
    s32 mask;
    u8 *c;
    Col4 *d;
    s32 row;
    s32 i;
    s32 k;

    mask = *src;
    c = (u8 *)(src + 1);
    d = D_80090928[0];
    for (row = 4; row != 0; row--) {
        if (mask & 0xF) {
            d++;
            k = 0;
            for (i = 15; i != 0; i--) {
                gte_lddp(4096 - k);
                gte_ldcv(c);
                gte_gpf12();
                gte_stcv(d);
                d++;
                k += (k >> 1) + 6;
            }
        } else {
            d += 16;
        }
        mask >>= 4;
        c += 4;
    }
}
