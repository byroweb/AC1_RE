#include "ac1/ac1.h"
/* ovl201 0x800512D4 - unpack an RLE block (count byte: >0 repeat the next byte,
 * <=0 copy -count bytes) into a new buffer and free the source */
u8 *func_80050FC4(s32 size);
void func_80051078(void *p);


u8 *func_800512D4(u32 *src)
{
    s32 n;
    u8 *s;
    u8 *out;
    u8 *d;
    s8 c;
    s16 i;

    n = src[1];
    s = (u8 *)(src + 2);
    out = func_80050FC4(n);
    d = out;
    while (n != 0) {
        c = *s++;
        if (c > 0) {
            for (i = 0; i < c; i++) {
                *d++ = *s;
            }
            s++;
        } else {
            c *= -1;
            for (i = 0; i < c; i++) {
                *d++ = *s++;
            }
        }
        n -= c;
    }
    func_80051078(src);
    return out;
}
