/* MATCH: row s16 + q = row + n / 16 (s32), y s16 / cx s32 params. */
#include "ac1/ac1.h"

typedef struct {
    u16 code;                   /* first SJIS code of the block */
    u16 u;                      /* glyph sheet origin */
    u16 v;
} GlyphBlock;

extern int LoadImage(RECT *rect, u32 *p);
extern GlyphBlock D_800B7470[16];
extern u32 D_800B74D0[];
extern void func_8005F49C(s16 u, s16 v, s16 x, s16 y, s32 gx, s16 gy);

/* Draw a '>'-terminated SJIS string glyph by glyph, then load its CLUT. */
void func_8005F260(u8 *str, s32 x, s16 y, s32 cx, s16 cy)
{
    u16 code;
    u16 i;
    u16 c;
    s16 n;
    s16 row;
    s32 q;
    s16 u;
    s16 v;
    RECT r;

    while (*str != '>') {
        code = *str++;
        code = (code << 8) | *str++;
        for (i = 0; i < 16; i++) {
            if (code >= D_800B7470[i].code) {
                u = D_800B7470[i].u;
                v = D_800B7470[i].v;
                break;
            }
        }
        if (i == 12) {
            n = 7;
            row = 11;
        } else {
            n = 0;
            row = 0;
        }
        c = D_800B7470[i].code;
        while (c < code) {
            if ((c & 0xFF) == 0xFD) {
                c += 0x43;
                if (c >= code) {
                    break;
                }
            } else if ((c & 0xFF) == 0x7F) {
                if (++c >= code) {
                    break;
                }
            }
            c++;
            n++;
        }
        q = row + n / 16;
        func_8005F49C(u, v, x, y, (n & 0xF) * 4, q * 16);
        x += 4;
    }
    r.x = cx;
    r.y = cy;
    r.w = 16;
    r.h = 1;
    LoadImage(&r, D_800B74D0);
}
