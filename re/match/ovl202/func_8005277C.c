#include "ac1/ac1.h"

/* ovl202 0x8005277C - parse a message box header (Mn lines, Lx,y position, Ww,h size,
 * Cn, Pn; '/' = nothing), measure the text that follows (6 px ASCII, 12 px SJIS,
 * 12 px lines; F/f ends) and open the box centred on it (func_80052650). */

extern s32 func_80052350(u8 **pp);
extern void func_80052650(s32 x, s32 y, s32 w, s32 h, s32 tx, s32 ty, s32 c, s32 one,
                          s32 a0, s32 a1, u8 *text);

void func_8005277C(s32 a0, s32 a1, u8 *p)
{
    s32 x;
    s32 y;
    s32 w;
    s32 h;
    s32 c;
    s32 one;
    s32 n;
    u8 *q;
    u8 **pp;
    u8 ch;
    u8 k;
    s32 lw;
    s32 maxw;
    s32 th;

    x = 50;
    y = 180;
    w = 220;
    h = 20;
    c = 0;
    one = 1;
    pp = &p;
loop:
    {
        ch = *p;
        p++;
        switch (ch) {
        case 'M':
        case 'm':
            n = (func_80052350(&p) - 1) * 12;
            y = 180 - n;
            h = n + 20;
            goto loop;
        case 'L':
        case 'l':
            x = func_80052350(pp);
            p++;
            y = func_80052350(pp);
            goto loop;
        case 'W':
        case 'w':
            w = func_80052350(pp);
            p++;
            h = func_80052350(pp);
            goto loop;
        case 'C':
        case 'c':
            c = func_80052350(&p);
            goto loop;
        case 'P':
        case 'p':
            a1 = func_80052350(&p);
            goto loop;
        case '/':
            return;
        default:
            break;
        }
    }
    do {
        p--;
    } while (0);
    q = p;
    maxw = 0;
    th = 12;
    ch = *q++;
    lw = 0;
    while (ch != 0) {
        if (ch < 0x80) {
            switch (ch) {
            case '
':
                th += 12;
                if (maxw < lw) {
                    maxw = lw;
                }
                lw = 0;
                break;
            case ' ':
                lw += 6;
                break;
            case 'F':
            case 'f':
                goto done;
            }
        } else if (ch < 0xF0) {
            lw += 6;
        } else {
            q++;
            lw += 12;
        }
        ch = *q++;
    }
done:
    if (maxw < lw) {
        maxw = lw;
    }
    func_80052650(x, y, w, h, (w - maxw) >> 1, (h - th) >> 1, c, one, a0, a1, p);
}
