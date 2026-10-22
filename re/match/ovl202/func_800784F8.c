#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x0A];
    s16 y;              /* +0x0A */
    u8 pad0c[0x1E];
    u16 h2a;            /* +0x2A */
    u8 pad2c[2];
    s16 h2e;            /* +0x2E */
} AcHeight;

extern u8 *D_801A5DC0;
extern s32 func_80014D78(s32 n);
extern void func_800783F8(AcHeight *e, s32 a, s32 b, s32 c, void *buf);

void func_800784F8(AcHeight *e, s32 h, s32 a, s32 n)
{
    s32 y;
    s32 top;

    y = e->y;
    top = e->h2e;
    if (y + h < top) {
        return;
    }
    if (top < y - e->h2a) {
        return;
    }
    if (y < top) {
        n -= top - y;
    }
    func_800783F8(e, a, n + func_80014D78(n >> 2), 66, D_801A5DC0 + 0x1F00);
}
