#include "ac1/ac1.h"
/* ovl201 0x8009A658 - mail list: build the up-arrow sprite above the list */
typedef struct MailView {
    s8 n;
    s8 rows;
    u8 pad02[0x22];
    s16 top;            /* +0x24 */
} MailView;
typedef struct MailState {
    u8 pad0[0x1190];
    MailView *view;     /* +0x1190 = 0x80180860 */
} MailState;
extern MailState D_8017F6D0;
extern u8 D_800B8D3C[];
extern void func_8009A6F0(UiObj *o);

void func_8009A658(void)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_8009A658;
    o->update = func_8009A6F0;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 0xB;
    o->unk14 = 1;
    o->res = D_800B8D3C;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = 0x120;
    o->pos.vy = D_8017F6D0.view->top + 0x5C;
    o->pos.vz = 0;
}
