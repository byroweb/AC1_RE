/* ovl201 0x8007145C  size 160  variant sweep (tools/match/variants.py): old lever: etype D_80039C5D s8->u8, from the ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8005E92C();                            /* extern */
M2C_UNK func_800866A8();                            /* extern */
M2C_UNK func_800866FC();                            /* extern */
M2C_UNK func_80096230();                            /* extern */
M2C_UNK func_8009C964(M2C_UNK);                     /* extern */
extern u8 D_80039C5D;
extern s8 D_80039CAA;
extern s8 D_800411F8;
extern s8 D_80041B10;
extern M2C_UNK func_8005C2D0;

void func_8007145C(void *arg0) {
    if (obj_find_by_handle(&func_8005C2D0) == &g_ui_none) {
        func_800866FC();
        func_80096230();
        D_80041B10 = -1;
        func_800866A8();
        D_800411F8 = 0x46;
        D_80039C5D = 0xFF;
        func_8009C964(0);
        D_80039CAA = 1;
        func_8005E92C();
        M2C_FIELD(arg0, s8 *, 1) = 1;
    }
}
