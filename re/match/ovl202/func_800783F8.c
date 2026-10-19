#include "ac1_types.h"
typedef struct {
    u8 pad[8];
    u16 f8;         /* +0x08 */
    u8 pad2[2];
    u16 fc;         /* +0x0C */
    u8 pad3[0x2E - 0x0E];
    u16 f2e;        /* +0x2E */
} Func800783F8Obj;
typedef struct {
    u16 a;
    u16 b;
    u16 c;
} Func800783F8Tmp;
extern void func_80086C9C(s32 a0, Func800783F8Tmp *t, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6, s32 a7, s32 a8, s32 a9, s32 a10, s32 a11);
void func_800783F8(Func800783F8Obj *o, s32 x, s32 y, s32 e, s32 f)
{
    Func800783F8Tmp t;
    t.a = o->f8;
    t.c = o->fc;
    t.b = o->f2e;
    func_80086C9C(0, &t, 40, x, y, x >> 2, y >> 2, -x >> 5, -y >> 3, y >> 2, e, f);
}
