#include "ac1/ac1.h"

typedef struct LetterWork {
    u8 pad00[0x0A];
    u8 unkA;                    /* +0x0A */
    u8 pad0B[0x515];
    u8 text[1];                 /* +0x520 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

extern char D_8004CE60[];

void func_80097F30(u8 *src)
{
    u8 *s;
    u8 *d;

    s = src;
    d = D_80180860.work->text;
    printf(D_8004CE60);
    while (*s != '>') {
        if (*s == '/') {
            break;
        }
        *d++ = *s++;
    }
    *d++ = *s;
    *d = 0;
    free2(src);
    D_80180860.work->unkA = 0;
}
