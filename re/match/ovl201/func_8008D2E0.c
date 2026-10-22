#include "ac1/ac1.h"
/* ovl201 0x8008D2E0 - build the sell list (owned parts): every part id 0..159 whose flag
 * at 0x80031A94 is set gets a row (category, index within the category),
 * counted per category; then the first row of each category. */

typedef struct {
    u8 pad00[0x5A];
    u16 top;            /* +0x5A */
    s16 total;          /* +0x5C */
    u8 pad5e[0x2E];
    s8 count[10];       /* +0x8C */
    s16 start[10];      /* +0x96 */
    u8 padAA[0x72];
    s8 kind[0xA0];      /* +0x11C */
    s8 idx[0xA0];       /* +0x1BC */
} ShopState;
extern ShopState D_8019C090;
extern u8 D_80031A94[];

void func_8008D2E0(void)
{
    s32 i;
    s32 id;
    s32 n;
    s16 sum;

    for (i = 9; i != -1; i--) {
        D_8019C090.count[i] = 0;
    }
    n = 0;
    for (id = 0; id < 160; id++) {
        if (D_80031A94[id] != 0) {
            if (id < 10) {
                D_8019C090.kind[n] = 0;
                D_8019C090.idx[n] = id;
                D_8019C090.count[0]++;
            } else if (id < 13) {
                D_8019C090.kind[n] = 1;
                D_8019C090.idx[n] = id - 10;
                D_8019C090.count[1]++;
            } else if (id < 29) {
                D_8019C090.kind[n] = 2;
                D_8019C090.idx[n] = id - 13;
                D_8019C090.count[2]++;
            } else if (id < 61) {
                D_8019C090.kind[n] = 3;
                D_8019C090.idx[n] = id - 29;
                D_8019C090.count[3]++;
            } else if (id < 68) {
                D_8019C090.kind[n] = 4;
                D_8019C090.idx[n] = id - 61;
                D_8019C090.count[4]++;
            } else if (id < 76) {
                D_8019C090.kind[n] = 5;
                D_8019C090.idx[n] = id - 68;
                D_8019C090.count[5]++;
            } else if (id < 87) {
                D_8019C090.kind[n] = 6;
                D_8019C090.idx[n] = id - 76;
                D_8019C090.count[6]++;
            } else if (id < 93) {
                D_8019C090.kind[n] = 7;
                D_8019C090.idx[n] = id - 87;
                D_8019C090.count[7]++;
            } else if (id < 127) {
                D_8019C090.kind[n] = 8;
                D_8019C090.idx[n] = id - 93;
                D_8019C090.count[8]++;
            } else if (id < 147) {
                D_8019C090.kind[n] = 9;
                D_8019C090.idx[n] = id - 127;
                D_8019C090.count[9]++;
            }
            n++;
        }
    }
    D_8019C090.total = n;
    sum = 0;
    for (i = 0; i < 10; i++) {
        D_8019C090.start[i] = sum;
        sum += D_8019C090.count[i];
    }
}
