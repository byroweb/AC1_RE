#include "ac1/ac1.h"
/* ovl202 0x80072120 - bump-allocate a block (16-byte header + size, 4-aligned) from the
 * mission heap and stamp its header with the 8-byte tag at 0x80039C78.
 * buf: the retail frame is 144 bytes with no stack accesses (an unused local). */
typedef struct Tag8 {
    u8 b[8];
} Tag8;
typedef struct HeapHdr {
    s32 unk00;
    s32 unk04;
    Tag8 tag;
} HeapHdr;
typedef struct Heap {
    u8 *cur;
} Heap;
extern Heap D_801E8B68;
extern Tag8 D_80039C78;

HeapHdr * func_80072120(s32 size)
{
    HeapHdr *h;
    u8 buf[144];

    h = (HeapHdr *)D_801E8B68.cur;
    D_801E8B68.cur = (u8 *)h + ((size + 0x13) & ~3);
    h->unk00 = 0;
    h->unk04 = 0;
    h->tag = D_80039C78;
    return h;
}
