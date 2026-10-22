#include "ac1/ac1.h"

extern AcEntity *D_800908EC;
extern void *D_800908F0;
extern WorldStruct *D_800908F4;
extern s32 D_800908F8;
extern s32 D_800908FC;
extern s32 D_80090900;
extern s32 D_80090904;
extern s32 D_80090908;
extern s32 D_8009090C;
extern s32 D_80090910;
extern SVECTOR D_80090914;
extern s32 D_8009091C;
extern u8 D_8019F308[];
extern s16 D_8019F508;
extern void func_8006D628(s32);

void func_8006D8F8(s32 a0, SVECTOR *pos, s32 w, s32 h, s32 arg4) {
    D_8009091C = a0;
    D_80090914 = *pos;
    D_80090908 = h;
    D_800908EC = g_ac_entities;
    D_800908F8 = 16;
    D_800908F0 = D_8019F308;
    D_800908F4 = g_world_structs;
    D_80090904 = w;
    D_80090900 = 512;
    D_800908FC = D_8019F508;
    D_8009090C = (w >> 1) * (w >> 1);
    D_80090910 = (h >> 1) * (h >> 1);
    func_8006D628(arg4);
}
