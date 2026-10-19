#include "ac1_types.h"
extern u8 D_80048661;
extern u8 D_801BC470;
extern u8 D_801BC471;
extern u8 D_801BC472;
extern u8 D_801BC473;
extern u8 D_801BC474;
extern s16 D_801BC454;
extern s16 D_801BC456;
extern s32 D_801BC458;
extern s32 D_801BC45C;
extern s32 D_801BC460;
extern s32 D_801BC464;
extern s32 D_801BC468;
extern s32 D_801BC46C;
extern void func_8001EE10(s32, s32);
extern void func_80022700(s32);
extern void func_80022668(s32, s32);
extern void func_8001EE60(s32, s32);
extern void func_800228E4(void);
void func_80050C18(void)
{
    func_8001EE10(0, 0);
    D_801BC470 = 0;
    D_801BC45C = 0;
    D_801BC458 = 0;
    D_801BC464 = 0;
    D_801BC460 = 0;
    D_801BC46C = 0;
    D_801BC468 = 0;
    D_801BC456 = 0;
    D_801BC454 = 0;
    D_801BC471 = 0;
    D_801BC473 = 0;
    D_801BC472 = 0;
    D_801BC474 = 0;
    func_8001EE10(D_80048661 * 2, D_80048661 * 2);
    func_80022700(4);
    func_80022668(40, 40);
    func_8001EE60(40, 40);
    func_800228E4();
}
