#include "ac1/ac1.h"

typedef struct { u8 b[7]; } Blk7;
typedef struct { s32 w[12]; } Blk48;

extern void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
extern void func_8005C2D0();
extern void func_800A9614(void);
extern void func_800AD2D8(UiObj *);
extern Blk7 D_80039D20;
extern Blk7 D_80040E38[];
extern Blk48 D_80048630;
extern Blk48 D_80048538[];
extern s32 D_801A2574;

void func_800AD198(UiObj *o)
{
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        D_80040E38[o->w.raw[5]] = D_80039D20;
        D_80048538[o->w.raw[5]] = D_80048630;
        D_801A2574 = 2;
        func_800A9614();
        func_8005BA2C(0x48, 0x50, 0x88, 0x78, 9, 0, 0);
        o->update = func_800AD2D8;
    }
}
