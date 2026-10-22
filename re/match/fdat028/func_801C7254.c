#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x18];
    SVECTOR a;          /* +0x18 */
    SVECTOR b;          /* +0x20 */
    u8 pad28[0x18];
} Func801C7254Slot;     /* 0x40 */
extern Func801C7254Slot D_801D0B68[];

void func_801C7254(SVECTOR *out, s32 i)
{
    Func801C7254Slot *s;

    s = &D_801D0B68[i];
    out->vx = (s->a.vx + s->b.vx) >> 1;
    out->vy = (s->a.vy + s->b.vy) >> 1;
    out->vz = (s->a.vz + s->b.vz) >> 1;
    out->pad = 1;
}
