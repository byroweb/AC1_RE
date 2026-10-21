#include "ac1/ac1.h"
#include "ac1/frameseq.h"
/* ovl202 0x8004EC4C - step the frame and wrap it into 0..frames-1. */

void func_8004EC4C(FrameSeq *s)
{
    s32 frames = FRAMESEQ_HDR(s)->frames;

    s->frame += s->step;
    if (s->frame >= frames) {
        s->frame -= frames;
    } else if (s->frame < 0) {
        s->frame += frames;
    }
}
