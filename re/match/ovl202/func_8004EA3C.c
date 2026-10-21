#include "ac1/ac1.h"

typedef struct {
    u8 c[4];
} Col4;

typedef struct {
    u8 pad[0x54];
    u8 count;
    u8 unk55;
    Col4 cols[1];
} ColObj;

void func_8004EA3C(ColObj *o, s32 n, Col4 *src)
{
    Col4 *dst = o->cols;

    o->count = n;
    o->unk55 = 0;
    if (n == 0xFF) {
        n = 1;
    }
    for (n--; n != -1; n--) {
        *dst++ = *src++;
    }
}
