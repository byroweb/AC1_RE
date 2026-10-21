/* exe 0x80011F28  size 176  permuter tier (tools/loop/permute.py): levers bytes &D_8004A268 + E, from the ctx-fix-frame draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK CdInit();                                   /* extern */
M2C_UNK ChangeClearPAD();                    /* extern */
M2C_UNK ExitCriticalSection();                      /* extern */
M2C_UNK InitCARD();                          /* extern */
M2C_UNK InitPAD(); /* extern */
M2C_UNK StartCARD();                                /* extern */
M2C_UNK StartPAD();                                 /* extern */
M2C_UNK __main();                                   /* extern */
M2C_UNK _bu_init();                                 /* extern */
M2C_UNK func_800122A8();                            /* extern */
M2C_UNK func_80015538();     /* extern */
M2C_UNK memset();        /* extern */
extern M2C_UNK D_80039CC0;
extern M2C_UNK D_8004A268;

void func_80011F28(void) {
    __main();
    func_80015538(((s32) &D_80039CC0), 0, (s32) (0x801F8000 - ((s32) &D_80039CC0)) >> 2);
    CdInit();
    memset(&D_8004A268, 0, 0x10);
    InitPAD(&D_8004A268, 8, ((M2C_UNK *)((u8 *)&D_8004A268 + 8)), 8);
    InitCARD(0);
    StartCARD();
    _bu_init();
    StartPAD();
    ChangeClearPAD(0);
    ExitCriticalSection();
    func_800122A8();
}
