#include "ac1/ac1.h"

/* save/load menu work area (UiObj +0x7C) */
typedef struct {
    UiObj *owner;       /* +0x7C */
    u8 pad80[0x18];
    s32 status[2];      /* +0x98 per card slot */
    u8 padA0[2];
    s8 cursor;          /* +0xA2 row on screen */
    u8 mode;            /* +0xA3 */
    u8 slot;            /* +0xA4 */
} SaveWork;

extern void func_800506EC(s32 id);
extern s32 func_800684A0(u8 slot);
extern void func_800531B8(UiObj *o);
extern void func_80055784(UiObj *o);
extern void func_800558DC(UiObj *o);
extern void func_8005BA2C();
extern void func_8005D8D4(void);

void func_800557CC(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;

    w->status[o->w.raw[0x28]] = func_800684A0(o->w.raw[0x28]);
    if (w->status[o->w.raw[0x28]] != 4) {
        obj_find_by_handle(func_8005D8D4)->kill = 1;
        obj_find_by_handle(func_8005BA2C)->kill = 1;
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        o->update = func_80055784;
        return;
    }
    if (o->w.raw[0x27] == 0) {
        func_8005BA2C(0xC, 0x60, 0x128, 0x30, 9, 1, 0);
        o->update = func_800558DC;
    } else if (o->w.raw[0x27] == 1) {
        o->update = func_800531B8;
    }
}
