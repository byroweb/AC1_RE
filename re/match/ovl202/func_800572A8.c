#include "ac1/ac1.h"

extern u8 D_800A1228[];
extern u8 D_8014C7C8[];
extern u8 D_801887C8[];
extern u8 D_801907C8[];
extern u8 *D_801A5E1C;
extern s8 D_801A5E36;
extern s8 D_801A5E38;
extern u8 *D_801A5E90;
extern u8 *D_801A5E94;
extern u8 *D_801A5E9C;
extern s8 D_801A5EB6;
extern s8 D_801A5EB8;
extern u8 *D_801A5F10;
extern u8 *D_801A5F14;
extern u8 *D_801A5F20;
void func_8001604C(void *, s32);

void func_800572A8(void)
{
    D_801A5E90 = D_801A5F20 = D_8014C7C8;
    D_801A5E94 = D_801A5E90 + 0x1C000;
    D_801A5F10 = D_801A5E90 + 0x1E000;
    D_801A5F14 = D_801A5F10 + 0x1C000;
    D_801A5E1C = D_801887C8;
    D_801A5E9C = D_801907C8;
    D_801A5EB6 = 1;
    D_801A5E36 = 1;
    D_801A5EB8 = 1;
    D_801A5E38 = 1;
    func_8001604C(D_800A1228, 0x10000);
}
