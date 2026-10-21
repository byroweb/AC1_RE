#include "ac1/ac1.h"
#include "ac1/vsplayer.h"

typedef struct {
    u8 unk00[0x10];
    s16 rx;                     /* +0x10 */
    s16 ry;                     /* +0x12 */
    u8 unk14[4];
    s16 vrx;                    /* +0x18 */
    s16 vry;                    /* +0x1A */
} Ovl203Turn;

/* Turn with the current player's turn buttons: accelerate by acc, else decay,
 * clamp the rate to +-max and integrate. */
void func_80078258(Ovl203Turn *o, s32 acc, s32 max) {
    s16 v;

    if (D_80039D18->pad_cur & D_80039D18->turn_l) {
        o->vry += acc;
    } else if (D_80039D18->pad_cur & D_80039D18->turn_r) {
        o->vry -= acc;
    } else {
        v = o->vry;
        if (v > 0) {
            if (v > acc) {
                o->vry = v - acc;
            } else {
                o->vry = 0;
            }
        } else {
            if (v >= -acc) {
                o->vry = 0;
            } else {
                o->vry = v + acc;
            }
        }
    }
    if (o->vry > max) {
        o->vry = max;
    }
    if (o->vry < -max) {
        o->vry = -max;
    }
    o->rx += o->vrx;
    o->ry += o->vry;
}
