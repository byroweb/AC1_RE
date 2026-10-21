#include "ac1/ac1.h"
#include "ac1/frameseq.h"
/* ovl202 0x8004ED5C - count one tick; once the hold time is over mark the
 * frame done (hold = 255) and return 1. */

s32 func_8004ED5C(FrameSeq *s)
{
    s32 lim;

    s->timer++;
    lim = *(u16 *)((s32)s->data + ((FrameSeqData *)s->data)->hold) + s->hold;
    if (s->timer >= lim) {
        s->hold = 255;
        return 1;
    }
    return 0;
}
