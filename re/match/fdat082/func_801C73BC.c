#include "ac1/ac1.h"

typedef struct {
    u8 st;              /* +0x00 */
    u8 pad1[9];
    s16 kind;           /* +0x0A */
    u8 padC[0x20];
} Slot;
extern Slot D_8019FB68[];
extern void (**D_8019F51C)();
extern void func_801C5438();
extern void func_801C5468();
extern void func_801C6544();

void func_801C73BC(s32 *score_a, s32 *score_b)
{
    Slot *s;
    s32 i;
    void (*f)();

    i = 0xFC;
    s = D_8019FB68;
    do {
        if (D_8019F51C[s->kind + 8] == func_801C6544 && s->st == 3) {
            *score_b -= 1000;
        }
        i--;
        s++;
    } while (i != 0);
    i = 0xFC;
    s = D_8019FB68;
    do {
        f = D_8019F51C[s->kind + 8];
        if ((f == func_801C5438 || f == func_801C5468) && s->st != 3) {
            return;
        }
        i--;
        s++;
    } while (i != 0);
    *score_a += 10000;
}
