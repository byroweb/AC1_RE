#include "ac1/ac1.h"

register u8 *gp_base asm("$28");

typedef struct {
    u8 pad[0x254];
    u32 lo : 16;
    u32 a : 1;
    u32 b : 1;
    u32 c : 1;
    u32 hi : 13;
} Rec39C;

extern void func_8001555C(void *p, s32 v, s32 n);

void func_80016E14(void)
{
    Rec39C *p;

    func_8001555C(*(Rec39C * const *)(gp_base + 0xBC), 0, 0x12A);
    p = *(Rec39C **)(gp_base + 0xBC);
    p->a = 0;
    p->b = 0;
    p->c = 1;
}
