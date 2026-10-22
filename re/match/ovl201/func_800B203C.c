#include "ac1/ac1.h"

typedef struct RankEnt {
    char *name;                 /* +0x00 */
    char *sub;                  /* +0x04 */
    u8 kind;                    /* +0x08 0xF = the player's row */
    u8 pad09[3];
    u32 points;                 /* +0x0C sort key */
} RankEnt;

extern RankEnt D_8017F584[16];
extern void func_800B2398(void);

s32 func_800B203C(void)
{
    u8 i;

    func_800B2398();
    for (i = 0; i < 16; i++) {
        if (D_8017F584[i].kind == 0xF) {
            return (u8)(i + 1);
        }
    }
    return 0;
}
