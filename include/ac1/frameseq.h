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
    u8 unk00[0xC];
    u16 w;                      /* +0x0C frame width (x4) */
    u16 h;                      /* +0x0E frame height (x4) */
    s16 frames;                 /* +0x10 frame count */
} FrameSeqHdr;

typedef struct FrameSeq {
    u8 seq;                     /* +0x00 sequence index into data->seq[] */
    u8 hold;                    /* +0x01 extra hold time; 255 = done */
    s16 unk02;
    u16 unk04;
    u16 w;                      /* +0x06 frame width (func_8004E98C) */
    u16 h;                      /* +0x08 frame height */
    s16 frame;                  /* +0x0A current frame */
    s16 step;                   /* +0x0C frames per tick, signed */
    s16 unk0E;                  /* +0x0E set with the sequence */
    s16 timer;                  /* +0x10 ticks on this frame */
    u8 unk12[0xA];
    u8 *data;                   /* +0x1C FrameSeqData */
    u8 unk20[0x34];
    u8 unk54;                   /* +0x54 cleared with a new sequence */
    u8 unk55;
    u8 unk56[2];
} FrameSeq;                     /* at least 0x58 */
AC1_ASSERT_SIZE(FrameSeq, 0x58);

#define FRAMESEQ_HDR(s) ((FrameSeqHdr *)(((FrameSeqData *)(s)->data)->seq[(s)->seq] + (s32)(s)->data))

#endif
