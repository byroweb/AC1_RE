#include "ac1_types.h"
typedef struct {
    s8 type;        /* +0x00 */
    s8 arg;         /* +0x01 */
} Func80096550Cond;
typedef struct {
    u8 pad[2];
    s8 b2;          /* +0x02 (0x801F3752) */
    u8 pad2[0x25];
    s32 w28;        /* +0x28 (0x801F3778) */
} Func80096550State;
extern u8 D_8017F6D0[];
extern Func80096550Cond D_8017F710[];
extern Func80096550State D_801F3750;
extern u8 D_801F3788[];
s32 func_80096550(void)
{
    s32 n;
    s32 k;
    u8 f;
    for (n = 64; n != 0; n--) {
        if (D_8017F6D0[64 - n] & 1) {
            continue;
        }
        k = 64 - n;
        switch (D_8017F710[k].type) {
        case 0:
            if (D_801F3750.b2 >= D_8017F710[k].arg) {
                return 1;
            }
            break;
        case 1:
            if (D_801F3788[D_8017F710[k].arg] & 2) {
                return 1;
            }
            break;
        case 2:
            f = D_801F3788[D_8017F710[k].arg];
            if (f & 2) {
                return 1;
            }
            if (f & 4) {
                return 1;
            }
            break;
        case 3:
            if (D_801F3750.w28 < 0) {
                if (D_801F3750.b2 >= 26) {
                    return 1;
                }
            }
            break;
        }
    }
    return 0;
}
