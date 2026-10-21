#include "ac1/ac1.h"

typedef struct {
    u8 pad00[0x41];
    u8 state;           /* +0x41 */
    u8 pad42[6];
    void *target;       /* +0x48 */
    u8 pad4c[0xA0];
    u8 mode;            /* +0xEC */
    u8 padED[3];
    void *tgt;          /* +0xF0 */
    SVECTOR goal;       /* +0xF4 */
} AcMoveCtx;

void func_80075ACC(AcMoveCtx *e, u8 state, SVECTOR *goal)
{
    e->state = state;
    e->mode = 5;
    e->tgt = e->target;
    e->goal = *goal;
}
