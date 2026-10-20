/* fdat000 (mission library, entry 0 @ 0x801C4B40; 31 twins, group g0027): does the
 * subtree of node n in the 0x34-byte table at 0x80198B08 contain id?  Children are
 * the indices at +0x28/+0x29 (0 = none). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad[0x28];
    union {
        u8 id[2];       /* +0x28 left, +0x29 right */
        u16 any;        /* both at once */
    } kids;
    u8 pad2a[0x34 - 0x2A];
} Func801C6B30Node;
extern Func801C6B30Node D_80198B08[];
s32 func_801C6B30(Func801C6B30Node *n, s32 id)
{
    if (n->kids.id[0] == id || n->kids.id[1] == id)
        return 1;
    if (n->kids.any == 0)
        return 0;
    if (n->kids.id[0] != 0 && func_801C6B30(&D_80198B08[n->kids.id[0]], id))
        return 1;
    if (n->kids.id[1] != 0 && func_801C6B30(&D_80198B08[n->kids.id[1]], id))
        return 1;
    return 0;
}
