#include "ac1/ac1.h"
#include "ac1/ovl201_t49.h"

extern void func_800154D8(void *dst, void *src, s32 nwords);
extern void free2(void *);

void func_800963A0(void *data) {
    s32 i;

    func_800154D8(D_8017F6D0.saved, data, 0x420);
    for (i = 63; i != -1; i--) {
        D_8017F6D0.cur[i] = D_8017F6D0.saved[i];
    }
    D_8017F6D0.f794 = 0;
    free2(data);
}
