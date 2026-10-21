#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x10];
    u8 ent;             /* +0x10 */
} Func801C72A8Ctl;
typedef struct {
    u8 pad0[0x2C];
    Func801C72A8Ctl *ctl; /* +0x2C */
} Func801C72A8Obj;
extern void func_80075350(AcEntity *, s32, s32, s32);

s32 func_801C72A8(Func801C72A8Obj *o, s32 msg, s32 arg2, ...)
{
    s32 *ap;

    ap = &arg2 + 1;
    if (msg == 0x8000) {
        func_80075350(&g_ac_entities[o->ctl->ent], ap[0], ap[1], ap[2]);
    }
    return 0;
}
