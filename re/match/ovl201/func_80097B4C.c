#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 unk50;                  /* +0x50 */
    s32 unk54;                  /* +0x54 */
    s32 unk58;                  /* +0x58 */
    s32 unk5C;                  /* +0x5C */
} LetterLine;

typedef struct LetterWork {
    u8 pad00[0x50];
    UiObj *label[2];            /* +0x50 */
    u8 pad58[0x2C];
    LetterLine line[5];         /* +0x84 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

UiObj *func_8005CB94(u8 *s, s32 x, s16 y, s32 c);
void func_80097C40(UiObj *o);
void func_8009795C(s32 i);

UiObj *func_80097B4C(void)
{
    UiObj *o;
    s32 i;

    o = ui_obj_alloc(6);
    o->ctor = func_80097B4C;
    o->update = func_80097C40;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    for (i = 1; i != -1; i--) {
        D_80180860.work->label[i] = func_8005CB94(D_80180860.work->line[i].text, 8, i * 16 + 5, 12);
        D_80180860.work->label[i]->light = 12;
        func_8009795C(i);
    }
    return o;
}
