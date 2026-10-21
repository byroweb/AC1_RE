/* include/ac1/sndbank.h - the overlay 201 music double buffer at 0x801BC450
 * (TU ovl201.t16: func_8005D430 opens the next bank, func_8005D544 swaps the
 * sequence).  Two slots; `which` is the slot playing.  Include after ac1/ac1.h. */
#ifndef AC1_SNDBANK_H
#define AC1_SNDBANK_H

#include "types.h"

typedef struct SndBanks {
    s16 vab[2];                 /* +0x00 SsVabOpenHead ids */
    s16 seq[2];                 /* +0x04 SsSeqOpen ids */
    u8 *head[2];                /* +0x08 VAB header data */
    u8 *body[2];                /* +0x10 VAB body data */
    u8 *seqdat[2];              /* +0x18 SEQ data */
    u8 which;                   /* +0x20 slot playing */
    u8 busy[3];                 /* +0x21 transfers in flight */
    s8 unk24;                   /* +0x24 */
} SndBanks;

#endif
