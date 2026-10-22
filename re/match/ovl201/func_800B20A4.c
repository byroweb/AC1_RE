#include "ac1/ac1.h"

typedef struct RankEnt {
    char *name;                 /* +0x00 */
    char *sub;                  /* +0x04 */
    s32 kind;                   /* +0x08 0xF = the player's row */
    u32 points;                 /* +0x0C sort key */
} RankEnt;

extern RankEnt D_800BDFC8[16];
extern RankEnt D_8017F584[16];
extern UiObj *D_8017F684[3][6];
extern s32 D_80049E70;
extern s32 D_80049E74[];
extern char D_80031BE6[];
extern u8 D_800BDED8[];
extern s32 D_801A2574;
extern UiObj *func_8005CAE0(void *, s32, s32, s32);
extern void func_8005CC48(void);
extern void func_8005DD38(s32, s32, s32);
extern void func_8005DE08(s32, s32, s32);
extern void func_800B2398(void);
extern void func_800B2584(UiObj *);
extern void func_800B26C8(UiObj *);

UiObj *func_800B20A4(void)
{
    UiObj *o;
    s16 i;
    s16 j;
    s16 k;
    RankEnt tmp;

    o = ui_obj_alloc(0);
    o->ctor = func_800B20A4;
    o->update = func_800B26C8;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    func_8005CC48();
    o->w.ring.item[4] = func_8005CAE0(D_800BDED8, 0x10, 0x10, 2);
    func_800B2584(o);
    o->w.raw[0x1D] = 0;
    o->w.raw[0x1C] = 0;
    func_8005DD38(0x380, 0x90, 0x1E);
    func_8005DE08(0x380, 0xA8, 0x1E);
    for (k = 0; k < 6; k++) {
        D_8017F684[0][k] = &g_ui_none;
        D_8017F684[1][k] = &g_ui_none;
        D_8017F684[2][k] = &g_ui_none;
    }
    func_800B2398();
    for (i = 0; i < 15; i++) {
        D_800BDFC8[i].points = D_80049E74[i];
    }
    D_800BDFC8[15].name = D_80031BE6;
    D_800BDFC8[15].sub = D_80031BE6 - 0x12;
    D_800BDFC8[15].points = D_80049E70;
    for (i = 0; i < 16; i++) {
        D_8017F584[i] = D_800BDFC8[i];
    }
    for (i = 0; i < 15; i++) {
        for (j = 15; i < j; j--) {
            if (D_8017F584[j].points > D_8017F584[j - 1].points) {
                tmp = D_8017F584[j];
                D_8017F584[j] = D_8017F584[j - 1];
                D_8017F584[j - 1] = tmp;
            }
        }
    }
    D_801A2574 = 0;
    return o;
}
