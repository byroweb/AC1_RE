#include "ac1/ac1.h"

typedef struct {
    u8 pad00[4];
    void (*fn)();               /* +0x04 */
} HitHandler;

extern AcEntity D_801AB748[2];
extern u8 D_801A4C68[];
extern WorldStruct D_801D4488[];
extern s32 D_801EC48C;
extern s16 D_800411EE;
extern s16 D_8004158A;
extern void func_80073EC8(void *t, u16 *a2, s32 a3, s32 a4);

/* Route a hit on t by src: the two ACs, the projectile pool, or a world struct. */
void func_80082018(u32 src, u8 *t, u16 *a2, s32 a3, s32 a4) {
    HitHandler *h;

    if ((u32)t >= (u32)D_801AB748 && (u32)t <= (u32)&D_801AB748[1]) {
        if (t != (u8 *)&D_801AB748[0] && (src & 0x7FFFFFFF) == ((u32)&D_801AB748[0] & 0x7FFFFFFF)) {
            D_800411EE = 40;
        }
        if (t != (u8 *)&D_801AB748[1] && (src & 0x7FFFFFFF) == ((u32)&D_801AB748[1] & 0x7FFFFFFF)) {
            D_8004158A = 40;
        }
        func_80073EC8(t, a2, a3, a4);
    } else if ((u32)t >= (u32)D_801A4C68 && (u32)t <= (u32)(D_801A4C68 + 0x67CC)) {
        h = *(HitHandler **)(t + 0x14);
        if (h != 0 && h->fn != 0) {
            h->fn(t, a2, a3, a4, a2[1]);
        }
    } else if ((u32)t >= (u32)D_801D4488 && (u32)t <= (u32)&D_801D4488[D_801EC48C - 1]) {
        h = ((WorldStruct *)t)->on_hit;
        if (h != 0 && h->fn != 0) {
            h->fn(t, 0x8000, 0, a2, a3, a4);
        }
    }
}
