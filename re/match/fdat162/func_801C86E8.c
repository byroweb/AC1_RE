#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x16];
    u16 a;              /* +0x16 */
    u16 b;              /* +0x18 */
    u8 pad1a[8];
    u8 c[1];            /* +0x22 */
} Obj;
extern s32 func_80012794(void *, u16, u16, s32, void *, u16, u16);
extern u8 D_801AB750[];
extern u16 D_801AB770;
extern u16 D_801AB772;
extern u16 D_801AB8E0;
extern u16 D_801AB8E2;

s32 func_801C86E8(Obj *o)
{
    if (func_80012794(o->c, o->a, o->b, 0, D_801AB750, D_801AB770, D_801AB772) == 0) {
        return func_80012794(o->c, o->a, o->b, 0, D_801AB750 + 0x170, D_801AB8E0, D_801AB8E2) != 0;
    }
    return 1;
}
