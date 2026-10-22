#include "ac1/ac1.h"

typedef struct LetterWork {
    u8 pad00[0x24];
    s16 unk24;                  /* +0x24 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

void func_8009A4A8(UiObj *o);
extern u8 D_800B8D24[];
extern s16 D_800B9024[9];

UiObj *func_8009A3B8(void)
{
    UiObj *o;

    o = ui_obj_alloc(2);
    o->ctor = func_8009A3B8;
    o->update = func_8009A4A8;
    o->destroy = ui_obj_nop;
    o->render_type = 3;
    o->light = 19;
    o->unk14 = 1;
    o->res = D_800B8D24;
    o->has_xform = 0;
    o->rot.vx = 0;
    o->rot.vy = 0;
    o->rot.vz = 0;
    o->pos.vx = 40;
    o->pos.vy = D_80180860.work->unk24 + 76;
    o->pos.vz = 0;
    D_800B9024[0] = 256;
    D_800B9024[1] = 256;
    D_800B9024[2] = 256;
    D_800B9024[3] = 128;
    D_800B9024[4] = 128;
    D_800B9024[5] = 128;
    D_800B9024[6] = 32;
    D_800B9024[7] = 32;
    D_800B9024[8] = 32;
    return o;
}
