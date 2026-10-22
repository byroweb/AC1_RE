#include "ac1/ac1.h"

typedef struct MailView {
    u8 pad0;
    s8 row;             /* +0x01 cursor row on the page */
    u8 pad2[0x22];
    s16 top;            /* +0x24 list y origin */
} MailView;

typedef struct MailState {
    u8 flags[64];
    u8 cond[64][2];
    u8 count;
    s8 mode;
    u8 padC2[3];
    s8 sel;             /* +0xC5 selected list entry */
    u8 padC6[0xA];
    s8 order[64];       /* +0xD0 */
} MailState;

extern MailState D_8017F6D0;
extern MailView *D_80180860;
extern s32 D_801A2568;
extern s32 D_801A256C;
extern s16 D_801A2318;

void func_800997C8(UiObj *o);
void func_8009A3B8(UiObj *o);
void func_8009A508(UiObj *o);
void func_8009A658(UiObj *o);
void func_8009A714(UiObj *o);
void func_80099E30(UiObj *o);
void func_8009A4FC(UiObj *o);
void func_8009A64C(UiObj *o);
void func_8009A708(UiObj *o);
void func_8009A7DC(UiObj *o);
void func_800978A8(UiObj *o);
void func_80098550(void);
void func_80097B4C(void);
void func_800506EC(s32 a);
void func_8005C2D0(s32 a, s16 b, s32 c, s32 d, s32 e);

void func_8009760C(UiObj *o)
{
    UiObj *o4;
    UiObj *o0;
    UiObj *o1;
    UiObj *o2;
    UiObj *o3;

    o4 = obj_find_by_handle(func_800997C8);
    o0 = obj_find_by_handle(func_8009A3B8);
    o1 = obj_find_by_handle(func_8009A508);
    o2 = obj_find_by_handle(func_8009A658);
    o3 = obj_find_by_handle(func_8009A714);
    if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
        D_8017F6D0.flags[D_8017F6D0.order[D_8017F6D0.sel]] |= 2;
        func_80098550();
        func_80097B4C();
        o0->update = func_8009A4FC;
        o1->update = func_8009A64C;
        o2->update = func_8009A708;
        o3->update = func_8009A7DC;
        o4->update = func_80099E30;
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x20, D_80180860->top + 92, 0x100, D_80180860->row * 24, 3);
        o->update = ui_obj_nop2;
    } else if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
        SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
        o0->update = func_8009A4FC;
        o1->update = func_8009A64C;
        o2->update = func_8009A708;
        o3->update = func_8009A7DC;
        o4->update = func_80099E30;
        func_800506EC(4);
        obj_find_by_id(3)->kill = 1;
        func_8005C2D0(0x20, D_80180860->top + 92, 0x100, D_80180860->row * 24, 3);
        o->update = func_800978A8;
    }
}
