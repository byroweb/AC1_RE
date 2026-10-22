#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x36];
    u16 f36;
    u8 pad38[8];
    u8 f40;
    u8 pad41[0x3B];
    s16 f7c;
} Ent;
extern s32 func_80073714();
extern void func_801C59A0();
extern void func_801C5A10();
extern void func_801C5A78();
extern s32 rand();

s32 func_801C5AE0(Ent *e, s32 msg, ...)
{
    s32 t;
    s32 v;

    t = func_80073714(e, 0x7FFFFFFF);
    switch (msg) {
    case 0x8100:
        return 0;
    case 5:
        goto done;
    case 4:
        if (e->f40 != 0xFF) {
            e->f40 = 0;
        }
        e->f36 &= 0xFFEF;
        goto c4b;
    case 1:
    case 2:
        v = rand() & 0xF;
        goto sw;
    default:
    c4b:
        e->f7c = -1;
        if (t >= 7000) {
            func_801C59A0(e, 2);
            return 0;
        }
        e->f7c = 0;
        v = rand() & 0x3F;
    }
sw:
    switch (v) {
    case 0:
    case 1:
    case 2:
        func_801C59A0(e, 2);
        break;
    case 3:
        func_801C59A0(e, 3);
        break;
    case 4:
    case 5:
        func_801C5A10(e, 2);
        break;
    case 6:
    case 7:
        func_801C5A78(e, 2);
        break;
    case 8:
        func_801C5A10(e, 3);
        break;
    case 11:
        func_801C5A78(e, 3);
        break;
    case 12:
        func_801C59A0(e, 4);
        break;
    case 13:
        func_801C59A0(e, 5);
        break;
    case 14:
        func_801C59A0(e, 1);
        break;
    }
done:
    return 0;
}
