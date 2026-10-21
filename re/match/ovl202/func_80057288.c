#include "ac1/ac1.h"

typedef struct {
    u8 buf[0x300];
    u8 *cur;
    s32 n;
} Pool300;

extern Pool300 D_801AE82C;

void func_80057288(void) {
    D_801AE82C.cur = D_801AE82C.buf;
    D_801AE82C.n = 0;
}
