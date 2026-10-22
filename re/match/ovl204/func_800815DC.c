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


void func_800815DC(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g, s32 h)
{
    D_80039D18->fx->unkA = a;
    D_80039D18->fx->unkC = b;
    D_80039D18->fx->unk10 = c;
    D_80039D18->fx->unk14 = d;
    D_80039D18->fx->unkE = e;
    D_80039D18->fx->unk1 = f;
    D_80039D18->fx->unk2 = g;
    D_80039D18->fx->unk4 = h;
}
