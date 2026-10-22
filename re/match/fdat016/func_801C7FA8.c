#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x22];
    u16 wid;            /* +0x22 index into g_world_structs */
    u8 pad24[8];
} Slot2C;               /* 0x2C */
typedef struct {
    Slot2C slots[256];              /* 0x8019FAB8 */
    AcEntity ents[16];              /* 0x801A26B8 */
} World;
typedef struct {
    s32 flags;          /* +0x34 of the world struct */
    u8 pad[0x3C];
} WSf;                  /* 0x40 */
extern WSf D_801D0B9C[];
extern World D_8019FAB8;
extern void (**D_8019F51C)(AcEntity *);
extern s32 D_801AC80C;
extern void func_80078B14(AcEntity *, Slot2C *);

void func_801C7FA8(void)
{
    s32 i;
    Slot2C *s;

    D_801AC80C = 0;
    func_80078B14(&D_8019FAB8.ents[1], &D_8019FAB8.slots[4]);
    D_8019F51C[D_8019FAB8.ents[1].pos.w + 8](&D_8019FAB8.ents[1]);
    s = &D_8019FAB8.slots[5];
    i = 5;
    do {
        D_801D0B9C[s->wid].flags &= ~0xFFF;
        s++;
        i--;
    } while (i != 0);
}
