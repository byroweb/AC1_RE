#include "ac1/ac1.h"

typedef struct MailLine {
    u8 x;
    u8 y;
    u8 str[32];
} MailLine;

typedef struct MailView {
    u8 pad0[0x447];
    MailLine lines[1];
} MailView;

typedef struct MailState {
    u8 flags[64];
    u8 pad40[0x80];
    s8 count;
    u8 padC1[0xF];
    s8 order[64];
    u8 pad110[0x80];
    u8 subject[128][32];
    MailView *view;
} MailState;

extern MailState D_8017F6D0;

void func_8009720C(u8 mail, u8 slot)
{
    u8 *src;
    u8 *dst;

    D_8017F6D0.view->lines[slot].x = 0x40;
    D_8017F6D0.view->lines[slot].y = 0x30;
    dst = D_8017F6D0.view->lines[slot].str;
    src = D_8017F6D0.subject[mail];
    while (*src != '>') {
        *dst++ = *src++;
    }
    dst[0] = '>';
    dst[1] = 0;
}
