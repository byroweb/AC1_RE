#include "ac1/ac1.h"
#include "ac1/frameseq.h"
/* ovl202 0x8004ECA8 - step the frame, stopping at the end it runs towards. */

void func_8004ECA8(FrameSeq *s)
{
    s16 step = s->step;

    if (step > 0) {
        if (s->frame < FRAMESEQ_HDR(s)->frames - 1) {
            s->frame += step;
        }
    } else {
        if (s->frame > 0) {
            s->frame += step;
        }
    }
}
