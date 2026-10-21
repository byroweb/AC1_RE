#include "ac1/ac1.h"

extern void func_800B54B4(char *msg);
extern volatile u32 *D_800BE350;
extern char D_8004DC30[];

/* wait for the busy bit to drop; give up after 0x100000 polls */
s32 func_800B536C(void)
{
    volatile s32 timeout;

    timeout = 0x100000;
    while (*D_800BE350 & 0x20000000) {
        if (--timeout == -1) {
            func_800B54B4(D_8004DC30);
            return -1;
        }
    }
    return 0;
}
