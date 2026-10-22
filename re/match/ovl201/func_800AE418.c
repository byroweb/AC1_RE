#include "ac1/ac1.h"
#include "ac1/uidialog.h"

typedef struct {
    u8 x;                       /* +0x00 */
    u8 y;                       /* +0x01 */
    u8 unk02[6];
    s32 tpage;                  /* +0x08 tpage bits: x/64, y/256 */
} UiDialogBox;

extern UiDialog D_801A2308;
extern UiDialogBox *D_801A2310;
extern u8 D_8004D5E4[], D_8004D608[], D_8004D62C[], D_8004D650[], D_8004D674[];
extern u8 D_8004D698[], D_8004D6AC[], D_8004D6C4[], D_8004D6D8[], D_8004D6F4[];
extern u8 D_8004D714[], D_8004D734[], D_8004D754[];
extern void func_8005BA2C(s32, s32, s32, s32, s32, s32, s32);
extern void func_8005D15C(u8 *, s16, s16);
extern void func_8005CAE0(UiDialogBox *, s32, s32, s32);
extern void func_8005CB94(u8 *, s32, s32, s32);
extern void func_800AE644(UiObj *);

/* Once the dialog window exists, print its message (kind sel). */
void func_800AE418(UiObj *o) {
    u8 *msg;
    s16 x;
    s16 y;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        x = D_801A2310->x + ((D_801A2310->tpage & 0xF) << 6);
        y = D_801A2310->y + (((D_801A2310->tpage & 0x800) >> 2) + ((D_801A2310->tpage & 0x10) << 4));
        if (D_801A2308.unk01[0] == 0) {
            switch (D_801A2308.unk01[1]) {
            case 3:
                func_8005D15C(D_8004D5E4, x, y);
                break;
            case 4:
                func_8005D15C(D_8004D608, x, y);
                break;
            case 5:
                func_8005D15C(D_8004D62C, x, y);
                break;
            case 6:
                func_8005D15C(D_8004D650, x, y);
                break;
            default:
                func_8005D15C(D_8004D674, x, y);
                break;
            }
            func_8005CAE0(D_801A2310, 8, 16, D_801A2308.sel);
        } else if (D_801A2308.unk01[0] == 1) {
            switch (D_801A2308.unk01[1]) {
            case 2:
                msg = D_8004D698;
                break;
            case 0:
                msg = D_8004D6AC;
                break;
            case 1:
                msg = D_8004D6C4;
                break;
            case 7:
                msg = D_8004D6D8;
                break;
            case 8:
                msg = D_8004D6F4;
                break;
            case 9:
                msg = D_8004D714;
                break;
            case 10:
                msg = D_8004D734;
                break;
            default:
                msg = D_8004D754;
                break;
            }
            func_8005CB94(msg, 8, 16, D_801A2308.sel);
        }
        o->update = func_800AE644;
    }
}
