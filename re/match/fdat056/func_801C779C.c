#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x2C];
} Slot;
typedef struct {
    Slot *slot;
    u8 pad4[0x15C];
    u16 f160;
} Ent;
extern Slot D_8019FAB8[];
extern s32 D_801CD760;
extern s32 D_801CD764;
extern s32 D_80049E78;

void func_801C779C(Ent *e)
{
    s32 i;

    if (e->f160 == 0) {
        i = e->slot - D_8019FAB8;
        if (i >= 4 && i < 11) {
            D_801CD760++;
        }
        if (i >= 11 && i < 18) {
            D_80049E78 = 0;
            D_801CD764++;
        }
    }
}
