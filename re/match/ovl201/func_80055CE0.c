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
    u8 kind;            /* +0xA5 'S' save / 'L' load */
    u8 padA6[2];
    u8 col;             /* +0xA8 */
    u8 row;             /* +0xA9 */
} SaveWork;

/* memory card directory, one per slot (0x88C) */
typedef struct {
    u32 used;           /* +0x00 used blocks */
    s32 files;          /* +0x04 */
    s32 saves;          /* +0x08 our save files */
    u8 pad[0x88C - 0xC];
} CardDir;

extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern s32 func_800684A0(u8 slot);
extern void func_800B4054(u8 slot);
extern s32 func_800B4BD4(u8 slot);
extern CardDir D_801EF6D0[2];
extern RECT D_8004B308;
extern void func_80054188(UiObj *o);
extern void func_800541F0(UiObj *o);
extern void func_80054258(UiObj *o);
extern void func_80055F08(UiObj *o);
extern void func_80055F70(UiObj *o);

void func_80055CE0(UiObj *o)
{
    SaveWork *w = (SaveWork *)&o->w;
    RECT r;

    if (func_800684A0(o->w.raw[0x28]) != 0) {
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        o->update = func_80054258;
    }
    func_800B4054(o->w.raw[0x28]);
    o->w.raw[0x2C] = 0;
    o->w.raw[0x2D] = 0;
    func_800506EC(10);
    obj_find_by_id(9)->kill = 1;
    func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
    if (o->w.raw[0x29] == 'S') {
        if (D_801EF6D0[o->w.raw[0x28]].used >= 14 && D_801EF6D0[o->w.raw[0x28]].saves == 0) {
            o->update = func_80054188;
        } else {
            o->update = func_80055F70;
        }
    } else if (D_801EF6D0[o->w.raw[0x28]].saves == 0) {
        o->update = func_800541F0;
    } else {
        o->update = func_80055F70;
    }
    r = D_8004B308;
    ClearImage(&r, 0, 0, 0);
    DrawSync(0);
    if (D_801EF6D0[w->slot].saves != 0) {
        if (!(u8)func_800B4BD4(w->slot)) {
            o->update = func_80055F08;
            return;
        }
        DrawSync(0);
    }
}
