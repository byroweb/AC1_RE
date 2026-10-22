#include "ac1/ac1.h"

typedef struct MemBlk {
    u8 used;
    u8 pad1[3];
    u32 size;
    u8 pad8[8];
} MemBlk;

#define NEXT(b) ((MemBlk *)((u8 *)(b) + ((b)->size + 0x10)))

extern void func_80015E48(MemBlk *b);
extern void func_80015DC8(MemBlk *b);

MemBlk *func_80015E68(MemBlk *heap, u32 size)
{
    MemBlk *b;
    MemBlk *start;

    b = heap;
    while (b->used != 0xFF) {
        if (b->used == 0 && b->size >= size) {
            return b;
        }
        b = NEXT(b);
    }
    b = heap;
    while (b->used != 0xFF) {
        start = b;
        if (b->used == 1) {
            while (b->used == 1) {
                b = NEXT(b);
            }
            if ((u32)((u8 *)b - (u8 *)start - 0x10) >= size) {
                b = start;
                while (b->used == 1) {
                    func_80015E48(b);
                    b = NEXT(b);
                }
                func_80015DC8(start);
                return start;
            }
        } else {
            b = NEXT(b);
        }
    }
    return 0;
}
