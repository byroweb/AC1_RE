#include "ac1/ac1.h"

typedef struct RankEnt {
    char *name;                 /* +0x00 */
    char *sub;                  /* +0x04 */
    s32 kind;                   /* +0x08 0xF = the player's row */
    u32 points;                 /* +0x0C sort key */
} RankEnt;

extern RankEnt D_800BDFC8[16];
extern RankEnt D_8017F584[16];
extern s32 D_80049E70;
extern s32 D_80049E74[];
extern char D_80031BE6[];

void func_800B2398(void)
{
    s16 i;
    s16 j;
    RankEnt tmp;

    for (i = 0; i < 15; i++) {
        D_800BDFC8[i].points = D_80049E74[i];
    }
    D_800BDFC8[15].name = D_80031BE6;
    D_800BDFC8[15].sub = D_80031BE6 - 0x12;
    D_800BDFC8[15].points = D_80049E70;
    for (i = 0; i < 16; i++) {
        D_8017F584[i] = D_800BDFC8[i];
    }
    for (i = 0; i < 15; i++) {
        for (j = 15; i < j; j--) {
            if (D_8017F584[j].points > D_8017F584[j - 1].points) {
                tmp = D_8017F584[j];
                D_8017F584[j] = D_8017F584[j - 1];
                D_8017F584[j - 1] = tmp;
            }
        }
    }
}
