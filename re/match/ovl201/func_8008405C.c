#include "ac1/ac1.h"
/* ovl201 0x8008405C - build the 11-entry picker object (rows of icons / labels from
 * the unlock mask at D_800485F0, two titles) */
typedef struct PickWork {
    void *src;                  /* +0x7C */
    s32 arg;                    /* +0x80 */
    UiObj *title[2];            /* +0x84 */
    u8 unk8C[4];
    UiObj *icon[6];             /* +0x90 */
    UiObj *label[5];            /* +0xA8 */
    UiObj *cursor;              /* +0xBC */
    u8 pad[4];
    u8 count;                   /* +0xC4 */
    u8 total;                   /* +0xC5 */
    u8 pos;                     /* +0xC6 */
    u8 unkC7;
    u8 slot;                    /* +0xC8 */
    u8 sel;                     /* +0xC9 */
    u8 unkCA;
    u8 unkCB;
    u8 unkCC;
} PickWork;
extern u8 D_800B91AC[];
extern u8 D_800B91B8[];
extern s32 D_801A2574;
UiObj *func_8005CAE0(u8 *s, s32 x, s32 y, s32 c);
u8 func_800842D4(void);
u8 func_80084290(void);
void func_800843A8(void);
void func_80084474(UiObj *o);
void func_80084758(UiObj *o);
void func_80084884(UiObj *o);
void func_80084A8C(UiObj *o);

UiObj *func_8008405C(void *src, s32 arg)
{
    UiObj *o;
    PickWork *w;
    s16 i;

    o = ui_obj_alloc(0);
    w = (PickWork *)&o->w;
    o->ctor = func_8008405C;
    o->update = func_80084758;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    for (i = 0; i < 5; i++) {
        w->icon[i] = &g_ui_none;
        w->label[i] = &g_ui_none;
    }
    w->total = func_800842D4();
    w->src = src;
    w->arg = arg;
    w->count = 0;
    w->pos = func_80084290();
    w->slot = 1;
    w->sel = 0xFF;
    w->unkC7 = 0;
    w->unkCA = 0;
    D_801A2574 = 2;
    func_800843A8();
    func_80084474(o);
    func_80084A8C(o);
    func_80084884(o);
    w->title[0] = func_8005CAE0(D_800B91AC, 14, 48, 2);
    w->title[0]->light = 11;
    w->title[0]->visible = 0;
    w->title[1] = func_8005CAE0(D_800B91B8, 14, 184, 2);
    w->title[1]->light = 11;
    if (func_800842D4() < 6) {
        w->title[1]->visible = 0;
    }
    return o;
}
