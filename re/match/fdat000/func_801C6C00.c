/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0026): depth at
 * which id occurs below node n of the 0x34-byte table at 0x80198B08 (children at
 * +0x28/+0x29, 0 = none), starting from depth; 9999 when it does not occur. */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x28];
    union {
        u8 id[2];       /* +0x28 left, +0x29 right */
        u16 any;        /* both at once */
    } kids;
    u8 pad2a[0x34 - 0x2A];
} Func801C6C00Node;
extern Func801C6C00Node D_80198B08[];
s32 func_801C6C00(Func801C6C00Node *n, s32 id, s32 depth)
{
    s32 a, b;

    if (n->kids.any == 0)
        return 9999;
    if (n->kids.id[0] == id || n->kids.id[1] == id)
        return depth;
    depth++;
    if (n->kids.id[0] != 0)
        a = func_801C6C00(&D_80198B08[n->kids.id[0]], id, depth);
    else
        a = 9999;
    if (n->kids.id[1] != 0)
        b = func_801C6C00(&D_80198B08[n->kids.id[1]], id, depth);
    else
        b = 9999;
    if (a <= b)
        return a;
    return b;
}
