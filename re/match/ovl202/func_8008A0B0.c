#include "ac1/ac1.h"

typedef struct {
    s32 pad;
    void (*fn)(WorldStruct *, s32, s32);
} WorldHandler;

typedef struct {
    u8 unk00[8];
    s32 (*fn)(s32);             /* +0x08 */
} ObjectiveObj;

extern void func_8008A048(s32);
extern void func_8008A080(s32);
extern void func_8008C2FC(s32);
extern void func_8008A778(s32, s32, s32);
extern void func_8004C318(s32);
extern s32 func_80052A2C(s32);

s32 func_8008A0B0(u8 op, u8 count, u16 idx, u8 arg3, u8 arg4) {
    s32 mode;
    s32 t;
    WorldStruct *w;
    WorldHandler *h;

    switch (op) {
    case 1:
        func_8008A048(arg3);
        while (count--) {
            ((u8 *)g_world_structs[idx].stats)[6] = 0;
            idx++;
        }
        break;
    case 2:
        func_8008C2FC(count);
        break;
    case 3:
        func_8008A048(count);
        break;
    case 4:
        mode = 0x100;
        func_8008A048(count);
        switch (arg4) {
        case 1:
            mode = 0x200;
            break;
        case 2:
            mode = 0x80;
            break;
        }
        t = idx * 22;
        func_8008A778(t, mode, arg3);
        g_mission_timer = t;
        break;
    case 5:
        if (g_mission_end_timer == 0) {
            func_8008A048(count);
            func_8008A080(idx);
            func_8004C318(0x80);
        }
        break;
    case 6:
        if (g_mission_end_timer == 0) {
            func_8008A048(count);
            func_8008A080(idx);
            func_8004C318(0x100);
        }
        break;
    case 7:
        func_8008A048(count);
        func_8008A080(idx);
        break;
    case 8:
        if (func_80052A2C(99) == 1) {
            func_8004C318(0x100);
        }
        break;
    case 9:
        return ((ObjectiveObj *)g_objective_obj)->fn(idx);
    case 10:
        func_8008A048(arg3);
        while (count--) {
            w = &g_world_structs[idx];
            h = w->on_hit;
            if (h != 0 && h->fn != 0) {
                h->fn(w, arg4 + 0x8200, 0);
            }
            idx++;
        }
        break;
    }
    return 1;
}
