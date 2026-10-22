#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 pos;                    /* +0x50 read position, 0xFFFF = line done */
    s32 next;                   /* +0x54 */
    s32 len;                    /* +0x58 */
    s32 unk5C;                  /* +0x5C */
} LetterLine;

typedef struct LetterWork {
    u8 pad00[2];
    u8 unk2;                    /* +0x02 */
    u8 unk3;                    /* +0x03 */
    u8 unk4;                    /* +0x04 */
    u8 unk5;                    /* +0x05 */
    u8 unk6;                    /* +0x06 */
    s8 unk7;                    /* +0x07 */
    s8 unk8;                    /* +0x08 */
    s8 unk9;                    /* +0x09 */
    u8 unkA;                    /* +0x0A */
    volatile s8 tim_slot;       /* +0x0B */
    u8 pad0C;
    u8 unkD;                    /* +0x0D */
    u8 unkE;                    /* +0x0E */
    u8 unkF;                    /* +0x0F */
    u8 pad10[0x16];
    s16 unk26;                  /* +0x26 */
    u8 pad28[0x28];
    UiObj *label[2];            /* +0x50 */
    u8 pad58[0x0C];
    UiObj *label2[5];           /* +0x64 */
    s32 unk78[3];               /* +0x78 */
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
    s8 unk1;                    /* +0x001 selected mail */
    s8 unk2;                    /* +0x002 */
    u8 pad003[9];
    s8 order[0x8C0];            /* +0x00C (0x8017F7A0) */
    MailMsgRow row[64];         /* +0x8CC (0x80180060) */
    LetterWork *work;           /* +0x10CC (0x80180860) */
} MailGlobals;
extern MailGlobals g_mail;      /* 0x8017F794 */

extern s16 D_800B8F44;
extern s16 D_800B8F46;
extern s16 D_800B8F48;
extern s16 D_800B8F4A;
extern s16 D_800B8F4C;
extern s16 D_800B8F4E;
extern s16 D_800B8F50;
extern s16 D_800B8F52;
extern s16 D_800B8F54;
extern RECT D_8004CE68;

UiObj *func_8005CB94(u8 *s, s32 x, s16 y, s32 c);
void func_80098968(UiObj *o);
void func_800981BC(s32 i);
void func_80015538(void *p, s32 v, s32 n);
void func_800513F0(s32 a, u16 b, void *c);
void func_80097F30(u8 *src);
int ClearImage(RECT *rect, u8 r, u8 g, u8 b);

UiObj *func_80098550(void)
{
    UiObj *o;
    s32 i;
    s32 j;
    s32 msg;
    RECT rect;

    o = ui_obj_alloc(6);
    o->ctor = func_80098550;
    o->update = func_80098968;
    o->destroy = ui_obj_nop;
    o->visible = 0;
    o->attached = 0;
    o->parent = 0;
    o->has_xform = 0;
    for (i = 4; i != -1; i--) {
        g_mail.work->label2[i] = func_8005CB94(g_mail.work->line2[i].text, 8, i * 16 + 5, 6);
        g_mail.work->label2[i]->light = 12;
        func_800981BC(i);
    }
    D_800B8F44 = 128;
    D_800B8F46 = 128;
    D_800B8F48 = 128;
    D_800B8F4A = 128;
    D_800B8F4C = 128;
    D_800B8F4E = 128;
    D_800B8F50 = 128;
    D_800B8F52 = 128;
    D_800B8F54 = 128;
    g_mail.work->unkD = 0;
    g_mail.work->unk26 = 0;
    g_mail.work->unk2 = 0;
    g_mail.work->unk3 = 0;
    g_mail.work->unk4 = 0;
    g_mail.work->unk5 = 0;
    g_mail.work->unk6 = 0;
    g_mail.work->unk7 = 0;
    g_mail.work->unkA = 0;
    g_mail.work->unkE = 0;
    g_mail.work->unkF = 0;
    for (j = 2; j != -1; j--) {
        g_mail.work->seen[j] = 0;
        g_mail.work->unk78[j] = 0;
    }
    g_mail.work->unk9 = -1;
    g_mail.work->unk8 = -1;
    g_mail.work->tim_slot = 0;
    func_80015538(g_mail.work->text, 0, 19);
    func_80015538(g_mail.work->text2, 0, 256);
    msg = g_mail.row[g_mail.order[g_mail.unk1]].id[g_mail.work->unk7];
    if (msg >= 0) {
        g_mail.work->unkA = 1;
        func_800513F0(19, msg + 819, func_80097F30);
    } else {
        g_mail.work->text[0] = '/';
        g_mail.work->text[1] = 0;
        g_mail.work->text2[0] = '/';
        g_mail.work->text2[1] = 0;
    }
    rect = D_8004CE68;
    rect.x = 896;
    rect.y = 0;
    ClearImage(&rect, 255, 255, 255);
    rect.x = 960;
    rect.y = 0;
    ClearImage(&rect, 255, 255, 255);
    rect.x = 896;
    rect.y = 128;
    ClearImage(&rect, 255, 255, 255);
    rect.w = 256;
    rect.h = 1;
    rect.x = 0;
    rect.y = 481;
    ClearImage(&rect, 0, 0, 0);
    rect.x = 0;
    rect.y = 482;
    ClearImage(&rect, 0, 0, 0);
    rect.x = 0;
    rect.y = 483;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    return o;
}
