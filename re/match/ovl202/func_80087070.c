#include "ac1_types.h"
typedef struct {
    s16 v[7];
} Func80087070Pos;       /* 14 bytes, 2-aligned (lwl/lwr + lh) */
typedef struct {
    s16 v[4];
} Func80087070Vec;       /* 8 bytes, 2-aligned */
typedef struct {
    u8 pad[0x10];
    u8 state;               /* +0x10 */
    u8 pad2[3];
    void (*fn)(void);       /* +0x14 */
    Func80087070Vec v18;    /* +0x18 */
    Func80087070Vec v20;    /* +0x20 */
    u8 pad3[0xB8 - 0x28];
    Func80087070Pos pos;    /* +0xB8 */
    Func80087070Vec vC6;    /* +0xC6 */
    s16 hCE;                /* +0xCE */
} Func80087070Obj;
extern Func80087070Obj D_8008EC88;
extern Func80087070Obj *func_80089ECC(s32);
extern void func_80085764(Func80087070Obj *, s32, s32, s32);
extern void func_80085510(void);
Func80087070Obj *func_80087070(s32 a, s32 b, Func80087070Vec *vc6, Func80087070Pos *pos,
                               Func80087070Vec *v18, Func80087070Vec *v20, s32 hce, s32 h)
{
    Func80087070Obj *o = func_80089ECC(0);
    if (o == &D_8008EC88) {
        return o;
    }
    func_80085764(o, a, b, h);
    o->state = 3;
    o->pos = *pos;
    o->vC6 = *vc6;
    o->hCE = hce;
    o->v18 = *v18;
    o->v20 = *v20;
    o->fn = func_80085510;
    return o;
}
