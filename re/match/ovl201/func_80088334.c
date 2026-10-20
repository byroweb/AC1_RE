/* ovl201 0x80088334  size 680  zero-agent sweep (tools/loop/sweep.py): m2c ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK SsUtKeyOn(s16, M2C_UNK, M2C_UNK, M2C_UNK, s32, s32, s32); /* extern */
M2C_UNK func_800506EC(M2C_UNK);                     /* extern */
M2C_UNK func_8005C2D0(M2C_UNK, s16, M2C_UNK, s32, s32); /* extern */
void func_800885DC(UiObj *o);                       /* extern */
M2C_UNK func_80088DE4();                            /* extern */
void func_8008B568(UiObj *);                        /* extern */
void func_8008BD9C(UiObj *);                        /* extern */
void func_8008BEEC(UiObj *);                        /* extern */
void func_8008BFA8(UiObj *);                        /* extern */
void func_8008C07C(UiObj *);                        /* extern */
extern s16 D_801A2318;
extern s32 D_801A2568;
extern s32 D_801A256C;
extern void *D_801F5094;
extern M2C_UNK func_8005BA2C;
extern M2C_UNK func_8008AFA8;
extern M2C_UNK func_8008BC58;
extern M2C_UNK func_8008BDA8;
extern M2C_UNK func_8008BEF8;
extern M2C_UNK func_8008BFB4;

void func_80088334(void *arg0) {
    UiObj *temp_s1;
    UiObj *temp_s2;
    UiObj *temp_s3;
    UiObj *temp_s4;
    UiObj *temp_s5;
    UiObj *temp_v0;
    void (*var_v0)(UiObj *);

    temp_s5 = obj_find_by_handle(&func_8008AFA8);
    temp_s3 = obj_find_by_handle(&func_8008BC58);
    temp_s4 = obj_find_by_handle(&func_8008BDA8);
    temp_s1 = obj_find_by_handle(&func_8008BEF8);
    temp_s2 = obj_find_by_handle(&func_8008BFB4);
    temp_v0 = obj_find_by_handle(&func_8005BA2C);
    if (temp_v0 == &g_ui_none) {
        if ((D_801A2568 & 0x40) && !(D_801A256C & 0x40)) {
            func_80088DE4();
            temp_s3->update = func_8008BD9C;
            temp_s4->update = func_8008BEEC;
            if (temp_s1 != temp_v0) {
                temp_s1->update = func_8008BFA8;
            }
            if (temp_s2 != temp_v0) {
                temp_s2->update = func_8008C07C;
            }
            temp_s5->update = func_8008B568;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x20, (s16) (M2C_FIELD(D_801F5094, u16 *, 0x24) + 0x5C), 0x100, M2C_FIELD(D_801F5094, s8 *, 1) * 0x18, 3);
            var_v0 = ui_obj_nop2;
            goto block_15;
        }
        if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
            SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
            temp_s3->update = func_8008BD9C;
            temp_s4->update = func_8008BEEC;
            if (temp_s1 != &g_ui_none) {
                temp_s1->update = func_8008BFA8;
            }
            if (temp_s2 != &g_ui_none) {
                temp_s2->update = func_8008C07C;
            }
            temp_s5->update = func_8008B568;
            func_800506EC(4);
            obj_find_by_id(3)->kill = 1;
            func_8005C2D0(0x20, (s16) (M2C_FIELD(D_801F5094, u16 *, 0x24) + 0x5C), 0x100, M2C_FIELD(D_801F5094, s8 *, 1) * 0x18, 3);
            var_v0 = func_800885DC;
block_15:
            M2C_FIELD(arg0, void (**)(UiObj *), 0x74) = var_v0;
        }
    }
}
