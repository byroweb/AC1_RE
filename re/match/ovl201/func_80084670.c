#include "ac1/ac1.h"
/* ovl201 0x80084670 - commit the picked entry: flush a busy CD queue, then either load
 * its file into the other slot or remember it for later; refresh the rows */
typedef struct PickWork {
    u8 pad7c[0x44];
    UiObj *cursor;              /* +0xC0 */
    u8 count;                   /* +0xC4 */
    u8 unkC5;
    u8 pos;                     /* +0xC6 */
    u8 unkC7;
    u8 slot;                    /* +0xC8 */
    u8 sel;                     /* +0xC9 */
    u8 unkCA;
    u8 unkCB;
    u8 unkCC;
} PickWork;
s16 func_800517C8(void);
void func_80051628(void);
u8 func_80084364(UiObj *o);
void func_80050AF4(s16 i, s16 id);
void func_80051450(s32 a, s32 b);
void func_80084A8C(UiObj *o);
void func_80084884(UiObj *o);

void func_80084670(UiObj *o)
{
    PickWork *w;
    u8 k;
    u8 s;

    w = (PickWork *)&o->w;
    if (func_800517C8() != 0) {
        func_80051628();
    }
    k = func_80084364(o);
    if (func_800517C8() == 0) {
        s = 1;
        if (w->slot == 1) {
            s = 2;
        }
        w->slot = s;
        func_80050AF4(s, k + 132);
        func_80051450(17, k);
        w->unkCA = 0;
        w->unkC7 = 1;
    } else {
        w->unkCA = 1;
        w->unkC7 = 0;
        w->unkCB = k + 132;
        w->unkCC = k;
    }
    func_80084A8C(o);
    func_80084884(o);
    w->cursor->kill = 1;
    w->sel = func_80084364(o);
}
