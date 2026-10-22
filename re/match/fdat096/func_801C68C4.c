#include "ac1/ac1.h"

extern u8 D_80041BBB;
extern u8 D_801AC8A0;
extern u8 D_801AC8A1;
extern u8 D_801AC8A2;
extern s8 D_801AC8A3;
extern u16 D_801D0B52;
extern void SsSetMVol();

void func_801C68C4(void)
{
    s32 v, w;
    u8 *m;

    D_801AC8A3 = 2;
    if (0xA1 - D_801D0B52 >= 0x87) {
        v = D_801AC8A2;
        v += 0xC;
        if (v >= 0x100) {
            v = 0xFF;
        }
        D_801AC8A0 = D_801AC8A1 = D_801AC8A2 = v;
    }
    if (D_801D0B52 < 0x15) {
        m = &D_80041BBB;
        *m -= 6;
        if (*m >= 0x81) {
            *m = 0;
        }
        SsSetMVol(*m, *m);
    }
}
