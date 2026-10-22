#include "ac1/ac1.h"

typedef struct {
    u8 mode;            /* 0x801987D0 */
    u8 pad01[7];
    s16 unk08;          /* 0x801987D8 */
    u8 pad0a[0x0E];
    s32 unk18;          /* 0x801987E8 */
    u8 pad1c[0x10];
    SVECTOR vec;        /* 0x801987FC */
} CamState;

extern CamState D_801987D0;
extern SVECTOR D_80039C78;

void func_80082C58(void)
{
    D_801987D0.mode = 0xFF;
    D_801987D0.unk08 = 0;
    D_801987D0.unk18 = 0;
    D_801987D0.vec = D_80039C78;
}
