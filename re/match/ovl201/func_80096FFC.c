#include "ac1/ac1.h"

typedef struct MailView {
    s8 n;               /* +0x00 mails in the list */
    s8 rows;            /* +0x01 visible rows, at most 5 */
    u8 pad2[0x22];
    s16 top;            /* +0x24 list y origin */
} MailView;

typedef struct MailState {
    u8 flags[64];
    u8 cond[64][2];
    u8 count;
    s8 mode;
    u8 padC2[2];
    volatile u8 busy;   /* +0xC4 */
    s8 sel;             /* +0xC5 selected list entry */
    u8 padC6[0xA];
    s8 order[64];       /* +0xD0 */
    u8 pad110[0x80];
    u8 subject[128][32]; /* +0x190 */
    MailView *view;     /* +0x1190 = 0x80180860 */
} MailState;

extern MailState D_8017F6D0;
extern s16 D_801A2318;

void func_80096684(void);
void func_800968C4(void);
void func_800968EC(void);
void func_800972A0(UiObj *o);
void func_8005BA2C(s32 a, s16 b, s32 c, s32 d, s32 e, s32 f, s32 g);

void func_80096FFC(UiObj *o)
{
    s32 i;

    if (obj_find_by_id(3) == &g_ui_none && D_8017F6D0.busy == 0) {
        func_80096684();
        func_800968C4();
        func_800968EC();
        D_8017F6D0.view->n = 0;
        for (i = 63; i != -1; i--) {
            if (D_8017F6D0.order[i] >= 0) {
                D_8017F6D0.view->n++;
            }
        }
        if (D_8017F6D0.view->n >= 6) {
            D_8017F6D0.view->rows = 5;
        } else {
            D_8017F6D0.view->rows = D_8017F6D0.view->n;
        }
        if (D_8017F6D0.view->n < D_8017F6D0.sel) {
            D_8017F6D0.sel = D_8017F6D0.view->n - 1;
        }
        D_8017F6D0.view->top = (5 - D_8017F6D0.view->rows) * 12;
        func_8005BA2C(0x20, D_8017F6D0.view->top + 92, 0x100, D_8017F6D0.view->rows * 24, 3, 0, 0);
        SsUtKeyOn(D_801A2318, 0, 6, 0x3C, 0, 0x7F, 0x7F);
        o->update = func_800972A0;
    }
}
