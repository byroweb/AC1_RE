#include "ac1/ac1.h"

typedef struct {
    s8 count;
    s8 rows;
    u8 pad2[0x23F - 2];
    char row[1][34];            /* +0x23F */
} NameList;

typedef struct {
    u8 pad0[4];
    s8 b4;                      /* +0x04 0x801F3754 */
    u8 pad5[0x38 - 5];
    u8 flags[64];               /* +0x38 0x801F3788 */
    u8 pad78[0x1C4 - 0x78];
    char names[64][32];         /* +0x1C4 0x801F3914 */
    u8 pad9C4[0x1944 - 0x9C4];
    NameList *list;           /* +0x1944 0x801F5094 */
} Garage3750;

extern Garage3750 D_801F3750;

void func_80087F58(u8 id, u8 row)
{
    char *src;
    char *dst;
    char *p;

    D_801F3750.list->row[row][0] = 0x40;
    D_801F3750.list->row[row][1] = 0x30;
    p = D_801F3750.list->row[row];
    dst = p + 2;
    src = D_801F3750.names[id];
    if (D_801F3750.b4 == 0) {
        dst = p + 4;
        if (D_801F3750.flags[id] & 2) {
            p[2] = 0x81;
            p[3] = 0xA6;
        } else {
            p[2] = 0x81;
            p[3] = 0x40;
        }
    }
    while (*src != '>') {
        *dst++ = *src++;
    }
    dst[0] = '>';
    dst[1] = 0;
}
