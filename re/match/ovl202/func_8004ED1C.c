#include "ac1/ac1.h"
#include "ac1/frameseq.h"
/* ovl202 0x8004ED1C - the frame a sequence runs towards: the last one when
 * stepping forward, 0 otherwise. */

s32 func_8004ED1C(FrameSeq *s)
{
    if (s->step > 0) {
        return FRAMESEQ_HDR(s)->frames - 1;
    }
    return 0;
}
