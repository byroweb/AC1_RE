#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 pos;                    /* +0x50 read position, 0xFFFF = line done */
    s32 next;                   /* +0x54 source offset of the next line */
    s32 len;                    /* +0x58 */
    s32 ended;                  /* +0x5C line ended with a line break */
} LetterLine;

typedef struct LetterWork {
    u8 pad00[2];
    s8 unk2;                    /* +0x02 */
    s8 unk3;                    /* +0x03 */
    s8 unk4;                    /* +0x04 scroll-down steps left */
    s8 unk5;                    /* +0x05 scroll-up steps left */
    s8 unk6;                    /* +0x06 */
    s8 unk7;                    /* +0x07 */
    s8 unk8;                    /* +0x08 */
    s8 unk9;                    /* +0x09 */
    u8 unkA;                    /* +0x0A */
    volatile s8 tim_slot;       /* +0x0B */
    u8 unkC;                    /* +0x0C */
    s8 unkD;                    /* +0x0D */
    u8 unkE;                    /* +0x0E */
    u8 unkF;                    /* +0x0F */
    u16 hist[10];               /* +0x10 */
    u8 pad24[2];
    s16 unk26;                  /* +0x26 */
    u8 pad28[0x28];
    UiObj *label[2];            /* +0x50 */
    u8 pad58[0x0C];
    UiObj *label2[5];           /* +0x64 */
    UiObj *button[3];           /* +0x78 */
    LetterLine line[5];         /* +0x84 */
    LetterLine line2[5];        /* +0x264 */
    u8 seen[3];                 /* +0x444 */
    u8 pad447[0xD9];
    u8 text[0x4C];              /* +0x520 */
    u8 text2[0x100];            /* +0x56C */
} LetterWork;

typedef struct MailMsgRow {
    s16 reply[8];               /* +0x00 */
    s16 id[8];                  /* +0x10 letter message ids, -1 = none */
} MailMsgRow;

typedef struct MailGlobals {
    s8 unk0;                    /* +0x000 (0x8017F794) */
    s8 sel;                     /* +0x001 selected mail */
    s8 top;                     /* +0x002 */
    u8 pad003[9];
    s8 order[0x8C0];            /* +0x00C (0x8017F7A0) */
    MailMsgRow row[64];         /* +0x8CC (0x80180060) */
    LetterWork *work;           /* +0x10CC (0x80180860) */
} MailGlobals;
extern MailGlobals g_mail;      /* 0x8017F794 */

extern s16 D_801A2318;
extern u32 D_801A2568;
extern u32 D_801A256C;
void func_80099E3C(UiObj *o);
void func_8005BA2C();
void func_800981BC(s32 i);
void func_80098268(LetterLine *l, u8 *src, s32 more);
void func_800513F0(s32 a, u16 b, void *c);
void func_80097F30(u8 *src);
void func_800980E4(u32 *tim);
void func_800506EC(s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_80099ECC(UiObj *o);
void func_800996E4(UiObj *o);
void func_80096944(UiObj *o);
void func_80096FFC(UiObj *o);
void func_8009A030(UiObj *o);
void func_8009A1D0(UiObj *o);
void func_8009A370(UiObj *o);
void func_8009A218(UiObj *o);

#define PRESSED(b) ((D_801A2568 & (b)) && !(D_801A256C & (b)))

void func_80098A58(UiObj *o)
{
    s16 a;
    s16 b;
    UiObj *box;
    u8 *src;
    s32 i;
    s32 fast;
    s32 msg;

    a = 0;
    b = 0;
    box = obj_find_by_handle(func_80099E3C);
    for (i = 4; i != -1; i--) {
        a += g_mail.work->line2[i].next;
    }
    src = g_mail.work->text2 + g_mail.work->unk26 + a;
    if (obj_find_by_handle(func_8005BA2C) != &g_ui_none) {
        return;
    }
    if (g_mail.work->unk4 > 0) {
        g_mail.work->unk4--;
        g_mail.work->unk26 += g_mail.work->line2[0].next;
        for (i = 0; i < 4; i++) {
            g_mail.work->line2[i] = g_mail.work->line2[i + 1];
        }
        func_800981BC(4);
        if (g_mail.work->unk4 == 0 && g_mail.work->unkD != 0) {
            g_mail.work->unk26 = 0;
            g_mail.work->unkD = 0;
        }
        return;
    }
    if (g_mail.work->unk5 > 0) {
        g_mail.work->unk5--;
        for (i = 3; i != -1; i--) {
            g_mail.work->line2[i + 1] = g_mail.work->line2[i];
        }
        func_800981BC(0);
        if (g_mail.work->unk5 == 0 && g_mail.work->unkD != 0) {
            g_mail.work->unk26 = 0;
            g_mail.work->unkD = 0;
        }
        return;
    }
    if (g_mail.work->unkA == 0) {
        fast = 0;
        if (g_mail.work->unk3 != 0) {
            fast = 1;
        } else if ((D_801A2568 & 0x40) || (D_801A256C & 0x40) || (D_801A2568 & 0x1000) ||
                   (D_801A256C & 0x1000) || (D_801A2568 & 0x4000) || (D_801A256C & 0x4000)) {
            fast = 1;
        }
        for (i = 0; i < 5; i++) {
            if (g_mail.work->line2[i].pos != 0xFFFF) {
                func_80098268(&g_mail.work->line2[i], src, fast);
                break;
            }
        }
    }
    g_mail.work->unkC++;
    for (i = 4; i != -1; i--) {
        b += g_mail.work->line2[i].next;
    }
    if (g_mail.work->line2[4].ended != 0 || g_mail.work->unk2 != 0) {
        if (g_mail.work->unk3 == 0 || g_mail.work->unk6 < g_mail.work->unk7) {
            if (box->visible == 0) {
                box->visible = 1;
            }
        }
    }
    if (PRESSED(0x1000)) {
        if (g_mail.work->unk2 != 0) {
            if (g_mail.work->unk6 > 0) {
                g_mail.work->unk6--;
                msg = g_mail.row[g_mail.order[g_mail.sel]].id[g_mail.work->unk6];
                if (msg >= 0) {
                    g_mail.work->unkA = 1;
                    func_800513F0(19, msg + 819, func_80097F30);
                }
                g_mail.work->unk2 = 0;
                g_mail.work->unkD = 1;
            } else {
                if (g_mail.work->unk26 == 0 || a != b) {
                    return;
                }
                g_mail.work->unkE--;
                g_mail.work->unk2 = 0;
                g_mail.work->unk26 -= g_mail.work->hist[g_mail.work->unkE];
            }
        } else {
            if (g_mail.work->line2[4].ended == 0 || g_mail.work->unk26 == 0 || a != b) {
                return;
            }
            g_mail.work->unkE--;
            g_mail.work->unk26 -= g_mail.work->hist[g_mail.work->unkE];
        }
        g_mail.work->unk5 = 5;
        box->visible = 0;
    } else if (PRESSED(0x4000) && g_mail.work->unk3 != 0) {
        if (g_mail.work->unk2 != 0) {
            if (g_mail.work->unk6 >= g_mail.work->unk7) {
                return;
            }
            g_mail.work->unk6++;
        send:
            msg = g_mail.row[g_mail.order[g_mail.sel]].id[g_mail.work->unk6];
            if (msg >= 0) {
                g_mail.work->unkA = 1;
                func_800513F0(19, msg + 819, func_80097F30);
            }
            g_mail.work->unk2 = 0;
            g_mail.work->unkD = 1;
        } else {
            if (g_mail.work->line2[4].ended == 0 || a != b) {
                return;
            }
            g_mail.work->hist[g_mail.work->unkE] = a;
            g_mail.work->unkE++;
            if (g_mail.work->unkF < g_mail.work->unkE) {
                g_mail.work->unkF = g_mail.work->unkE;
            }
        }
        g_mail.work->unk4 = 5;
        box->visible = 0;
    } else if (PRESSED(0x40) || PRESSED(0x4000)) {
        if (g_mail.work->unk3 != 0) {
            return;
        }
        if (g_mail.work->unk2 != 0) {
            if (g_mail.work->unk6 < g_mail.work->unk7) {
                g_mail.work->unk6++;
            } else {
                g_mail.work->unk6++;
                g_mail.work->unk7++;
            }
            goto send;
        } else {
            if (g_mail.work->line2[4].ended == 0 || a != b) {
                return;
            }
            g_mail.work->hist[g_mail.work->unkE] = a;
            g_mail.work->unkE++;
            if (g_mail.work->unkF < g_mail.work->unkE) {
                g_mail.work->unkF = g_mail.work->unkE;
            }
        }
        g_mail.work->unk4 = 5;
        box->visible = 0;
    } else if (PRESSED(0x20)) {
        box->update = func_80099ECC;
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_8005C2D0(8, 128, 304, 96, 5);
        o->update = func_800996E4;
        obj_find_by_handle(func_80096944)->update = func_80096FFC;
        if (g_mail.work->button[0] != 0) {
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(4, 22, 104, 100, 7);
            g_mail.work->button[0]->update = func_8009A030;
        }
        if (g_mail.work->button[1] != 0) {
            func_800506EC(10);
            obj_find_by_id(9)->kill = 1;
            func_8005C2D0(108, 22, 104, 100, 9);
            g_mail.work->button[1]->update = func_8009A1D0;
        }
        if (g_mail.work->button[2] != 0) {
            func_800506EC(8);
            obj_find_by_id(7)->kill = 1;
            func_8005C2D0(212, 22, 104, 100, 7);
            g_mail.work->button[2]->update = func_8009A370;
        }
    } else if (PRESSED(0x8)) {
        if (obj_find_by_handle(func_8009A218) == &g_ui_none) {
            return;
        }
        if (g_mail.work->unk9 < 2) {
            return;
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (g_mail.work->unk8 < 0) {
            return;
        }
        g_mail.work->unk8++;
        if (g_mail.work->unk9 < g_mail.work->unk8) {
            g_mail.work->unk8 = g_mail.work->unk9;
        }
        msg = g_mail.row[g_mail.order[g_mail.sel]].reply[g_mail.work->unk8];
        if (msg < 0) {
            return;
        }
        while (g_mail.work->tim_slot != 0) {
        }
        g_mail.work->tim_slot = 3;
        func_800513F0(19, msg + 819, func_800980E4);
    } else if (PRESSED(0x4)) {
        if (obj_find_by_handle(func_8009A218) == &g_ui_none) {
            return;
        }
        if (g_mail.work->unk9 < 2) {
            return;
        }
        SsUtKeyOn(D_801A2318, 0, 1, 60, 0, 127, 127);
        if (g_mail.work->unk8 < 0) {
            return;
        }
        g_mail.work->unk8--;
        if (g_mail.work->unk8 < 0) {
            g_mail.work->unk8 = 0;
        }
        msg = g_mail.row[g_mail.order[g_mail.sel]].reply[g_mail.work->unk8];
        if (msg < 0) {
            return;
        }
        while (g_mail.work->tim_slot != 0) {
        }
        g_mail.work->tim_slot = 3;
        func_800513F0(19, msg + 819, func_800980E4);
    }
}
