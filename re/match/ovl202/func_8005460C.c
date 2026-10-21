#include "ac1/ac1.h"

extern void func_800543F0(u8 *);

void func_8005460C(u8 *p, u32 n) {
    while (n--) {
        func_800543F0(p);
        p += 0x7C;
    }
}
