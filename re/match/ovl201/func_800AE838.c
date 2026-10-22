#include "ac1/ac1.h"

extern s32 D_80048630[12];      /* config: pad mapping */
extern u8 D_80048660;
extern u8 D_80048661;           /* master volume / 2 */
extern u8 D_80048662;           /* music volume / 2 */
extern s16 D_80041230[12];
extern u8 D_80041BB8;
extern u8 D_80041BBA;
extern u8 D_80039D10;
extern u8 D_801BC44C;
typedef struct {
    s16 seq[14];                /* 0x801BC454 SEQ access numbers */
    u8 cur;                     /* 0x801BC470 */
} SeqState;

extern SeqState D_801BC454;

/* Apply the config block: pad mapping, volumes. */
void func_800AE838(void) {
    s16 mv = D_80048661 * 2;

    D_80041BB8 = D_80048661 * 2;
    D_80041230[0] = D_80048630[0];
    D_80041230[3] = D_80048630[3];
    D_80041230[2] = D_80048630[2];
    D_80041230[1] = D_80048630[1];
    D_80041230[4] = D_80048630[10];
    D_80041230[7] = D_80048630[8];
    D_80041230[5] = D_80048630[9];
    D_80041230[6] = D_80048630[11];
    D_80041230[8] = D_80048630[4];
    D_80041230[9] = D_80048630[6];
    D_80041230[10] = D_80048630[5];
    D_80041230[11] = D_80048630[7];
    D_80041BBA = D_80048662 * 2;
    D_80039D10 = D_80048660;
    SsSetMVol(mv, mv);
    if (D_801BC44C) {
        SsSeqSetVol(D_801BC454.seq[D_801BC454.cur], D_80048662 * 2, D_80048662 * 2);
    }
}
