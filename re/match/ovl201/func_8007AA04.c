#include "ac1/ac1.h"
/* ovl201 0x8007AA04 - lowest set bit (0..63) of the 64-bit set of the part
 * category cat, 255 when the set is empty. */

extern u32 D_800485A0[][2];

u8 func_8007AA04(u8 cat)
{
    u32 *bits;
    u32 m;
    u8 i;

    switch (cat) {
    case 2:
        bits = D_800485A0[5];
        break;
    case 3:
        bits = D_800485A0[2];
        break;
    case 0:
        bits = D_800485A0[0];
        break;
    case 11:
        bits = D_800485A0[1];
        break;
    case 8:
        bits = D_800485A0[11];
        break;
    case 1:
        bits = D_800485A0[6];
        break;
    case 9:
        bits = D_800485A0[9];
        break;
    }
    m = bits[0];
    for (i = 0; i < 32; i++) {
        if (m & 1) {
            return i;
        }
        m >>= 1;
    }
    m = bits[1];
    for (i = 32; i < 64; i++) {
        if (m & 1) {
            return i;
        }
        m >>= 1;
    }
    return 255;
}
