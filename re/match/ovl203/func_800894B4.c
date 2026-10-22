#include "ac1/ac1.h"
/* ovl203 0x800894B4 - ovl204 func_80089264 for mode 1: clear the other pilot's flag,
 * set 0x400, reset the current pilot's yaw into (-2048, 2048], func_80089420 */
typedef struct Pilot {
    u8 unk000[0x220];
    s16 yaw;                    /* +0x220 */
    s16 unk222;
    u8 unk224[0x30];
    u32 flags;                  /* +0x254 */
} Pilot;
typedef struct Flags32 {
    u32 w;
} Flags32;
extern Pilot *D_80039D18;
extern Pilot *D_80039D30;
extern Flags32 D_8019B48C;
s16 func_800132CC(s32 a, s32 b, s32 c);
void func_80089420();

extern Flags32 D_801A3D94;
void func_800894B4(void)
{
    s16 a;

    D_801A3D94.w = 0;
    D_80039D30->flags &= ~0x10000;
    D_8019B48C.w |= 0x400;
    a = func_800132CC(D_80039D18->yaw, 0, 8);
    D_80039D18->yaw = a;
    if (a > 2048) {
        D_80039D18->yaw = a - 4096;
    }
    D_80039D18->unk222 = 0;
    func_80089420();
}
