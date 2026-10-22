#include "ac1/ac1.h"
/* ovl201 0x80084364 - position of the count-th set bit of D_800485F0 after pos */
extern u32 D_800485F0;
typedef struct PickObj {
    u8 unk00[0xC4];
    u8 count;                   /* +0xC4 */
    u8 unkC5;
    u8 pos;                     /* +0xC6 */
} PickObj;

u8 func_80084364(PickObj *o)
{
    u32 m;
    u8 i;
    u8 k;

    m = D_800485F0;
    i = o->pos;
    k = o->count;
    m >>= i + 1;
    while (k != 0) {
        if (m & 1) {
            k--;
        }
        i++;
        m >>= 1;
    }
    return i;
}
