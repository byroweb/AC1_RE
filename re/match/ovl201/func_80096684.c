#include "ac1/ac1.h"

typedef struct MailCond {
    s8 type;
    s8 arg;
} MailCond;

typedef struct MailState {
    u8 flags[64];
    MailCond cond[64];
    u8 count;
    s8 mode;
} MailState;

extern MailState D_8017F6D0;
extern s32 D_801A2568;
extern s8 D_801F3752;

typedef struct GameProg {
    u8 pad0[2];
    s8 rank;            /* +0x02 = 0x801F3752 */
    u8 pad3[0x25];
    s32 x28;            /* +0x28 = 0x801F3778 */
} GameProg;

extern GameProg D_801F3750;
extern u8 D_801F3788[];

void func_80096684(void)
{
    s32 i;

    i = 64;
    if (D_8017F6D0.mode == 1 && (D_801A2568 & 0xC) == 0xC) {
        for (; i != 0; i--) {
            if (!(D_8017F6D0.flags[64 - i] & 1)) {
                switch (D_8017F6D0.cond[64 - i].type) {
                case 0:
                case 1:
                case 2:
                case 3:
                    D_8017F6D0.flags[64 - i] |= 1;
                    D_8017F6D0.flags[64 - i] |= D_8017F6D0.count << 2;
                    D_8017F6D0.count++;
                    break;
                }
            }
        }
    }
    for (; i != 0; i--) {
        if (!(D_8017F6D0.flags[64 - i] & 1)) {
            switch (D_8017F6D0.cond[64 - i].type) {
            case 0:
                if (D_801F3752 >= D_8017F6D0.cond[64 - i].arg) {
                    D_8017F6D0.flags[64 - i] |= 1;
                    D_8017F6D0.flags[64 - i] |= D_8017F6D0.count << 2;
                    D_8017F6D0.count++;
                }
                break;
            case 1:
                if (D_801F3788[D_8017F6D0.cond[64 - i].arg] & 2) {
                    D_8017F6D0.flags[64 - i] |= 1;
                    D_8017F6D0.flags[64 - i] |= D_8017F6D0.count << 2;
                    D_8017F6D0.count++;
                }
                break;
            case 2:
                if ((D_801F3788[D_8017F6D0.cond[64 - i].arg] & 2) || (D_801F3788[D_8017F6D0.cond[64 - i].arg] & 4)) {
                    D_8017F6D0.flags[64 - i] |= 1;
                    D_8017F6D0.flags[64 - i] |= D_8017F6D0.count << 2;
                    D_8017F6D0.count++;
                }
                break;
            case 3:
                if (D_801F3750.x28 < 0 && D_801F3750.rank >= 26) {
                    D_8017F6D0.flags[18] |= 1;
                    D_8017F6D0.flags[18] |= D_8017F6D0.count << 2;
                    D_8017F6D0.count++;
                }
                break;
            }
        }
    }
}
