#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x2C];
} Slot2C;               /* 0x2C */
typedef struct {
    Slot2C slots[256];              /* 0x8019FAB8 */
    AcEntity ents[16];              /* 0x801A26B8 */
} World;
extern World D_8019FAB8;
extern void (**D_8019F51C)(AcEntity *);
extern s32 D_801AC80C;
extern void func_80078B14(AcEntity *, Slot2C *);
extern void func_80056C6C(s32, s32, s32, s32, s32, s32, s32);

void func_801C6DC4(void)
{
    D_801AC80C = 0;
    func_80078B14(&D_8019FAB8.ents[1], &D_8019FAB8.slots[4]);
    D_8019F51C[D_8019FAB8.ents[1].pos.w + 8](&D_8019FAB8.ents[1]);
    func_80056C6C(1, 507, 1, 1, 2, 13, 1);
}
