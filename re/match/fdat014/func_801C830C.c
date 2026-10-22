#include "ac1/ac1.h"

typedef struct {
    u8 pad0[6];
    s16 kind;           /* +0x06 */
    u8 pad8[0x2C];
} Func801C830CSlot;     /* 0x34 */
typedef struct {
    u8 pad0[0x0E];
    s16 id;             /* +0x0E */
    u8 pad10[8];
} Func801C830CKind;     /* 0x18 */
extern Func801C830CSlot D_80198B08[];
extern Func801C830CKind D_80198800[];

s32 func_801C830C(s32 id)
{
    Func801C830CSlot *s;
    s32 i;
    s32 n;

    s = D_80198B08;
    i = 512;
    n = 0;
    do {
        if (D_80198800[s->kind].id == id) {
            n++;
        }
        i--;
        s++;
    } while (i != 0);
    return n;
}
