#include "ac1/ac1.h"

typedef struct PadRep {
    u32 latch;                  /* +0x00 0x801A2550 */
    s16 delay;                  /* +0x04 */
    s16 rate;                   /* +0x06 */
    u8 pad08[0x10];
    u32 press;                  /* +0x18 0x801A2568 */
    u32 held;                   /* +0x1C 0x801A256C */
} PadRep;

extern PadRep D_801A2550_s;
extern u32 D_801A2550;
extern s16 D_801A2554;
extern s16 D_801A2556;
extern u32 D_801A256C;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern void func_800506EC(s32);
extern void func_800A576C(void);
extern void func_800A6A04(UiObj *);
extern void func_800A6A64(UiObj *);
extern void func_800A6948(UiObj *);

void func_800A6664(UiObj *o)
{

    u8 *w = o->w.raw;

    if (D_801A2550_s.press & 0x1000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550_s.latch |= 0x1000;
        if (D_801A2574 != 0) {
            D_801A2550_s.delay = 6 / D_801A2574;
            if (!(D_801A2550_s.held & 0x1000)) {
                D_801A2550_s.rate = 12 / D_801A2574;
            }
        } else {
            D_801A2550_s.delay = 6;
            if (!(D_801A2550_s.held & 0x1000)) {
                D_801A2550_s.rate = 12;
            }
        }
        if (w[15] == 0) {
            w[15] = 1;
        } else {
            w[15]--;
        }
    } else if (D_801A2550_s.press & 0x4000) {
        SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550 |= 0x4000;
        if (D_801A2574 != 0) {
            D_801A2554 = 6 / D_801A2574;
            if (!(D_801A256C & 0x4000)) {
                D_801A2556 = 12 / D_801A2574;
            }
        } else {
            D_801A2554 = 6;
            if (!(D_801A256C & 0x4000)) {
                D_801A2556 = 12;
            }
        }
        if (w[15] == 1) {
            w[15] = 0;
        } else {
            w[15]++;
        }
    } else if (D_801A2550_s.press & 0x40) {
        SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550 |= 0x40;
        if (o->w.raw[15] == 0) {
            func_800A576C();
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            o->update = func_800A6A04;
        } else {
            func_800A576C();
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            o->update = func_800A6A64;
        }
    } else if (D_801A2550_s.press & 0x20) {
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        D_801A2550_s.latch |= 0x20;
        o->update = func_800A6948;
    }
}
