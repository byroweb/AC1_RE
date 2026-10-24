#include "ac1/ac1.h"

typedef struct {
    s16 x, y, z;        /* +0 */
    u8 pad6[0x1C];
    u16 h;              /* +0x22 */
} P;
extern s32 func_80014DAC();
extern void func_8004C318();
extern void func_8004D178();
extern s32 func_80052338();
extern void func_800529F0();
extern void func_80062860();
extern void func_80086854();
extern void func_8008A80C();
extern s32 rsin(s32);
extern s32 D_800411F4;
extern s32 D_8019F524;
extern u16 D_8019F52C;
extern P D_801A26C0;
extern s32 D_801AC81C;
extern s32 D_801CCB60;
extern s32 D_801CCB64;
extern s32 D_801CCB68;
extern s32 D_801CCB8C;

void func_801C67F8(void)
{
    s16 sp18[3];
    s32 sp20;
    s32 n;
    u32 h;
    s32 r;

    func_8008A80C();
    if (D_801CCB68 != -1) {
        sp20 = 0xFB201D88;
        {
            s32 *p = &D_800411F4;
            *p &= 0xFFFEFFFF;
        }
        {
            s32 *q = &D_8019F524;
            *q |= 0x2400;
        }
        n = 2;
        for (;;) {
            sp18[0] = D_801A26C0.x + func_80014DAC(0x7D0);
            sp18[2] = D_801A26C0.z + func_80014DAC(0x7D0);
            n--;
            r = func_80014DAC(0x3E8);
            h = D_801A26C0.h >> 1;
            sp18[1] = D_801A26C0.y - h + r;
            func_80086854(0, sp18, &sp20, h);
            if (n == 0) {
                return;
            }
        }
    } else {
        if (D_801CCB60 != 0 && func_80052338() != 0) {
            if (D_801CCB8C == 10) {
                func_800529F0(0, 0, 0x68);
                func_8004C318(0x80);
            }
            D_801CCB8C++;
            return;
        }
        if (D_801CCB64 != 0) {
            func_80062860(0, 9, 8, (rsin(D_801AC81C << 9) + 0x1400) >> 1);
            func_8004D178(0x4B, 0x6000, 0x64, 0x64, 0);
            if (D_8019F52C == 0x528) {
                func_800529F0(0, 0, 0xD);
                return;
            }
            if (D_8019F52C == 0x294) {
                func_800529F0(0, 0, 0xE);
                return;
            }
            if (D_8019F52C == 0x9A) {
                func_800529F0(0, 0, 0xF);
                return;
            }
            if (D_8019F52C == 1) {
                D_801CCB68 = 0x1E;
            }
        }
    }
}
