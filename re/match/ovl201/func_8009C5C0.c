#include "ac1/ac1.h"
extern u8 D_80039D21;
extern s32 D_80039CA4;
extern s32 D_80049E70;
extern s8 D_80041B10;
extern s32 D_80041B3C;
extern u8 D_80041B5D;
extern u8 D_80041B5E;
extern void func_8009C6DC(void);
extern void func_800B1F20(void);
extern void func_80086EC8(void);
extern void func_8009646C(void);
extern void func_800866FC(void);
extern void func_80096230(void);
extern void func_800866A8(void);
extern void func_800A448C(s16 idx, u8 mode);

s8 func_8009C5C0(s8 id)
{
    if (D_80039D21 != 0) {
        return id;
    }
    {
        if (D_80039CA4 <= -50000) {
            func_8009C6DC();
            D_80039CA4 = 0;
            D_80049E70 = 0;
            func_800B1F20();
            func_80086EC8();
            func_8009646C();
            func_800866FC();
            func_80096230();
            D_80041B10 = -1;
            D_80041B3C = 0;
            func_800866A8();
            return -1;
        }
        if (id == 22 && (D_80041B5E & 2)) {
            func_800A448C(4, 0);
            return -1;
        }
        if (id == 21 && (D_80041B5D & 2)) {
            func_800A448C(3, 0);
            return -1;
        }
    }
    return id;
}
