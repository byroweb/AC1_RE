#include "ac1/ac1.h"

/* save/load menu work area (UiObj +0x7C) */
typedef struct {
    UiObj *owner;       /* +0x7C */
    UiObj *arg;         /* +0x80 */
    UiObj *box;         /* +0x84 */
    u8 pad88[0x10];
    s32 status[2];      /* +0x98 per card slot */
    u8 unkA0;           /* +0xA0 */
    u8 unkA1;           /* +0xA1 */
    s8 cursor;          /* +0xA2 row on screen */
    u8 mode;            /* +0xA3 */
    u8 slot;            /* +0xA4 */
    u8 kind;            /* +0xA5 'S' save / 'L' load */
    u8 top;             /* +0xA6 first entry shown */
    u8 unkA7;
    u8 padA8[2];
    u8 unkAA;           /* +0xAA */
    u8 own_cc48;        /* +0xAB */
} SaveWork;

extern void func_80052334(UiObj *o);
extern void func_80053008(UiObj *o);
extern void func_8005BA2C(s32 x, s32 y, s32 w, s32 h, s32 a, s32 b, s32 c);
extern UiObj *func_8005CAE0(void *p, s32 x, s32 y, s32 n);
extern void func_8005CC48(void);
extern void func_8005D71C(void);
extern void func_8005DD38(s32 a, s32 b, s32 c);
extern void func_8005DE08(s32 a, s32 b, s32 c);
extern void func_80086654(void);
extern u8 D_800B6EEC[];
extern s16 D_801A2318;
extern s32 D_801A2574;
extern void func_8004FDF4(UiObj *o);
extern void func_800A2D0C(UiObj *o);
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);

UiObj *func_80052134(UiObj *owner, UiObj *arg)
{
    UiObj *o;
    SaveWork *w;
    void *ctor;

    o = ui_obj_alloc(0);
    w = (SaveWork *)&o->w;
    o->ctor = func_80052134;
    o->update = func_80052334;
    w->owner = owner;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    w->unkA0 = 0;
    w->unkA1 = 0;
    w->cursor = 0;
    w->arg = arg;
    if (arg == 0) {
        w->box = func_8005CAE0(D_800B6EEC, 0x10, 0x10, 2);
    } else {
        w->box = &g_ui_none;
    }
    if (obj_find_by_handle(func_8005CC48) == &g_ui_none) {
        w->own_cc48 = 1;
        func_8005CC48();
    } else {
        w->own_cc48 = 0;
    }
    func_8005D71C();
    func_8005DD38(0x3C0, 0x10, 0xE);
    func_8005DE08(0x3C0, 0x28, 0xE);
    func_8005DD38(0x3C0, 0x40, 0x1E);
    func_8005DE08(0x3C0, 0x58, 0x1E);
    func_80086654();
    ctor = w->owner->ctor;
    if (ctor == func_800A2D0C) {
        o->update = func_80053008;
        w->kind = 'L';
        w->unkAA = 'D';
    } else if (ctor == func_8004FDF4) {
        o->update = func_80053008;
        w->kind = 'S';
        w->unkAA = 'D';
    } else if (arg != 0) {
        o->update = func_80053008;
        w->kind = 'L';
        w->unkAA = 'D';
    } else {
        func_8005BA2C(0x10, 0x28, 0x80, 0x78, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
    }
    D_801A2574 = 2;
    return o;
}
