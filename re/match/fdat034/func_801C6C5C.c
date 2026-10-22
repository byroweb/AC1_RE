#include "ac1/ac1.h"

typedef struct {
    u8 st;
    u8 pad1[0x2B];
} Slot;
extern Slot D_8019FAB8[];
extern void func_801C6B44();

void func_801C6C5C(s32 a0, s32 a1, s32 a2, s32 a3)
{
    s32 i;
    char pad[8];
    Slot *p;

    i = a3 - 1;
    if (a3 != 0) {
        do {
            func_801C6B44(a0, a1, a2);
            i--;
        } while (i != -1);
    }
    p = &D_8019FAB8[a1];
    a2--;
    if (a2 != -1) {
        do {
            if (p->st == 0) {
                p->st = 3;
            }
            a2--;
            p++;
        } while (a2 != -1);
    }
}
