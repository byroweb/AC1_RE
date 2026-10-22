#include "ac1/ac1.h"
#include "ac1/frameseq.h"
/* ovl202 0x8004E98C - start sequence seq with step: a new sequence also takes
 * its frame size from the header and the extra value; the frame starts at 0
 * (forward) or the last frame (backward).  Unchanged (seq, step) is left
 * alone. */

void func_8004E98C(FrameSeq *s, s32 seq, s32 step, s32 extra)
{
    u16 *wh;

    if (s->seq == seq) goto check;
    wh = (u16 *)(s->data + (((FrameSeqData *)s->data)->seq[seq] + 12));
    s->w = wh[0] >> 2;
    s->h = wh[1] >> 2;
    s->unk0E = extra;
set:
    s->seq = seq;
    s->step = step;
    if (step >= 0) {
        s->frame = 0;
    } else {
        s->frame = FRAMESEQ_HDR(s)->frames - 1;
    }
    s->unk55 = 0;
    s->unk54 = 0;
    return;
check:
    if (s->step != step) goto set;
}
