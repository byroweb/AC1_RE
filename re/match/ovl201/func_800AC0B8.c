#include "ac1/ac1.h"

typedef struct { u8 r, g, b; } Rgb3;
typedef struct { u8 b[160]; } Blk160;
typedef struct { u8 b[36]; } Blk36;

extern void func_8005C2D0();
extern void func_800AE184(void);
extern void func_800A9AFC(UiObj *);
extern Blk160 D_80031A94;
extern Blk160 D_80040E60[];
extern Rgb3 D_80031A64[4][4];
extern Rgb3 D_8004AC60[][4][4];
extern u8 D_80031A58[];
extern u8 D_80040830[][12];
extern u16 D_80039CA0;
extern u16 D_80039D1C[];
extern void *D_801F36D0[];
extern s32 D_801BC478[];
extern void *D_80041A10[][30];
extern u16 D_80041858[][30];
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_80039D28[];
extern u8 D_80039D12[];
extern Blk36 D_80031BD4;
extern Blk36 D_800419B8[];
extern s8 D_801A2570;
extern u32 D_801A2550;

void func_800AC0B8(UiObj *o)
{
    u8 which;
    s16 i;
    s16 j;

    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        which = o->w.raw[0] == 2;
        if (which == 1) {
            func_800AE184();
        }
        D_80040E60[which] = D_80031A94;
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                D_8004AC60[which][i][j] = D_80031A64[i][j];
            }
        }
        for (i = 0; i < 12; i++) {
            D_80040830[which][i] = D_80031A58[i];
        }
        D_80039D1C[which] = D_80039CA0;
        for (i = 3; i < 15; i++) {
            D_80041A10[which][i] = D_801F36D0[i];
            D_80041858[which][i] = D_801BC478[i];
        }
        D_80039D28[which] = D_80039CA2;
        D_80039D12[which] = D_80039CA3;
        D_800419B8[which] = D_80031BD4;
        ui_pool_set_light(4, 21);
        D_801A2570 = 0;
        D_801A2550 |= 0x200020;
        /* plain (not in-struct) store: keeps it after the latch store */
        *(void (**)(UiObj *))((u8 *)o + 0x74) = func_800A9AFC;
    }
}
