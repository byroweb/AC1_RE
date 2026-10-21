#include "ac1/ac1.h"
#include "ac1/frameseq.h"
/* ovl202 0x8004EBF8 - has the sequence reached its end frame (in the
 * direction it steps)? */

s32 func_8004EBF8(FrameSeq *s)
{
    if (s->step > 0) {
        return s->frame >= FRAMESEQ_HDR(s)->frames - 1;
    } else {
        return s->frame <= 0;
    }
}
