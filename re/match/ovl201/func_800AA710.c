#include "ac1/ac1.h"

typedef struct { u8 b[7]; } Blk7;

extern void func_8009C964(s32);
extern void func_800AAF20(void);
extern void func_800506EC(s32);
extern void func_8005C2D0(s32, s32, s32, s32, s32);
extern void func_800A9AFC(UiObj *);
extern void func_800AAB98(UiObj *);
extern u8 D_80031A58[];
extern u8 D_80040830[];
extern u8 D_8004083C[];
extern Blk7 D_80039D20;
extern Blk7 D_80040E38;
extern Blk7 D_80040E3F;
extern u16 D_80039CA0;
extern u16 D_80039D1C;
extern u16 D_80039D1E;
extern u8 D_80039CA2;
extern u8 D_80039CA3;
extern u8 D_80039D28;
extern u8 D_80039D12;
extern RECT D_8004D454;
extern RECT D_8004D45C;

void func_800AA710(UiObj *o)
{
    s16 i;

    if (o->w.raw[3] == 0) {
        o->update = func_800A9AFC;
        return;
    }
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_80040830[i];
    }
    D_80039D20 = D_80040E38;
    D_80039CA0 = D_80039D1C;
    func_8009C964(0);
    if (o->w.raw[5] != 0) {
        func_800AAF20();
        o->w.raw[5] = 0;
    }
    if ((D_80040830[6] != 0xFF && D_80040830[6] >= 31) || (D_80040830[7] != 0xFF && D_80040830[7] >= 31)) {
        RECT a = D_8004D454;
        RECT b = D_8004D45C;
        MoveImage(&a, 768, 448);
        MoveImage(&b, 0, 493);
        MoveImage(&b, 0, 494);
        DrawSync(0);
    }
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_8004083C[i];
    }
    D_80039D20 = D_80040E3F;
    D_80039CA0 = D_80039D1E;
    func_8009C964(1);
    if (o->w.raw[5] == 0) {
        func_800AAF20();
        o->w.raw[5] = 1;
    }
    if ((D_8004083C[6] != 0xFF && D_8004083C[6] >= 31) || (D_8004083C[7] != 0xFF && D_8004083C[7] >= 31)) {
        RECT a = D_8004D454;
        RECT b = D_8004D45C;
        MoveImage(&a, 768, 448);
        MoveImage(&b, 0, 493);
        MoveImage(&b, 0, 494);
        DrawSync(0);
    }
    for (i = 0; i < 12; i++) {
        D_80031A58[i] = D_80040830[i];
    }
    D_80039D20 = D_80040E38;
    D_80039CA2 = D_80039D28;
    D_80039CA3 = D_80039D12;
    D_80039CA0 = D_80039D1C;
    func_800506EC(4);
    obj_find_by_id(3)->kill = 1;
    func_800506EC(6);
    obj_find_by_id(5)->kill = 1;
    func_800506EC(8);
    obj_find_by_id(7)->kill = 1;
    func_800506EC(2);
    obj_find_by_id(1)->kill = 1;
    func_8005C2D0(0x10, 0x28, 0x88, 0x60, 3);
    func_8005C2D0(0xA0, 0x10, 0x98, 0x38, 5);
    func_8005C2D0(0xB8, 0x60, 0x6C, 0x68, 7);
    o->update = func_800AAB98;
}
