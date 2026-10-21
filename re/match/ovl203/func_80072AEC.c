#include "ac1/ac1.h"

/* 0x170-byte records; the five words cleared here are +0x15C..+0x16C. */
typedef struct {
    s32 pad[0x170 / 4 - 5];
    s32 a, b, c, d, e;
} Rec170;

extern Rec170 D_8019B88C[];

/* The counter starts at 0 and is pre-decremented: the retail loop wraps. */
void func_80072AEC(void) {
    s32 i;
    Rec170 *p = (Rec170 *)((u8 *)D_8019B88C - 0x170 + 4);

    i = 0;
    do {
        p->a = 0;
        p->b = 0;
        p->c = 0;
        p->d = 0;
        p->e = 0;
        p++;
    } while (--i != 0);
}
