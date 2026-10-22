#include "ac1/ac1.h"

typedef struct MailLine {
    u8 x;
    u8 y;
    u8 str[32];
} MailLine;

typedef struct MailView {
    s8 n;               /* +0x00 mails in the list */
    s8 rows;            /* +0x01 visible rows, at most 5 */
    u8 pad2[0x22];
    s16 top;            /* +0x24 list y origin */
    u8 pad26[2];
    UiObj *label[5];    /* +0x28 subject text objects */
    UiObj *mark[5];     /* +0x3C read/unread marks */
    u8 pad50[0x3F7];
    MailLine lines[5];  /* +0x447 */
} MailView;

typedef struct MailState {
    u8 flags[64];
    u8 cond[64][2];
    u8 count;
    s8 mode;
    u8 padC2[2];
    volatile u8 busy;   /* +0xC4 */
    s8 sel;             /* +0xC5 selected list entry */
    s8 scroll;          /* +0xC6 first entry on the page */
    u8 padC7[9];
    u8 order[64];       /* +0xD0 */
    u8 pad110[0x80];
    u8 subject[128][32]; /* +0x190 */
    MailView *view;     /* +0x1190 = 0x80180860 */
} MailState;

extern MailState D_8017F6D0;
extern s16 D_800B8F44[];
extern s16 D_800B9044[];
extern u8 D_800B8CD0[];
extern u8 D_800B8D54[];

s16 func_800517C8(void);
void func_8009A3B8(void);
void func_8009A508(void);
void func_80096E60(void);
void func_800997C8(void);
void func_8005BA2C(s32 a, s16 b, s32 c, s32 d, s32 e, s32 f, s32 g);
UiObj *func_8005CAE0(void *res, s32 x, s16 y, s32 z);
UiObj *func_8005CB94(void *str, s32 x, s16 y, s32 z);
void func_800971AC(u8 mail, u8 slot);
void func_8009720C(u8 mail, u8 slot);
void func_8009760C(UiObj *o);

void func_800972A0(UiObj *o)
{
    s32 i;
    s16 m;
    u8 m2;

    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        while (func_800517C8() != 0) {
        }
        func_8009A3B8();
        func_8009A508();
        func_80096E60();
        D_800B8F44[0] = 0x80;
        D_800B8F44[1] = 0x80;
        D_800B8F44[2] = 0x80;
        D_800B8F44[3] = 0x80;
        D_800B8F44[4] = 0x80;
        D_800B8F44[5] = 0x80;
        D_800B8F44[6] = 0x80;
        D_800B8F44[7] = 0x80;
        D_800B8F44[8] = 0x80;
        D_800B9044[0] = 0x20;
        D_800B9044[1] = 0x20;
        D_800B9044[2] = 0x20;
        D_800B9044[3] = 0x20;
        D_800B9044[4] = 0x20;
        D_800B9044[5] = 0x20;
        D_800B9044[6] = 0x20;
        D_800B9044[7] = 0x20;
        D_800B9044[8] = 0x20;
        func_800997C8();
        i = D_8017F6D0.view->rows;
        while (--i != -1) {
            m = (i + D_8017F6D0.scroll) % D_8017F6D0.view->n;
            D_8017F6D0.view->mark[i] = func_8005CAE0(D_800B8D54, 0xE0, i * 24 + 5, 4);
            func_800971AC(D_8017F6D0.order[(u8)m], i);
        }
        DrawSync(0);
        i = D_8017F6D0.view->rows;
        while (--i != -1) {
            m2 = (i + D_8017F6D0.scroll) % D_8017F6D0.view->n;
            func_8009720C(D_8017F6D0.order[m2], i);
            D_8017F6D0.view->label[i] = func_8005CB94(&D_8017F6D0.view->lines[i], 8, i * 24 + 5, 4);
            if (D_8017F6D0.flags[(s8)D_8017F6D0.order[m2]] & 2) {
                D_8017F6D0.view->label[i]->light = 20;
            } else {
                D_8017F6D0.view->label[i]->light = 12;
            }
        }
        i = D_8017F6D0.view->rows;
        while (--i != -1) {
            func_8005CAE0(D_800B8CD0, 0, i * 24, 4);
        }
        o->update = func_8009760C;
    }
}
