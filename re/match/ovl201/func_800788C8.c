#include "ac1/ac1.h"
/* ovl201 0x800788C8 - copy the TMD model of slot src into slot dst (freeing
 * the old one, func_80051078 / func_80050FC4) and relocate the copy's object
 * table to its new address. */

typedef struct {
    u32 *vert_top;
    u32 n_vert;
    u32 *normal_top;
    u32 n_normal;
    u32 *primitive_top;
    u32 n_primitive;
    s32 scale;
} TmdObj;

typedef struct {
    u32 id;
    u32 flags;
    u32 nobj;
    TmdObj obj[1];
} Tmd;

extern u32 D_801BC478[];        /* model size in bytes, per slot */
extern Tmd *D_801F36D0[];       /* model data, per slot */
extern void func_80051078(void *p);
extern void *func_80050FC4(u32 size);

void func_800788C8(s16 dst, s16 src)
{
    s32 n;
    u32 *p;
    u32 *q;
    s32 d;
    u32 **t;
    u32 i;

    n = D_801BC478[src] >> 2;
    if (D_801F36D0[dst]) {
        func_80051078(D_801F36D0[dst]);
    }
    p = (u32 *)(D_801F36D0[dst] = func_80050FC4(D_801BC478[src]));
    q = (u32 *)D_801F36D0[src];
    while (n--) {
        *p++ = *q++;
    }
    D_801BC478[dst] = D_801BC478[src];
    d = (s32)D_801F36D0[dst] - (s32)D_801F36D0[src];
    t = &D_801F36D0[dst]->obj[0].vert_top;
    i = D_801F36D0[dst]->nobj;
    do {
        *t = (u32 *)((s32)*t + d);
        t += 2;
        *t = (u32 *)((s32)*t + d);
        t += 2;
        *t = (u32 *)((s32)*t + d);
        t += 3;
    } while (--i);
}
