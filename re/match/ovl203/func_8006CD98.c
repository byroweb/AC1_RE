#include "ac1/ac1.h"

typedef struct {
    u8 b[8];
} Bytes8;

extern void func_8006CAC8(s32);
extern void *D_8008C85C;
extern void *D_8008C860;
extern void *D_8008C864;
extern s32 D_8008C868;
extern s32 D_8008C86C;
extern s32 D_8008C870;
extern s32 D_8008C874;
extern s32 D_8008C878;
extern s32 D_8008C87C;
extern s32 D_8008C880;
extern Bytes8 D_8008C884;
extern s32 D_8008C88C;
extern u8 D_8019B270[];
extern s16 D_8019B470;
extern u8 D_8019B550[];
extern u8 D_801C40F0[];

void func_8006CD98(s32 arg0, Bytes8 *arg1, s32 w, s32 h, s32 arg4)
{
    s32 hw;
    s32 hh;

    hw = w >> 1;
    D_8008C88C = arg0;
    D_8008C884 = *arg1;
    D_8008C878 = h;
    hh = h >> 1;
    D_8008C85C = D_8019B550;
    D_8008C868 = 2;
    D_8008C860 = D_8019B270;
    D_8008C864 = D_801C40F0;
    D_8008C874 = w;
    D_8008C870 = 0x200;
    D_8008C86C = D_8019B470;
    D_8008C87C = hw * hw;
    D_8008C880 = hh * hh;
    func_8006CAC8(arg4);
}
