#include "ac1/ac1.h"

typedef struct LetterWork {
    u8 pad00[0x24];
    s16 unk24;                  /* +0x24 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

void func_8009A5F8(UiObj *o);
extern u8 D_800B8D30[];
extern s16 D_800B8F84[9];

UiObj *func_8009A508(void)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_8009A508;
    o->update = func_8009A5F8;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 14;
    o->unk14 = 1;
    o->res = D_800B8D30;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = 248;
    o->pos.vy = D_80180860.work->unk24 + 76;
    o->pos.vz = 0;
    D_800B8F84[0] = 32;
    D_800B8F84[1] = 32;
    D_800B8F84[2] = 32;
    D_800B8F84[3] = 128;
    D_800B8F84[4] = 128;
    D_800B8F84[5] = 128;
    D_800B8F84[6] = 256;
    D_800B8F84[7] = 256;
    D_800B8F84[8] = 256;
    return o;
}
