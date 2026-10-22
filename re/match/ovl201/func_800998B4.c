#include "ac1/ac1.h"

/* g_mail.work as the mail list screen sees it */
typedef struct MailList {
    s8 count;                   /* +0x00 mails in the list */
    s8 rows;                    /* +0x01 rows on screen */
    u8 pad02[0x26];
    UiObj *label[5];            /* +0x28 row name labels */
    UiObj *icon[5];             /* +0x3C row icons */
    u8 pad50[0x3F7];
    char name[5][34];           /* +0x447 row texts */
} MailList;

typedef struct MailGlobals {
    s8 unk0;                    /* +0x000 (0x8017F794) */
    s8 sel;                     /* +0x001 selected mail */
    s8 top;                     /* +0x002 */
    u8 pad003[9];
    s8 order[0x8C0];            /* +0x00C (0x8017F7A0) */
    u8 row[64][32];             /* +0x8CC (0x80180060) */
    MailList *work;             /* +0x10CC (0x80180860) */
} MailGlobals;
extern MailGlobals g_mail;      /* 0x8017F794 */

extern u8 D_8017F6D0[];
extern s16 D_801A2318;
extern u32 D_801A2564;
extern u32 D_801A2568;
extern u32 D_801A256C;
extern s32 D_800B8CC8;
extern s32 D_800B8CCC;
void func_80099868(char *dst, char *src);
void func_8009720C(u8 id, u8 row);
void func_800971AC(u8 id, u8 row);
void func_80096E60(void);

void func_800998B4(UiObj *o)
{
    s32 old;
    s32 i;

    old = g_mail.sel;
    if ((D_801A2568 & 0x1000) && !(D_801A256C & 0x1000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (--g_mail.sel < 0) {
            if (g_mail.work->count < 6) {
                g_mail.sel += g_mail.work->count;
            } else {
                g_mail.sel = 0;
            }
        }
        if (o->pos.vy <= 0) {
            if (g_mail.work->count < 6) {
            o->pos.vy = (g_mail.work->count - 1) * 24;
        } else if (old != g_mail.sel) {
            i = g_mail.work->rows;
            while (--i != 0) {
                func_80099868(g_mail.work->name[i], g_mail.work->name[i - 1]);
                g_mail.work->icon[i]->visible = g_mail.work->icon[i - 1]->visible;
                g_mail.work->label[i]->light = g_mail.work->label[i - 1]->light;
            }
            func_8009720C(g_mail.order[g_mail.sel], 0);
            func_800971AC(g_mail.order[g_mail.sel], 0);
            if (D_8017F6D0[g_mail.order[g_mail.sel]] & 2) {
                g_mail.work->label[0]->light = 20;
            } else {
                g_mail.work->label[0]->light = 12;
            }
            g_mail.top--;
            func_80096E60();
            }
        } else {
            o->pos.vy -= 24;
        }
    } else if ((D_801A2568 & 0x4000) && !(D_801A256C & 0x4000)) {
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (++g_mail.sel > g_mail.work->count - 1) {
            if (g_mail.work->count < 6) {
                g_mail.sel -= g_mail.work->count;
            } else {
                g_mail.sel = g_mail.work->count - 1;
            }
        }
        if (o->pos.vy >= (g_mail.work->rows - 1) * 24) {
            if (g_mail.work->count < 6) {
            o->pos.vy = 0;
        } else if (old != g_mail.sel) {
            for (i = 1; i < g_mail.work->rows; i++) {
                func_80099868(g_mail.work->name[i - 1], g_mail.work->name[i]);
                g_mail.work->icon[i - 1]->visible = g_mail.work->icon[i]->visible;
                g_mail.work->label[i - 1]->light = g_mail.work->label[i]->light;
            }
            func_8009720C(g_mail.order[g_mail.sel], g_mail.work->rows - 1);
            func_800971AC(g_mail.order[g_mail.sel], g_mail.work->rows - 1);
            if (D_8017F6D0[g_mail.order[g_mail.sel]] & 2) {
                g_mail.work->label[g_mail.work->rows - 1]->light = 20;
            } else {
                g_mail.work->label[g_mail.work->rows - 1]->light = 12;
            }
            g_mail.top++;
            func_80096E60();
            }
        } else {
            o->pos.vy += 24;
        }
    }
    if (!(D_801A2564 & 0x5000)) {
        D_800B8CC8 = 0;
    } else {
        D_800B8CC8++;
    }
    if (D_800B8CC8 >= 20) {
        if (++D_800B8CCC >= 5) {
            D_800B8CCC = 0;
            D_801A2564 = 0;
        }
    }
}
