/* include/ac1/frameseq.h - overlay 202 frame sequencer (ovl202.t12,
 * 0x8004EC4C..0x8004F1A0, twins in 203/204): a cursor that steps through
 * the frames of one sequence in a data block.
 *
 * Evidence: func_8004ED1C (last frame), func_8004EBF8 (at the end?),
 * func_8004ECA8 (clamped step), func_8004EC4C (wrapping step),
 * func_8004ED5C (hold timer).
 */
#ifndef AC1_FRAMESEQ_H
#define AC1_FRAMESEQ_H

#include "types.h"

typedef struct FrameSeqData {
    u8 unk00[0x8];
    s32 hold;                   /* +0x08 byte offset of the u16 hold-time base */
    u16 seq[1];                 /* +0x0C byte offset of each sequence's header */
} FrameSeqData;

typedef struct FrameSeqHdr {
    u8 unk00[0x10];
    s16 frames;                 /* +0x10 frame count */
} FrameSeqHdr;

typedef struct FrameSeq {
    u8 seq;                     /* +0x00 sequence index into data->seq[] */
    u8 hold;                    /* +0x01 extra hold time; 255 = done */
    s16 unk02;
    u16 unk04;
    u16 unk06;
    u8 unk08[0x2];
    s16 frame;                  /* +0x0A current frame */
    s16 step;                   /* +0x0C frames per tick, signed */
    u8 unk0E[0x2];
    s16 timer;                  /* +0x10 ticks on this frame */
    u8 unk12[0xA];
    u8 *data;                   /* +0x1C FrameSeqData */
} FrameSeq;
AC1_ASSERT_SIZE(FrameSeq, 0x20);

#define FRAMESEQ_HDR(s) ((FrameSeqHdr *)(((FrameSeqData *)(s)->data)->seq[(s)->seq] + (s32)(s)->data))

#endif
