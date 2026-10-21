#include "ac1/ac1.h"

typedef struct {
    u8 kind;
    u8 pad;
    u16 val;
} ScaleItem;

void func_80075418(AcEntity *e, ScaleItem *it, s32 dmg, s32 def, s32 k)
{
    s32 cur;

    cur = e->ai_mode;
    if (cur == 255) {
        cur = def;
    }
    if (it->kind & 8) {
        dmg >>= 3;
    }
    if (dmg != 0) {
        if (k == -1) {
            if (def < 256) {
                cur -= (((dmg * (256 - def)) >> 8) + 7) >> 3;
            }
        } else {
            cur -= (dmg << 4) / ((e->ap_max * k) >> 8);
        }
    }
    if (cur <= 0) {
        e->ai_mode = 0;
    } else if (cur >= 254) {
        e->ai_mode = 254;
    } else {
        e->ai_mode = cur;
    }
}
