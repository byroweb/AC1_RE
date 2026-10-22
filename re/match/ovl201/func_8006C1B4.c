#include "ac1/ac1.h"

typedef struct { u8 r, g, b, a; } EmbRgb;
extern EmbRgb D_8017E5D8[];     /* editor palette */
extern EmbRgb D_800B7E08[];     /* default palette */
extern RECT D_800B811C;
extern RECT D_800B8124;
int LoadImage(RECT *, u32 *);
int DrawSync(int);

void func_8006C1B4(void)
{
    u16 out[16];
    s16 i;

    for (i = 0; i < 16; i++) {
        D_8017E5D8[i] = D_800B7E08[i];
    }
    for (i = 0; i < 16; i++) {
        if (i == 0) {
            out[i] = 0;
        } else {
            out[i] = 0;
            out[i] = (D_8017E5D8[i].b >> 3) << 10 | (D_8017E5D8[i].g >> 3) << 5 | (D_8017E5D8[i].r >> 3);
            if (D_8017E5D8[i].r == 0 && D_8017E5D8[i].g == 0 && D_8017E5D8[i].b == 0)
                out[i] |= 0x8000;
        }
    }
    LoadImage(&D_800B811C, (u32 *)out);
    LoadImage(&D_800B8124, (u32 *)out);
    DrawSync(0);
}
