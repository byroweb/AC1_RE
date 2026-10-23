#include "ac1/ac1.h"
/* ovl202 0x8004C854 - open a VAB pack: s32 n, n header blocks
 * {s32 size; u8 data[size]}, then n body blocks {s32 size; s32 slot; ...};
 * each pair goes to func_8004C7A8(slot, header, body). Returns the start of
 * the body blocks (the pack itself when n is 0). */

extern void func_8004C7A8(s32 slot, u8 *head, u8 *body);

u8 *func_8004C854(u8 *p)
{
    s32 n;
    s32 i;
    u8 *h;
    u8 *b;
    u8 *q;
    u8 *hd;
    u8 *bd;
    s32 *x;

    n = *(s32 *)p;
    h = p + 4;
    if (n != 0) {
        q = h;
        for (i = n - 1; i != -1; i--) {
            q += *(s32 *)q + 4;
        }
        b = q;
        for (i = n - 1; i != -1; i--) {
            hd = h + 4;
            bd = b + 4;
            h += *(s32 *)h + 4;
            b += *(s32 *)b + 4;
            func_8004C7A8(*(s32 *)bd, hd, bd + 4);
        }
        p = q;
    }
    return p;
}
