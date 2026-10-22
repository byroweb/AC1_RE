#include "ac1/ac1.h"
/* ovl201 0x800829F4 - fill the character list of the picked name-entry key: up to eight
 * fixed characters from the key's table entry, then the SJIS range D_800B8628[r + c * 5][8]..D_800B8628[r + c * 5][9] (skipping the
 * 0x7F and 0xFD..0x13F holes), laid out 18 per row of 38 bytes ('>' terminated) in
 * 0x8017EF9C; one text label per row (max 8); row count, last-row length, scroll := 0 */
typedef struct {
    u8 pad7c[0x1B];
    u8 rows;                    /* +0x97 */
    u8 last_len;                /* +0x98 */
    u8 top;                     /* +0x99 */
    u8 col;                     /* +0x9A key column (read through o->w.raw) */
    u8 row;                     /* +0x9B key row */
} KanaWork;

extern u16 D_800B8628[][10];
extern char D_8017EF9C[8][38];
extern UiObj *func_8005CB94(char *s, s32 a, s16 y, s32 b);

void func_800829F4(UiObj *o)
{
    KanaWork *w;
    u8 c;
    u8 r;
    u16 k;
    u16 pos;
    u16 line;
    s16 last;

    c = o->w.raw[0x1E];
    r = o->w.raw[0x1F];
    k = 0;
    D_8017EF9C[0][k++] = D_800B8628[r + c * 5][0] >> 8;
    D_8017EF9C[0][k++] = D_800B8628[r + c * 5][0];
    w = (KanaWork *)&o->w;
    if (D_800B8628[r + c * 5][1] != 0xFFFF) {
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][1] >> 8;
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][1];
    }
    if (D_800B8628[r + c * 5][2] != 0xFFFF) {
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][2] >> 8;
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][2];
    }
    if (D_800B8628[r + c * 5][3] != 0xFFFF) {
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][3] >> 8;
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][3];
    }
    D_8017EF9C[0][k++] = D_800B8628[r + c * 5][4] >> 8;
    D_8017EF9C[0][k++] = D_800B8628[r + c * 5][4];
    if (D_800B8628[r + c * 5][5] != 0xFFFF) {
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][5] >> 8;
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][5];
    }
    if (D_800B8628[r + c * 5][6] != 0xFFFF) {
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][6] >> 8;
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][6];
    }
    if (D_800B8628[r + c * 5][7] != 0xFFFF) {
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][7] >> 8;
        D_8017EF9C[0][k++] = D_800B8628[r + c * 5][7];
    }
    pos = k;
    line = 0;
    k = D_800B8628[r + c * 5][8];
    if (k != 0xFFFF) {
        while (k <= D_800B8628[r + c * 5][9]) {
            if ((k & 0xFF) == 0xFD) {
                k += 67;
                if (k >= D_800B8628[r + c * 5][9]) {
                    break;
                }
            } else if ((k & 0xFF) == 0x7F) {
                if (++k >= D_800B8628[r + c * 5][9]) {
                    break;
                }
            }
            D_8017EF9C[line][pos] = k >> 8;
            D_8017EF9C[line][pos + 1] = k;
            pos += 2;
            if (pos == 36) {
                pos = 0;
                D_8017EF9C[line][36] = '>';
                D_8017EF9C[line][37] = 0;
                line++;
            }
            k++;
        }
    }
    if (pos != 0) {
        D_8017EF9C[line][pos] = '>';
        D_8017EF9C[line][pos + 1] = 0;
    }
    last = pos ? line : line - 1;
    for (k = 0; k <= last; k++) {
        if (k == 8) {
            break;
        }
        func_8005CB94(D_8017EF9C[k], 8, k * 16 + 4, 8)->unk14 = 3;
    }
    w->rows = line;
    w->last_len = pos >> 1;
    w->top = 0;
}
