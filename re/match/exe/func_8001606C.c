#include "ac1/ac1.h"

typedef struct MemBlk {
    u8 used;
    u8 pad1[3];
    s32 size;
    void **owner;
    s32 tag;
} MemBlk;

extern MemBlk *func_80015E68(MemBlk *heap, s32 size);

void *func_8001606C(MemBlk *heap, s32 size, void **owner, s32 tag)
{
    MemBlk *b;
    MemBlk *n;
    s32 rest;
    void *p;

    size = (size + 3) & ~3;
    b = func_80015E68(heap, size);
    if (b == 0) {
        return 0;
    }
    {
        rest = b->size - (size + 0x10);
        if (rest >= 0x810) {
            n = (MemBlk *)((u8 *)b + size);
            n[1].used = 0;
            n[1].size = rest;
            n[1].owner = 0;
        } else {
            size = b->size;
        }
        b->used = 2;
        p = b + 1;
        b->size = size;
        b->owner = owner;
        b->tag = tag;
        if (owner != 0) {
            *owner = p;
        }
    }
    return p;
}
