#include "ac1/ac1.h"

typedef struct MailState {
    u8 flags[64];
    u8 pad40[0x80];
    s8 count;
    u8 padC1[0xF];
    s8 order[64];
} MailState;

extern MailState D_8017F6D0;

void func_800968EC(void)
{
    s32 i;
    s32 n;

    n = D_8017F6D0.count - 1;
    for (i = 64; i != 0; i--) {
        if (D_8017F6D0.flags[64 - i] & 1) {
            D_8017F6D0.order[n - (D_8017F6D0.flags[64 - i] >> 2)] = 64 - i;
        }
    }
}
