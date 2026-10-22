#include "ac1/ac1.h"

extern RECT D_800B80EC;
extern RECT D_800B80F4;
extern s32 D_801A2550;
void func_800506EC(s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_8006BA08(UiObj *);
void func_80068FBC(UiObj *);
void func_8006B1E8(UiObj *);

void func_8006B080(UiObj *o)
{
    if (o->w.raw[0xC4 - 0x7C] == 0) {
        MoveImage(&D_800B80EC, 0x220, 0x180);
        MoveImage(&D_800B80F4, 0x100, 0x1FE);
        func_800506EC(4);
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_800506EC(10);
        obj_find_by_id(3)->kill = 1;
        obj_find_by_id(9)->kill = 1;
        { s32 *p = &D_801A2550; *p |= 0x20; }
        func_8005C2D0(0x50, 0x32, 0x68, 0x90, 9);
        o->update = func_8006B1E8;
        return;
    }
    if (o->w.raw[0xC4 - 0x7C] == 1) {
        o->update = func_80068FBC;
    }
    func_8006BA08(o);
}
