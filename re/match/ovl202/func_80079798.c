#include "ac1/ac1.h"

typedef struct {
    u8 pad[0x10];
    s16 x;              /* +0x10 */
    s16 y;              /* +0x12 */
    u8 pad14[4];
    s16 dx;             /* +0x18 */
    s16 vel;            /* +0x1A */
} PadAxis;

extern s32 D_80040FA8;
extern u16 D_80041234;
extern u16 D_80041236;

void func_80079798(PadAxis *o, s32 acc, s32 max)
{
    s16 v;

    if (D_80040FA8 & D_80041234) {
        o->vel = o->vel + acc;
    } else if (D_80040FA8 & D_80041236) {
        o->vel = o->vel - acc;
    } else {
        v = o->vel;
        if (v > 0) {
            if (acc < v) {
                o->vel = v - acc;
            } else {
                o->vel = 0;
            }
        } else {
            if (v >= -acc) {
                o->vel = 0;
            } else {
                o->vel = v + acc;
            }
        }
    }
    if (max < o->vel) {
        o->vel = max;
    }
    if (o->vel < -max) {
        o->vel = -max;
    }
    o->x += o->dx;
    o->y += o->vel;
}
