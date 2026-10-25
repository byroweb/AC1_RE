#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x40];
} World;
typedef struct {
    u8 pad0[0x10];
    u8 mv[0x18];        /* +0x10 */
    u16 *ids;           /* +0x28 */
    u8 pad2c[0xC];
    u8 *def;            /* +0x38 */
} Obj;
extern World D_801D0B68[];
extern void func_801C5CE0(void *, void *, u8);

s32 func_801C5DE4(Obj *o, s32 msg, s32 **arg, ...)
{
    if (msg == 0x8100 && ((*arg)[3] & 7) == 2) {
        func_801C5CE0(&D_801D0B68[*o->ids], o->mv, *o->def);
        return 1;
    }
    return 0;
}
