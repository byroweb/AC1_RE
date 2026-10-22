#include "ac1/ac1.h"

/* name-entry work area (UiObj +0x7C) */
typedef struct {
    void *dst;          /* +0x7C */
    void *arg;          /* +0x80 */
    u8 pad84[0x11];
    u8 mode;            /* +0x95 'A' = AC name, else pilot name */
    u8 len;             /* +0x96 */
    u8 pad97[3];
    u8 x;               /* +0x9A */
    u8 y;               /* +0x9B */
} NameWork;

extern char D_80031BD4[];
extern char D_80031BE6[];
extern char D_8004C6EC[];
extern char D_8004C6F4[];
extern UiObj *func_8005CB94(char *s, s32 a, s32 b, s32 c);
extern UiObj *func_8005D80C(s32 a, char *s, s32 b, s32 c, s32 d, s32 e);
extern u8 func_80081260(char *s);
extern void func_8008131C(void);
extern void func_80081C5C(UiObj *o);
extern void func_80083894(u8 *x, u8 *y, s32 n);
extern void func_80083984(UiObj *o);

void func_800810F0(void *dst, void *arg, u8 mode)
{
    UiObj *o;
    NameWork *w;
    char *name;

    o = ui_obj_alloc(0);
    o->ctor = func_800810F0;
    o->update = func_80081C5C;
    o->destroy = ui_obj_nop;
    ((NameWork *)&o->w)->x = 0x10;
    ((NameWork *)&o->w)->dst = dst;
    ((NameWork *)&o->w)->mode = mode;
    ((NameWork *)&o->w)->y = 7;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    ((NameWork *)&o->w)->arg = arg;
    w = (NameWork *)&o->w;
    if (mode == 'A') {
        ((NameWork *)&o->w)->len = func_80081260(D_80031BD4);
    } else {
        ((NameWork *)&o->w)->len = func_80081260(D_80031BE6);
    }
    func_80083894(&w->x, &w->y, 4);
    func_80083984(o);
    if (w->mode == 'A') {
        func_8005D80C(1, D_8004C6EC, 0x10, 8, 4, 6);
        name = D_80031BD4;
    } else {
        func_8005D80C(1, D_8004C6F4, 0x10, 8, 4, 6);
        name = D_80031BE6;
    }
    func_8005CB94(name, 8, 0x1C, 6)->unk14 = 3;
    func_8008131C();
}
