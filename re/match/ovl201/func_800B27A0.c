#include "ac1/ac1.h"

typedef struct RankEnt {
    char *name;                 /* +0x00 */
    char *sub;                  /* +0x04 */
    s32 kind;                   /* +0x08 0xF = the player's row */
    u32 points;                 /* +0x0C sort key */
} RankEnt;

extern RankEnt D_8017F584[16];
extern char D_8017F538[6][3];
extern char D_8017F54C[6][7];
extern UiObj *D_8017F684[3][6];
extern void func_800518D4(char *, s16);
extern UiObj *func_8005CB94(char *, s32, s16, s32);
extern UiObj *func_8005D80C(s32, char *, s32, s32, s32, s32);
extern void func_800B29AC(char *, u32);

void func_800B27A0(UiObj *o)
{
    s16 i;
    s16 y;
    s16 r;

    r = o->w.raw[0x1D];
    for (i = 0; i < 6; i++) {
        func_800518D4(D_8017F538[i], r + 1);
        func_800B29AC(D_8017F54C[i], D_8017F584[r].points);
        D_8017F684[0][i]->kill = 1;
        D_8017F684[1][i]->kill = 1;
        D_8017F684[2][i]->kill = 1;
        y = i * 24 + 4;
        D_8017F684[1][i] = func_8005D80C(2, D_8017F538[i], 16, 16, y, 4);
        D_8017F684[0][i] = func_8005CB94(D_8017F584[r].name, 0x30, y, 4);
        D_8017F684[0][i]->unk14 = 2;
        D_8017F684[2][i] = func_8005D80C(2, D_8017F54C[i], 16, 0xC0, y, 4);
        r++;
    }
    o->w.ring.item[2]->visible = 1;
    o->w.ring.item[3]->visible = 1;
    if (o->w.raw[0x1D] == 0) {
        o->w.ring.item[2]->visible = 0;
    }
    if (o->w.raw[0x1D] >= 4) {
        o->w.ring.item[3]->visible = 0;
    }
}
