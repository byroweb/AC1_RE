/* ovl201 0x8006AFA0  emblem editor: answer of the exit dialog */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_800506EC(s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_8006BA08(void *);
extern s32 D_801A2550;
extern M2C_UNK func_80068FBC;
extern M2C_UNK func_8006B1E8;

void func_8006AFA0(void *arg0)
{
    u8 a = M2C_FIELD(arg0, u8 *, 0xC4);

    if (a == 0) {
        func_800506EC(4);
        func_800506EC(6);
        obj_find_by_id(5)->kill = 1;
        func_800506EC(8);
        obj_find_by_id(7)->kill = 1;
        func_800506EC(0xA);
        obj_find_by_id(9)->kill = 1;
        { s32 *p = &D_801A2550; *p |= 0x20; }
        func_8005C2D0(0x50, 0x32, 0x68, 0x90, 9);
        M2C_FIELD(arg0, void **, 0x74) = &func_8006B1E8;
        return;
    }
    if (a == 1) {
        M2C_FIELD(arg0, void **, 0x74) = &func_80068FBC;
    }
    func_8006BA08(arg0);
}
