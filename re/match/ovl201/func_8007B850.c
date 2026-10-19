#include "ac1_types.h"
/* ovl201 func_8007B850 - when bit 0x20 or 0x40 of D_801A2568 is set: play a key-on
 * sound (tone 2 for 0x40), latch the bits into D_801A2550, mark menu items 3 and 10,
 * release the two handles at D_801F36D4/D_801F36D8, draw a box, and then either
 * install func_8007BBC0 as this object's handler (when its parent is D_801BC4F8)
 * or mark a few more items and hand the parent back its saved handler. */

typedef struct {
    u8 b0;
    u8 visible;     /* +0x01 */
} Func8007B850Item;

typedef struct Func8007B850Obj Func8007B850Obj;

typedef struct {
    Func8007B850Obj *parent;    /* +0x00 */
    void (*saved)(void);        /* +0x04 */
} Func8007B850Link;

struct Func8007B850Obj {
    u8 b0;
    u8 active;                  /* +0x01 */
    u8 pad[0x72];
    void (*handler)(void);      /* +0x74 */
    u8 pad2[0x7C - 0x78];
    Func8007B850Link link;      /* +0x7C */
};

extern s32 D_801A2568;
extern s32 D_801A2550;
extern s16 D_801A2318;
extern s32 D_801F36D4;
extern s32 D_801F36D8;
extern s32 D_801BC47C;
extern s32 D_801BC480;
extern Func8007B850Obj D_801BC4F8;
extern u8 D_80072838[];

extern short SsUtKeyOn(short vab, short prog, short tone, short note, short fine, short voll, short volr);
extern void func_800506EC(s32 n);
extern Func8007B850Item *func_8005015C(s32 n);
extern Func8007B850Item *func_80050078(u8 *name);
extern void func_80051078(s32 h);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 kind);
extern void func_8007BBC0(void);

void func_8007B850(Func8007B850Obj *obj)
{
    Func8007B850Link *link = &obj->link;
    s32 *flags;
    s32 *handle;

    if (D_801A2568 & 0x60) {
        if (D_801A2568 & 0x40) {
            SsUtKeyOn(D_801A2318, 0, 2, 60, 0, 127, 127);
        } else {
            SsUtKeyOn(D_801A2318, 0, 0, 60, 0, 127, 127);
        }
        flags = &D_801A2550;
        *flags |= D_801A2568;
        func_800506EC(4);
        func_8005015C(3)->visible = 1;
        func_800506EC(10);
        func_8005015C(10)->visible = 1;
        handle = &D_801F36D4;
        if (*handle != 0) {
            func_80051078(*handle);
            *handle = 0;
            D_801BC47C = 0;
        }
        if (D_801F36D8 != 0) {
            func_80051078(D_801F36D8);
            D_801F36D8 = 0;
            D_801BC480 = 0;
        }
        func_8005C2D0(16, 40, 144, 128, 3);
        if (link->parent == &D_801BC4F8) {
            obj->handler = func_8007BBC0;
        } else {
            func_800506EC(6);
            func_8005015C(5)->visible = 1;
            func_8005C2D0(176, 24, 128, 144, 5);
            func_80050078(D_80072838)->visible = 1;
            link->parent->handler = link->saved;
            obj->active = 1;
        }
    }
}
