#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 pos;                    /* +0x50 read position in the source */
    s32 unk54;                  /* +0x54 */
    s32 len;                    /* +0x58 write position in text */
    s32 unk5C;                  /* +0x5C */
} LetterLine;

typedef struct LetterWork {
    u8 pad00[2];
    u8 unk2;                    /* +0x02 */
    u8 unk3;                    /* +0x03 */
    u8 pad04[0x4C];
    UiObj *label[2];            /* +0x50 */
    u8 pad58[0x2C];
    LetterLine line[5];         /* +0x84 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

void func_80097A08(LetterLine *l, u8 *src, s32 more)
{
    u8 *s;
    u8 *d;
    s32 i;
    s32 j;
    s32 pos;
    int c;

    do {
        pos = l->pos;
        s = src + pos;
        c = *s;
        d = l->text + l->len;
        if (c == '/') {
            for (i = 1; i != -1; i--) {
                D_80180860.work->line[i].pos = 0xFFFF;
            }
            D_80180860.work->unk2 = 1;
            D_80180860.work->unk3 = 1;
            return;
        }
        if (c == '>') {
            for (j = 1; j != -1; j--) {
                D_80180860.work->line[j].pos = 0xFFFF;
            }
            if (c) {   /* both arms equal: an extra use of c that the retail register choice (c=a3, d=v1) needs */
                D_80180860.work->unk2 = 1;
                return;
            } else {
                D_80180860.work->unk2 = 1;
                return;
            }
        }
        if (c == '\r') {
            more = 0;
            l->unk54 = pos + 2;
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
}
