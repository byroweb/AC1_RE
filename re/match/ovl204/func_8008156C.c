#include "ac1/ac1.h"

typedef struct {
    u8 b[8];
} Blk8;

typedef struct {
    u8 state;                   /* +0x00 */
    s8 unk1;                    /* +0x01 */
    s8 unk2;                    /* +0x02 */
    u8 unk3;
    s32 unk4;                   /* +0x04 */
    s16 unk8;                   /* +0x08 */
    s16 unkA;                   /* +0x0A */
    s16 unkC;                   /* +0x0C */
    s16 unkE;                   /* +0x0E */
    s16 unk10;                  /* +0x10 */
    s16 unk12;
    s32 unk14;                  /* +0x14 */
    s32 unk18;                  /* +0x18 */
    u8 unk1C[0x10];
    Blk8 blk;                   /* +0x2C */
} Fx;

typedef struct {
    u8 unk0[0x398];
    Fx *fx;                     /* +0x398 */
} Ctl;

extern Ctl *D_80039D18;
extern Blk8 D_80039C78;

void func_8008156C(void)
{
    D_80039D18->fx->state = 0xFF;
    D_80039D18->fx->unk8 = 0;
    D_80039D18->fx->unk18 = 0;
    D_80039D18->fx->blk = D_80039C78;
}
