#include "ac1/ac1.h"

typedef struct {
    u32 a;
    u32 b;
} Pair485;

extern u32 *D_800B89E8[10];
extern s16 D_800B8A10[10];
extern u8 D_80042AA8[];
extern Pair485 D_800485B8[2];
extern s16 func_8008CFD4(s8 cat, s8 n);

void func_80083A58(void)
{
    s16 i;
    s16 j;
    u32 *mask;

    for (i = 0; i < 10; i++) {
        mask = D_800B89E8[i];
        mask[1] = 0;
        mask[0] = 0;
        for (j = 0; j < D_800B8A10[i]; j++) {
            if (j < 32) {
                if (D_80042AA8[func_8008CFD4(i, j)]) {
                    mask[0] |= 1 << j;
                }
            } else {
                if (D_80042AA8[func_8008CFD4(i, j)]) {
                    mask[1] |= 1 << (j - 32);
                }
            }
        }
    }
    D_800485B8[0] = D_800485B8[1];
    D_800485B8[1].a &= 0xFFFF;
    D_800485B8[0].a >>= 16;
}
