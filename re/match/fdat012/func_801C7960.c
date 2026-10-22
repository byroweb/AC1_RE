#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x2C];
} Func801C7960Slot;     /* 0x2C */
typedef struct {
    Func801C7960Slot slots[256];    /* 0x8019FAB8 */
    AcEntity ents[16];              /* 0x801A26B8 */
} Func801C7960World;
extern Func801C7960World D_8019FAB8;
extern void (**D_8019F51C)(AcEntity *);
extern s32 D_801AC80C;
extern void func_80078B14(AcEntity *, Func801C7960Slot *);
extern void func_8008A778(s32, s32, s32);

void func_801C7960(void)
{
    D_801AC80C = 0;
    func_80078B14(&D_8019FAB8.ents[1], &D_8019FAB8.slots[4]);
    D_8019F51C[D_8019FAB8.ents[1].pos.w + 8](&D_8019FAB8.ents[1]);
    func_8008A778(0x1EF0, 0x100, 0x8B);
}
