#include "ac1/ac1.h"

extern void func_801C5B1C(void);

typedef struct WS {
    s16 pad0[3];
    s16 f6;
    u8 pad8[0x35];
    u8 f3d;
    u8 pad3e[2];
} WS;

void func_801C6540(s32 unused, s32 *score)
{
    s32 i;
    WS *w;
    u8 k;

    i = g_world_struct_count;
    w = (WS *)g_world_structs;
    while (--i != -1) {
        if (w->f6 == -1) {
            k = w->f3d;
            if (k != 0xFF && ((void **)g_objective_obj)[k + 24] == (void *)func_801C5B1C) {
                *score -= 12000;
                return;
            }
        }
        w++;
    }
}
