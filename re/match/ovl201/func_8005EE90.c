#include "ac1/ac1.h"

extern u8 D_80031A58[];
extern s32 func_8007AA04(s32 slot);
extern void func_80050AF4(s32 kind, s32 id);
extern void func_80074A18(void);

#define CHECK(slot, kind, base) \
    v = D_80031A58[slot]; \
    if (v == 0xFF) { \
        r = func_8007AA04(slot); \
        if ((u8)r != v) { \
            func_80050AF4(kind, (r & 0xFF) + (base)); \
        } \
    }

void func_8005EE90(u8 refresh)
{
    u8 v;
    s32 r;

    CHECK(0, 4, 3);
    CHECK(11, 6, 6);
    CHECK(3, 5, 38);
    CHECK(2, 3, 54);
    CHECK(1, 14, 64);
    CHECK(9, 12, 124);
    CHECK(8, 11, 143);
    if (refresh) {
        func_80074A18();
    }
}
