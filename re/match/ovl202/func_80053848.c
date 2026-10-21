#include "ac1/ac1.h"

typedef struct {
    u8 *cur;                    /* 0x801A5F30 */
    void *tbl[1];               /* 0x801A5F34 */
} TexSlots;

extern TexSlots D_801A5F30;
extern void func_800574D8(u8 *);

void func_80053848(u16 slot, s32 *p) {
    u32 size;
    u16 n;
    u8 *q;

    n = *p++;
    while (n--) {
        size = *p;
        if (size >= 5) {
            D_801A5F30.tbl[slot] = p;
            q = (u8 *)p + p[2];
            D_801A5F30.cur = q;
            func_800574D8(q);
        }
        p = (s32 *)((u8 *)p + size);
        slot++;
    }
}
