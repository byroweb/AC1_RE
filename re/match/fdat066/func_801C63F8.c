#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    s16 kind;           /* +0x06 */
    u8 pad8[0x2C];
} Func801C63F8Slot;     /* 0x34 */
typedef struct {
    u8 pad0[0x0E];
    s16 id;             /* +0x0E */
    u8 pad10[8];
} Func801C63F8Kind;     /* 0x18 */
extern Func801C63F8Slot D_80198B08[];
extern Func801C63F8Kind D_80198800[];
extern s32 D_801CC0BC;

void func_801C63F8(void)
{
    Func801C63F8Slot *s;
    s32 i;

    D_801CC0BC = 0;
    i = 0;
    s = D_80198B08;
    do {
        if (s->kind != -1 && D_80198800[s->kind].id == 13) {
            D_801CC0BC--;
        }
        i++;
        s++;
    } while (i < 0x200);
}
