#include "ac1/ac1.h"

typedef struct {
    u8 pad[7];
    u8 a;
    u8 b;
    u8 c;
    u8 pad0a[0x32];
} AimEntry;

typedef struct {
    u8 sel;             /* 0x80040FB8 */
    u8 pad[0x2C1];
    AimEntry tab[1];    /* 0x8004127A */
} AimCfg;

extern AimCfg D_80040FB8;
extern u16 D_800411B8;
extern u16 D_800411BA;
extern u16 D_800411BC;

void func_8007DF08(void)
{
    AimEntry *t;

    t = &D_80040FB8.tab[D_80040FB8.sel];
    D_800411B8 = ((t->a * 5u) >> 5) + 1;
    D_800411BA = ((t->b * 15) >> 5) + 1;
    D_800411BC = ((t->c * 75) >> 5) + 1;
}
