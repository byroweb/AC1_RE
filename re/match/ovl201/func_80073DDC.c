#include "ac1/ac1.h"
/* ovl201 0x80073DDC - build the four paint CLUTs: for each of the 4 x 4
 * colours of D_80031A64 a 64-entry ramp (pairs of equal entries) from the
 * colour down to a dark end, then upload each paint's 256 entries to the
 * RECT of D_800B859C. */

extern u8 D_80031A64[4][4][3];
extern u16 D_8017E620[4][256];
extern RECT D_800B859C[4];
extern int LoadImage(RECT *rect, u16 *p);

void func_80073DDC(void)
{
    s16 p;
    s16 row;
    s16 i;
    u16 r, g, b;
    u16 r1, g1, b1;
    u16 dr, dg, db;
    u16 c;

    for (p = 0; p < 4; p++) {
        for (row = 0; row < 4; row++) {
            if (D_80031A64[p][row][0] >= 32) {
                r = 7936;
                r1 = (D_80031A64[p][row][0] - 30) << 8;
            } else {
                r = D_80031A64[p][row][0] << 8;
                r1 = 256;
            }
            if (D_80031A64[p][row][1] >= 32) {
                g = 7936;
                g1 = (D_80031A64[p][row][1] - 30) << 8;
            } else {
                g = D_80031A64[p][row][1] << 8;
                g1 = 256;
            }
            if (D_80031A64[p][row][2] >= 32) {
                b = 7936;
                b1 = (D_80031A64[p][row][2] - 30) << 8;
            } else {
                b = D_80031A64[p][row][2] << 8;
                b1 = 256;
            }
            if (r1 > 7936) {
                r1 = 7936;
            }
            if (g1 > 7936) {
                g1 = 7936;
            }
            if (b1 > 7936) {
                b1 = 7936;
            }
            if (r == 0) {
                r = 1;
            }
            if (g == 0) {
                g = 1;
            }
            if (b == 0) {
                b = 1;
            }
            dr = (r - r1) / 32;
            dg = (g - g1) / 32;
            db = (b - b1) / 32;
            for (i = 0; i < 64; i += 2) {
                c = ((b >> 8 & 0x1F) << 10) | ((g >> 8 & 0x1F) << 5) | (r >> 8 & 0x1F);
                r -= dr;
                g -= dg;
                D_8017E620[p][row * 64 + i + 1] = c;
                D_8017E620[p][row * 64 + i] = c;
                b -= db;
                if (r < r1) {
                    r = r1;
                }
                if (g < g1) {
                    g = g1;
                }
                if (b < b1) {
                    b = b1;
                }
            }
        }
        LoadImage(&D_800B859C[p], D_8017E620[p]);
    }
}
