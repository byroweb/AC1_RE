#include "ac1/ac1.h"
#include "m2c_compat.h"

M2C_UNK func_8004E98C(void *, M2C_UNK, M2C_UNK, M2C_UNK); /* extern */
typedef struct {
    u16 flags;          /* +0x00 */
    u8 pad2[0x12];
    AcPos pos;          /* +0x14 */
} Func801C6528Cam;
extern Func801C6528Cam D_801987D8[];
extern AcPos D_801A26C0;
extern AcPos D_80039C78;
extern s32 D_8019F524[];
extern u16 D_8019F52A;
extern u16 D_801A26CA;
extern s16 D_801A26EA;
extern s8 D_801A26F9;
extern M2C_UNK func_801C5EA8;
extern M2C_UNK func_801C64AC;

void func_801C6528(void *arg0) {
    M2C_FIELD(arg0, s16 *, 0x36) = 1;
    M2C_FIELD(arg0, M2C_UNK **, 0x58) = &func_801C5EA8;
    M2C_FIELD(arg0, s32 *, 0x50) = 0;
    M2C_FIELD(arg0, s32 *, 0x54) = 0;
    M2C_FIELD(arg0, M2C_UNK **, 0x5C) = &func_801C64AC;
    M2C_FIELD(arg0, s8 *, 0x42) = 0;
    M2C_FIELD(arg0, s16 *, 0x44) = 0;
    func_8004E98C(arg0 + 0x64, 0, 0, 0);
    *(s16 *)&g_ac_entities[0].unk28[10] = 1;
    g_ac_entities[0].state = 0;
    g_ac_entities[0].yaw = M2C_FIELD(arg0, u16 *, 0x12);
    D_8019F52A = 0xFFFF;
    D_801987D8[0].flags |= 1;
    D_801987D8[0].pos = g_ac_entities[0].pos;
    D_8019F524[0] |= 0x2400;
    M2C_FIELD(arg0, AcPos *, 0xCC) = D_80039C78;
}
