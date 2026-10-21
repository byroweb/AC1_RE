#include "ac1/ac1.h"

typedef struct {
    s16 id;
    u16 sub;
    u8 a;
    u8 b;
    u8 c;
    u8 pad;
} SndSlot;

extern SndSlot D_80042A86[4];

void func_8004D558(s32 id, s32 sub, s32 a, s32 b, s32 c)
{
    SndSlot *s;
    s32 i;
    s32 n = a + b;

    for (s = D_80042A86, i = 4; i != 0; s++, i--) {
        if (s->id == id && s->sub == sub) {
            if (s->a + s->b < n) {
                goto set;
            }
            return;
        }
    }
    for (s = D_80042A86, i = 4; i != 0; i--, s++) {
        if (s->id == -1) {
            s->id = id;
            s->sub = sub;
            goto set;
        }
    }
    return;
set:
    s->a = a;
    s->b = b;
    s->c = c;
}
