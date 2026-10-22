#include "ac1/ac1.h"

typedef struct {
    u8 used;
    u8 pad1[3];
    s32 size;
    u8 pad8[8];
} MemBlk;

extern void func_80015A48(void);

void func_80015FB4(MemBlk *b)
{
    while (b->used != 0xFF) {
        while (b->used == 3) {
            func_80015A48();
        }
        b = (MemBlk *)((u8 *)b + (b->size + 0x10));
    }
}
