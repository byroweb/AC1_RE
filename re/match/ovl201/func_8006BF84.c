#include "ac1/ac1.h"

typedef struct { u8 r, g, b, a; } EmbRgb;
extern EmbRgb D_8017E5D8[];
extern EmbRgb D_800B7E48;
extern RECT D_800B8104;         /* canvas image rect */
extern RECT D_800B810C;
extern RECT D_800B8114;         /* canvas CLUT rect */
int StoreImage(RECT *, u32 *);
int LoadImage(RECT *, u32 *);
int DrawSync(int);
void func_8006F55C(void);

void func_8006BF84(u32 *pixels)
{
    u16 clut[16];
    u16 out[16];
    s16 i;

    StoreImage(&D_800B810C, pixels);
    StoreImage(&D_800B8114, (u32 *)clut);
    DrawSync(0);
    D_8017E5D8[0].r = D_8017E5D8[0].g = D_8017E5D8[0].b = 0;
    D_8017E5D8[0].a = 0x60;
    D_800B7E48.r = D_800B7E48.g = D_800B7E48.b = 0;
    for (i = 1; i < 16; i++) {
        D_8017E5D8[i].r = (clut[i] & 0x1F) << 3;
        D_8017E5D8[i].g = (clut[i] & 0x3E0) >> 2;
        D_8017E5D8[i].b = (clut[i] & 0x7C00) >> 7;
        D_8017E5D8[i].a = 0x60;
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
    LoadImage(&D_800B8114, (u32 *)out);
    DrawSync(0);
    LoadImage(&D_800B8104, pixels);
    DrawSync(0);
    func_8006F55C();
}
