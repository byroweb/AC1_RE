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
    u8 tag;             /* +0xA5 */
    u8 top;             /* +0xA6 first entry shown */
} SaveWork;

extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern void func_8005DED8(UiObj *o, void *fn, void *msg, s32 n);
extern void func_8005E23C(UiObj *o, void *fn, void *msg, s32 n);
extern s32 func_800B4398(u8 slot, s32 n);
extern s32 func_800684A0(u8 slot);
extern u8 D_8004B1E8[];
extern u8 D_8004B208[];
extern u8 D_8004B21C[];
extern u16 D_801A257C;
extern s16 D_801A2318;
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_8004FDF4(UiObj *o);
extern void func_800537E0(UiObj *o);
extern void func_80054258(UiObj *o);
extern void func_8005552C(UiObj *o);
extern void func_800549D0(UiObj *o);

void func_800569E8(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;
    UiObj *self;

    w->status[o->w.raw[0x28]] = func_800684A0(o->w.raw[0x28]);
    self = o;
    if (w->status[self->w.raw[0x28]] != 0) {
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
        func_8005DED8(self, func_800549D0, D_8004B1E8, 0xC);
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
    } else {
        if ((u8)func_800B4398(self->w.raw[0x28], self->w.raw[0x2C] + self->w.raw[0x2D] * 4)) {
            func_8005DED8(self, func_800537E0, D_8004B208, 0xC);
        } else {
            func_8005DED8(self, func_80054258, D_8004B21C, 0xC);
        }
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
    }
    self->update = ui_obj_nop2;
}
