#include "ac1/ac1.h"
typedef struct {
    UiObj *left;        /* +0x7C */
    UiObj *right;       /* +0x80 */
    s16 timer;          /* +0x84 */
} CreditWork;
extern MATRIX D_800B8E44;
extern MATRIX D_800B8F24;
extern u8 D_800BDBBD;
extern s32 func_800517C8(void);
extern UiObj *func_800A4274(u8 kind);
extern void func_800A2EA0(UiObj *o);
extern void func_800A2FE0(UiObj *o);

void func_800A2DBC(UiObj *o)
{
    CreditWork *w = (CreditWork *)&o->w;

    if ((s16)func_800517C8() == 0) {
        if (D_800BDBBD != 0) {
            D_800B8F24 = D_800B8E44;
            w->left = func_800A4274('L');
            w->right = func_800A4274('R');
            w->timer = 0;
        }
        if (D_800BDBBD != 0) {
            o->update = func_800A2EA0;
        } else {
            o->update = func_800A2FE0;
        }
        D_800BDBBD = 0;
    }
}
