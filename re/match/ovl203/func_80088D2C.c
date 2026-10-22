#include "ac1/ac1.h"

typedef struct {
    u8 pad[6];
    u8 flag6;
} WsStats;

typedef struct {
    void *unk0;
    void (*fn)(void *obj, s32 msg, s32 arg);
} HitHandler;

typedef struct {
    u8 pad[8];
    s32 (*fn8)(s32 idx);
} ScriptHost;

extern WorldStruct D_801C40F0[];
extern ScriptHost *D_8019B484;
extern u16 D_8019B490;
extern s16 D_8019B494;
extern void func_80088CF4(s32 a);
extern void func_80088CFC(s32 idx);
extern void func_8004BF6C(s32 a);
extern void func_80089310(s32 a, s32 b, s32 c);

s32 func_80088D2C(u8 op, u8 count, u16 idx, u8 x, u8 arg)
{
    s32 mode;
    s32 t;
    WorldStruct *w;
    HitHandler *h;

    switch (op) {
    case 1:
        func_80088CF4(x);
        while (count-- != 0) {
            ((WsStats *)D_801C40F0[idx].stats)->flag6 = 0;
            idx++;
        }
        break;
    case 4:
        mode = 256;
        func_80088CF4(count);
        switch (arg) {
        case 1:
            mode = 512;
            break;
        case 2:
            mode = 128;
            break;
        }
        t = idx * 22;
        func_80089310(t, mode, x);
        D_8019B494 = t;
        break;
    case 5:
        if (D_8019B490 == 0) {
            func_80088CF4(count);
            func_80088CFC(idx);
            func_8004BF6C(128);
        }
        break;
    case 6:
        if (D_8019B490 == 0) {
            func_80088CF4(count);
            func_80088CFC(idx);
            func_8004BF6C(256);
        }
        break;
    case 7:
        func_80088CF4(count);
        func_80088CFC(idx);
        break;
    case 9:
        return D_8019B484->fn8(idx);
    case 10:
        func_80088CF4(x);
        while (count-- != 0) {
            w = &D_801C40F0[idx];
            h = w->on_hit;
            if (h != 0 && h->fn != 0) {
                h->fn(w, arg + 0x8200, 0);
            }
            idx++;
        }
        break;
    }
    return 1;
}
