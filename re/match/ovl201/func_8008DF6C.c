#include "ac1/ac1.h"
#include "ac1/gfxframe.h"
/* ovl201 0x8008DF6C - shop category list setup (once no func_8005BA2C object
 * is left): wrap the selected category, draw the four visible category rows,
 * make their labels and cursors, set up both draw environments centred on
 * 320x240, and make the two arrow objects at +0x7C / +0x80. */

typedef struct {
    u8 pad00[2];
    s8 kind;            /* +0x02 selected category */
    s8 first;           /* +0x03 category in the top row */
    u8 pad04[0xC0];
    UiObj *cursor[4];   /* +0xC4 */
    u8 padD4[0x14];
    UiObj *label[4];    /* +0xE8 */
} ShopState;
extern ShopState D_8019C090;
extern DRAWENV D_801BD1A8;
extern DRAWENV D_801D6600;
extern u8 D_800B8B90[];
extern u8 D_800B8BFC[][12];
extern u8 D_800B91AC[];
extern u8 D_800B91B8[];
extern void func_8005BA2C(void);
extern void func_800953AC(void);
extern void func_8008DE30(u8 kind, u8 row);
extern UiObj *func_8005CAE0(void *res, s32 x, s32 y, s32 z);
extern void func_8008E260(UiObj *o);

void func_8008DF6C(UiObj *o)
{
    s32 i;
    UiObj *l;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        func_800953AC();
        D_8019C090.kind %= 10;
        for (i = 3; i != -1; i--) {
            func_8008DE30((i + D_8019C090.first) % 10, i);
        }
        for (i = 3; i != -1; i--) {
            D_8019C090.label[i] = func_8005CAE0(D_800B8BFC[i], 8, (s16)(i * 24 + 5), 6);
            D_8019C090.label[i]->light = 20;
        }
        DrawSync(0);
        for (i = 3; i != -1; i--) {
            D_8019C090.cursor[i] = func_8005CAE0(D_800B8B90, 0, (s16)(i * 24), 6);
        }
        o->update = func_8008E260;
        SetDefDrawEnv(&D_801BD1A8, 0, 0, 320, 240);
        SetDefDrawEnv(&D_801D6600, 0, 240, 320, 240);
        D_801BD1A8.ofs[0] = D_801BD1A8.clip.x + (D_801BD1A8.clip.w - 320) / 2;
        D_801BD1A8.ofs[1] = D_801BD1A8.clip.y + (D_801BD1A8.clip.h - 240) / 2;
        D_801D6600.ofs[0] = D_801D6600.clip.x + (D_801D6600.clip.w - 320) / 2;
        D_801D6600.ofs[1] = D_801D6600.clip.y + (D_801D6600.clip.h - 240) / 2;
        o->w.list.src = l = func_8005CAE0(D_800B91AC, 152, 96, 18);
        l->light = 11;
        ((UiObj *)o->w.list.src)->visible = 0;
        o->w.ring.item[0] = l = func_8005CAE0(D_800B91B8, 152, 168, 18);
        l->light = 11;
        o->w.ring.item[0]->visible = 0;
    }
}
