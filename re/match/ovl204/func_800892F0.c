#include "ac1/ac1.h"
/* ovl204 0x800892F0 - run func_80089264 for both pilot records (0x39C bytes apart) */
typedef struct Flags32 {
    u32 w;
} Flags32;
extern u8 *D_80039D18;
extern u8 D_80040FA0[];
extern Flags32 D_801AB684;
extern Flags32 D_801B4104;
void func_80089264(void);

void func_800892F0(void)
{
    u8 *p;

    D_801B4104.w = 0;
    p = D_80040FA0;
    D_80039D18 = p;
    D_801AB684.w |= 0x2400;
    func_80089264();
    D_80039D18 = p + 0x39C;
    func_80089264();
}
