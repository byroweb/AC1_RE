#include "ac1/ac1.h"

typedef struct {
    u8 state;           /* +0x00 */
    u8 idx;             /* +0x01 index into g_ac_entities */
    u8 pad2[8];
    s16 link;           /* +0x0A -1 = none */
    u8 pad0c[0x18];
    u8 f24;             /* +0x24 */
    u8 pad25[7];
} Slot2C;               /* 0x2C */

extern Slot2C D_8019FB68;
extern Slot2C D_8019FE28;
extern u16 D_8019F528;
extern s32 D_801C6DE4;
extern s32 D_801C6DE8;
extern s32 D_801C6DEC;
extern void func_8004C318(s32);
extern s32 func_80052338(void);
extern void func_800529F0(s32, s32, s32);
extern void func_8008A80C(void);

void func_801C6994(void)
{
    func_8008A80C();
    if (D_8019F528 == 0) {
        if (D_801C6DE8 == 0) {
            if (D_8019FB68.link != -1) {
                if (g_ac_entities[D_8019FB68.idx].def == (void *)&D_8019FB68
                    && g_ac_entities[D_8019FB68.idx].ai_mode != 0xFF) {
                    D_801C6DE8 = 1;
                }
                goto block6;
            }
            goto block12;
        }
block6:
        if (D_8019FB68.link != -1 && D_8019FB68.state == 3 && func_80052338() != 0) {
            if (D_801C6DEC == 10) {
                func_800529F0(0, 0, 104);
                func_8004C318(128);
            }
            D_801C6DEC++;
        } else {
block12:
            if (D_801C6DE4 == 0 && D_8019FB68.state == 1 && D_8019FB68.f24 != 0
                && g_ac_entities[D_8019FB68.idx].ap <= 4000) {
                D_801C6DE4 = D_8019FB68.state;
            }
        }
        if (D_8019FE28.state == 1 && func_80052338() != 0) {
            if (g_ac_entities[D_8019FE28.idx].ai_mode == 0xFF) {
                g_ac_entities[D_8019FE28.idx].ai_mode = 20;
                g_ac_entities[D_8019FE28.idx].ap = 100;
            }
        }
    }
}
