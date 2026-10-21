#include "ac1/ac1.h"

typedef struct {
    s16 a;
    s16 b;
    s16 c;
    s16 d;
} StrmEnt;

typedef struct {
    u8 pad[0x18];
    StrmEnt ent[2];     /* +0x18 */
    s32 sel;            /* +0x28 */
    s16 h2c;            /* +0x2C */
    s16 h2e;            /* +0x2E */
    s16 h30;            /* +0x30 */
    s16 pad32;
    s32 done;           /* +0x34 */
} StrmSync;

void func_800A4DA0(StrmSync *s)
{
    volatile s32 t;

    t = 0x800000;
    if (s->done == 0) {
        do {
            if (--t == 0) {
                s->done = 1;
                s->sel = s->sel == 0;
                s->h2c = s->ent[s->sel].a;
                s->h2e = s->ent[s->sel].b;
            }
        } while (s->done == 0);
    }
    s->done = 0;
}
