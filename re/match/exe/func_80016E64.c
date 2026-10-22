#include "ac1/ac1.h"

register u8 *gp_base asm("$28");

extern u8 D_80040FA0[];
extern void func_80016E14(void);

void func_80016E64(void)
{
    *(u8 * volatile *)(gp_base + 0xBC) = D_80040FA0;
    func_80016E14();
    *(u8 * volatile *)(gp_base + 0xBC) = D_80040FA0 + 0x39C;
    func_80016E14();
}
