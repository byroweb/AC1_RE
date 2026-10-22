#include "ac1/ac1.h"
/* ovl204 0x80089264 - reset the current pilot's yaw into (-2048, 2048] and clear the
 * flag bits, then func_800891D0 (which takes no argument). The flag words are
 * struct members: retail keeps their stores in source order. */
typedef struct Pilot {
    u8 unk000[0x220];
    s16 yaw;                    /* +0x220 */
    s16 unk222;
} Pilot;
typedef struct Flags32 {
    u32 w;
} Flags32;
extern Pilot *D_80039D18;
extern Flags32 D_800411F4;
extern Flags32 D_801B4104;
s16 func_800132CC(s32 a, s32 b, s32 c);
void func_800891D0();


void func_80089264(void)
{
    s16 a;

    D_801B4104.w = 0;
    D_800411F4.w &= ~0x10000;
    a = func_800132CC(D_80039D18->yaw, 0, 8);
    D_80039D18->yaw = a;
    if (a > 2048) {
        D_80039D18->yaw = a - 4096;
    }
    D_80039D18->unk222 = 0;
    func_800891D0();
}
