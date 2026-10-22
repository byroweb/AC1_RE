#include "ac1/ac1.h"

typedef struct {
    u8 st;              /* +0x00 */
    u8 pad1[9];
    s16 kind;           /* +0x0A */
    u8 padC[0x20];
} Slot;
extern Slot D_8019FB68[];
extern u16 D_801C713C;
extern u8 D_801C713E;
extern void func_8004C318();
extern void func_800529F0();
extern void func_8008A80C();

void func_801C6F78(void)
{
    Slot *s;
    s32 i;

    func_8008A80C();
    if (D_801C713E == 0) {
        s32 t = D_801C713C;
        D_801C713C = t + 1;
        if ((t & 0xF) == 0) {
            i = 0xFC;
            s = D_8019FB68;
            do {
                if (s->kind != -1 && s->st != 3) {
                    return;
                }
                i--;
                s++;
            } while (i != 0);
            func_800529F0(0, 0, 0x68);
            func_8004C318(0x80);
            D_801C713E = 1;
        }
    }
}
