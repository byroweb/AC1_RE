#include "ac1/ac1.h"

typedef struct {
    u8 *wr;
    u8 *rd;
} CdQueue;
extern CdQueue D_8004A260;
void func_800161F8(void);

void func_80015A48(void)
{
    for (;;) {
        EnterCriticalSection();
        if (D_8004A260.rd == D_8004A260.wr && *D_8004A260.rd == 0)
            break;
        ExitCriticalSection();
        func_800161F8();
    }
    ExitCriticalSection();
}
