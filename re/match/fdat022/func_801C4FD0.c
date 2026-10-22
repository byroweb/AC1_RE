#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8007732C(void *, M2C_UNK, M2C_UNK, M2C_UNK, void *, M2C_UNK *); /* extern */
M2C_UNK func_80078598(void *);                      /* extern */
extern M2C_UNK func_801C4CFC;

void func_801C4FD0(void *arg0, s32 arg1)
{
    s32 s;

    s = arg1;
    if (M2C_FIELD(arg0, u16 *, 0x36) & 0x10) {
        s = M2C_FIELD(arg0, u8 *, 0x3C);
    }
    func_80078598(arg0);
    switch (s) {
    case 0:
        func_8007732C(arg0, 0, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    case 1:
        func_8007732C(arg0, 1, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    case 2:
        func_8007732C(arg0, 2, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    case 3:
        func_8007732C(arg0, 3, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    case 4:
        func_8007732C(arg0, 4, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    case 5:
        func_8007732C(arg0, 5, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    case 6:
        func_8007732C(arg0, 6, 1, 0x200, (u8 *)arg0 + 0xCC, &func_801C4CFC);
        break;
    }
    if (!(M2C_FIELD(arg0, u16 *, 0x36) & 0x10)) {
        M2C_FIELD(arg0, u8 *, 0x3C) = s;
    }
}
