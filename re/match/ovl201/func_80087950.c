/* ovl201 0x80087950  size 412  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8008BEF8();                            /* extern */
void func_8008BFA8(UiObj *);                        /* extern */
M2C_UNK func_8008BFB4();                            /* extern */
void func_8008C07C(UiObj *);                        /* extern */
extern s8 D_801F37CA;
extern s8 *D_801F5094;

void func_80087950(void) {
    UiObj *temp_v0;
    UiObj *temp_v0_2;
    s8 temp_a0;
    s8 temp_v1;

    if ((*D_801F5094 >= 6) && (D_801F37CA > 0) && (obj_find_by_handle(func_8008BEF8) == &g_ui_none)) {
        func_8008BEF8();
    }
    if (*D_801F5094 >= 6) {
        if (D_801F37CA <= 0) {
            temp_v0 = obj_find_by_handle(func_8008BEF8);
            if (temp_v0 != &g_ui_none) {
                temp_v0->update = func_8008BFA8;
            }
        }
        temp_a0 = *D_801F5094;
        if (temp_a0 >= 6) {
            if (((temp_a0 - 6) >= D_801F37CA) && (obj_find_by_handle(func_8008BFB4) == &g_ui_none)) {
                func_8008BFB4();
            }
            temp_v1 = *D_801F5094;
            if ((temp_v1 >= 6) && (D_801F37CA >= (temp_v1 - 5))) {
                temp_v0_2 = obj_find_by_handle(func_8008BFB4);
                if (temp_v0_2 != &g_ui_none) {
                    temp_v0_2->update = func_8008C07C;
                }
            }
        }
    }
}
