#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    s16 kind;           /* +0x06 */
    u8 pad8[0x2C];
} Func801C6474Slot;     /* 0x34 */
typedef struct {
    u8 pad0[0x0E];
    s16 id;             /* +0x0E */
    u8 pad10[8];
} Func801C6474Kind;     /* 0x18 */
extern Func801C6474Slot D_80198B08[];
extern Func801C6474Kind D_80198800[];
extern s32 D_801CC0BC;

void func_801C6474(s32 a0, s32 *p)
{
    Func801C6474Slot *s;
    s32 i;

    i = 0;
    s = D_80198B08;
    do {
        if (s->kind != -1 && D_80198800[s->kind].id == 13) {
            D_801CC0BC++;
        }
        i++;
        s++;
    } while (i < 0x200);
    *p -= D_801CC0BC * 200;
}

