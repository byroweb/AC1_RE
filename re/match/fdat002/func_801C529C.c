#include "ac1/ac1.h"

extern void func_8004E98C(void *, s32, s32, s32);
extern void func_80076548(void *, s32, void *);
extern void func_80076578(void *, s32, s32, void *);
extern void func_80076658(void *, s32, s32, s32, s32, s32, void *);
extern void func_8007790C(void *, s32, s32, s32, void *, s32, void *);
extern void func_80078598(void *);
extern s32 func_801C4FD8(void *);
extern void func_801C5128(void *, s32, s32, s32, s32);
extern u8 D_80076014[];
extern u8 D_800764C4[];
extern void func_801C4F30();
extern void func_801C4F84();

void func_801C529C(u8 *o, s32 arg1)
{
    s32 s;

    s = arg1;
    if (*(u16 *)(o + 0x36) & 0x10) {
        s = o[0x3C];
    }
    func_80078598(o);
    switch (s) {
    case 1:
        func_801C5128(o, 2, 1, 3, 5);
        break;
    case 2:
        func_801C5128(o, 4, 3, 1, 5);
        break;
    case 3:
        func_801C5128(o, 6, 5, 1, 3);
        break;
    case 6:
        func_801C5128(o, 15, 14, 16, 18);
        break;
    case 7:
        func_801C5128(o, 17, 16, 14, 18);
        break;
    case 8:
        func_801C5128(o, 19, 18, 14, 16);
        break;
    case 4:
        func_801C5128(o, 9, 8, 255, 255);
        break;
    case 5:
        func_801C5128(o, 12, 11, 255, 255);
        break;
    case 0:
        if (func_801C4FD8(o) != 0) {
            func_80076578(o, 0, 1, D_80076014);
            *(s16 *)(o + 0x72) = 0x100;
        }
        break;
    case 9:
        if (o[0x64] == 21) {
            func_80076548(o, 2, func_801C4F30);
            func_8004E98C(o + 0x64, 21, 0, 0);
            *(s16 *)(o + 0xCE) = 8;
            o[0x66] = 2;
        } else if (func_801C4FD8(o) != 0) {
            func_80076658(o, 21, 0, 2, 8, 32, func_801C4F30);
        }
        break;
    case 10:
        if (o[0x64] == 21) {
            func_80076548(o, 3, func_801C4F84);
            func_8004E98C(o + 0x64, 21, 0, 0);
            *(s16 *)(o + 0xCE) = -8;
            o[0x66] = 2;
        } else if (func_801C4FD8(o) != 0) {
            func_80076658(o, 21, 0, 3, -8, -32, func_801C4F84);
        }
        break;
    case 11:
        func_8007790C(o, o[0x64], 1, 1024, o + 0xCC, -1, D_800764C4);
        break;
    }
    if (!(*(u16 *)(o + 0x36) & 0x10)) {
        o[0x3C] = s;
    }
}
