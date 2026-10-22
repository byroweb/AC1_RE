#include "ac1/ac1.h"
/* ovl201 0x8006599C - draw the object's '>'-terminated string (o->res) at
 * o->pos: control codes CR (new line), ';' (new line by size), ' ', '{n'/'}n'
 * (move left/right n), '^n' (size), '~' (up 4), '@n' (palette); ASCII and
 * half-width kana through func_800660C4, Shift-JIS through the glyph-page
 * table D_800B75B8 and func_80065DBC. */

typedef struct {
    u16 first;                  /* first SJIS code on the page (table descends) */
    u16 tpage;
    u16 clut_y;
} GlyphPage;

extern GlyphPage D_800B75B8[16];
extern void func_800660C4(UiObj *o, u8 c, s16 x, s16 y, u8 size, u8 pal);
extern void func_80065DBC(UiObj *o, s16 tpage, s16 clut, s16 x, s16 y, s16 u, s16 v);

void func_8006599C(UiObj *o)
{
    u8 *s;
    s16 x;
    s16 y;
    s16 x0;
    u8 c;
    u8 size;
    u8 pal;
    u16 code;
    u16 i;
    u16 j;
    s16 n;
    s16 row;
    s32 v;
    s16 tpage;
    u16 clut;

    pal = 0;
    size = 0;
    s = o->res;
    x0 = x = o->pos.vx;
    y = o->pos.vy;
    while ((c = *s) != '>') {
        if (c == '\r') {
            s += 2;
            x = x0;
            y += 16;
        } else if (c == ';') {
            x = x0;
            s++;
            switch (size) {
            case 0:
                y += 16;
                break;
            case 4:
            case 6:
            case 8:
                y += 8;
                break;
            }
        } else if (c == ' ') {
            s++;
            x += 8;
        } else if (c == '{') {
            s++;
            x -= *s++ - '0';
        } else if (c == '}') {
            s++;
            x += *s++ - '0';
        } else if (c == '^') {
            s++;
            size = *s++ - '0';
        } else if (c == '~') {
            s++;
            y -= 4;
        } else if (c == '@') {
            s++;
            pal = *s++ - '0';
        } else if (c < 0x80 || (u8)(c + 0x60) < 0x40) {
            s++;
            func_800660C4(o, c, x, y, size, pal);
            switch (size) {
            case 0:
                x += 8;
                break;
            case 8:
                x += 8;
                break;
            case 6:
                x += 6;
                break;
            case 4:
                x += 4;
                break;
            }
        } else {
            code = *s++;
            code = *s++ | (code << 8);
            for (i = 0; i < 16; i++) {
                if (code >= D_800B75B8[i].first) {
                    tpage = D_800B75B8[i].tpage;
                    clut = ((D_800B75B8[i].clut_y + pal * 4) << 6) | 0x14;
                    break;
                }
            }
            if (i == 16) {
                code = 0x8140;
                i = 15;
                tpage = D_800B75B8[15].tpage;
                clut = ((D_800B75B8[15].clut_y + pal * 4) << 6) | 0x14;
            }
            if (i == 12) {
                n = 7;
                row = 11;
            } else {
                n = 0;
                row = 0;
            }
            for (j = D_800B75B8[i].first; j < code; j++) {
                if ((j & 0xFF) == 0xFD) {
                    j += 0x43;
                    if (j >= code) {
                        break;
                    }
                } else if ((j & 0xFF) == 0x7F) {
                    if (++j >= code) {
                        break;
                    }
                }
                n++;
            }
            v = (row + n / 16) << 4;
            func_80065DBC(o, tpage, clut, x, y, (n & 0xF) << 4, v);
            x += 16;
        }
    }
}
