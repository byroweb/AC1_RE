#include "ac1/ac1.h"
/* ovl201 0x8007D07C - draw the window frames of the variant screen and the
 * four lines of the current variant (D_800B82BC), then func_8007FED0. */

extern u8 D_80039CA2;
extern u8 D_800B8454[];
extern u8 D_800B84F0[];
extern u8 D_800B846C[];
extern char *D_800B82BC[][4];
extern void func_8005D754(s32 layer, void *frame, s32 x, s32 y, s32 kind);
extern void func_8005D80C(s32 layer, char *s, s32 x, s32 w, s32 y, s32 kind);
extern void func_8007FED0(UiObj *o);

void func_8007D07C(UiObj *o)
{
    func_8005D754(4, D_800B8454, 0, 0, 4);
    func_8005D754(3, D_800B84F0, 8, 4, 4);
    func_8005D754(2, D_800B846C, 0, 0, 4);
    func_8005D754(4, D_800B8454, 0, 32, 4);
    func_8005D754(4, D_800B8454, 0, 56, 4);
    func_8005D754(4, D_800B8454, 0, 80, 4);
    func_8005D754(4, D_800B8454, 0, 104, 4);
    func_8005D80C(3, D_800B82BC[D_80039CA2 - 1][0], 16, 8, 36, 4);
    func_8005D80C(3, D_800B82BC[D_80039CA2 - 1][1], 16, 8, 60, 4);
    func_8005D80C(3, D_800B82BC[D_80039CA2 - 1][2], 16, 8, 84, 4);
    func_8005D80C(3, D_800B82BC[D_80039CA2 - 1][3], 16, 8, 108, 4);
    func_8007FED0(o);
}
