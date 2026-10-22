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

extern s16 D_801A2318;
void func_800513F0(s32 a, u16 b, void *c);
void func_800980E4(u32 *tim);
s32 func_8009A218(void);

void func_80098268(LetterLine *l, u8 *src, s32 more)
{
    u8 *s;
    u8 *d;
    s32 i;
    s32 j;
    s32 pos;
    u8 c;
    s32 n;
    s32 id;

    do {
        pos = l->pos;
        s = src + pos;
        d = l->text + l->len;
        c = *s;
        if (c == '/') {
            for (i = 4; i != -1; i--) {
                g_mail.work->line2[i].pos = 0xFFFF;
            }
            g_mail.work->unk2 = 1;
            g_mail.work->unk3 = 1;
            return;
        }
        if (c == '>') {
            for (j = 4; j != -1; j--) {
                g_mail.work->line2[j].pos = 0xFFFF;
            }
            g_mail.work->unk2 = 1;
            return;
        }
        if (c == '%') {
            n = s[1] - '0';
            l->pos = pos + 2;
            if (g_mail.work->unk9 < n) {
                n = n % 3;
                if ((u32)n < 3) {
                    g_mail.work->unk9++;
                    g_mail.work->unk8 = g_mail.work->unk9;
                    id = g_mail.row[g_mail.order[g_mail.unk1]].reply[g_mail.work->unk9];
                    if (id >= 0) {
                        while (g_mail.work->tim_slot != 0) {
                        }
                        g_mail.work->tim_slot = 3;
                        func_800513F0(19, id + 819, func_800980E4);
                    }
                    if (g_mail.work->seen[n] == 0) {
                        g_mail.work->seen[n] = 1;
                        switch (n) {
                        case 0:
                        case 1:
                        case 2:
                            g_mail.work->unk78[2] = func_8009A218();
                            break;
                        }
                    }
                }
            }
            return;
        }
        if (c == '\r') {
            more = 0;
            l->next = pos + 2;
            l->pos = 0xFFFF;
            l->unk5C = 1;
        } else {
            if (c == '@' || c == '^' || c == '{') {
                l->pos = pos + 1;
                *d++ = *s++;
            }
            l->pos++;
            *d++ = *s;
        }
        l->len = d - l->text;
        *d = '>';
        d[1] = 0;
    } while (more);
    SsUtKeyOn(D_801A2318, 0, 9, 60, 0, 127, 127);
}
