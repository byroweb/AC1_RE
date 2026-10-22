#include "ac1/ac1.h"

typedef struct {
    u8 c0, c1, c2, c3;
    u8 pad04[0x4C];
    s32 s50;
    s32 s54;
    s32 s58;
    s32 s5C;
} GarageEnt;

typedef struct {
    u8 hdr[0x5C];
    GarageEnt ent[1];
} GarageWork;

extern struct { GarageWork *w; } D_801F5094;

void func_80088A18(s32 i)
{
    D_801F5094.w->ent[i].s50 = 0;
    D_801F5094.w->ent[i].s54 = 0;
    D_801F5094.w->ent[i].c0 = 0x40;
    D_801F5094.w->ent[i].c1 = 0x30;
    D_801F5094.w->ent[i].c2 = 0x3E;
    D_801F5094.w->ent[i].c3 = 0;
    D_801F5094.w->ent[i].s58 = 2;
    D_801F5094.w->ent[i].s5C = 0;
}
