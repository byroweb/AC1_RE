#include "ac1/ac1.h"

typedef struct {
    u8 used;
    u8 pad1[3];
    s32 size;
    u8 pad8[8];
} MemBlk;

void func_80015DC8(MemBlk *head)
{
    MemBlk *p;

    p = head;
    while (p->used != 0xFF) {
        if (p->used == 0) {
            p = (MemBlk *)((u8 *)p + (p->size + 0x10));
            if (p->used != 0) {
                return;
            }
            head->size += 0x10 + p->size;
        } else {
            p = (MemBlk *)((u8 *)p + (p->size + 0x10));
        }
    }
}
