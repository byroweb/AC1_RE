#include "ac1/ac1.h"

/* memory card directory per slot, 0x88C each at 0x801EF6D0 */
typedef struct {
    u32 used;           /* +0x00 */
    u8 pad4[4];
    s32 unk8;           /* +0x08 */
    u8 padC[0x88C - 0xC];
} CardInfo;
extern CardInfo D_801EF6D0[];

extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern void func_80066994(u8 slot);
extern void func_80066CB0(u8 slot);
extern s32 func_800684A0(u8 slot);
extern void func_80053C98(UiObj *o);
extern void func_80054188(UiObj *o);
extern void func_800541F0(UiObj *o);
extern void func_80054258(UiObj *o);

void func_80053AB4(UiObj *o)
{
    u8 *w = o->w.raw;

    if (func_800684A0(o->w.raw[0x28]) != 0) {
        func_800506EC(10);
        obj_find_by_id(9)->kill = 1;
        o->update = func_80054258;
    }
    func_80066994(o->w.raw[0x28]);
    if (o->w.raw[0x29] == 'S' && D_801EF6D0[o->w.raw[0x28]].used < 15) {
        o->w.raw[0x2B] = 1;
    } else {
        o->w.raw[0x2B] = 0;
    }
    func_80066CB0(w[0x28]);
    w[0x2A] = 0;
    w[0x26] = 0;
    func_800506EC(10);
    obj_find_by_id(9)->kill = 1;
    func_8005C2D0(0xC, 0x68, 0x128, 0x20, 9);
    if (w[0x29] == 'S') {
        if (D_801EF6D0[w[0x28]].used >= 15 && D_801EF6D0[w[0x28]].unk8 == 0) {
            o->update = func_80054188;
        } else {
            o->update = func_80053C98;
        }
    } else if (D_801EF6D0[w[0x28]].unk8 == 0) {
        o->update = func_800541F0;
    } else {
        o->update = func_80053C98;
    }
}
