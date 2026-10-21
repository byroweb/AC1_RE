#include "ac1/ac1.h"

typedef struct {
    u8 pad0[2];
    u16 busy;           /* +0x02 */
    u8 pad4[6];
    s16 kind;           /* +0x0A */
    u8 padC[0x20];
} Func801C77F0Slot;     /* 0x2C */
extern Func801C77F0Slot D_8019FAB8[];
extern void (**D_8019F51C)();
extern void func_801C57D4();

void func_801C77F0(s32 arg0, s32 *score)
{
    Func801C77F0Slot *s;
    s32 i;

    i = 255;
    s = D_8019FAB8;
    do {
        if (s->kind != -1 && s->busy == 0 && D_8019F51C[s->kind + 8] == func_801C57D4) {
            *score -= 500;
        }
        i--;
        s++;
    } while (i != -1);
}
