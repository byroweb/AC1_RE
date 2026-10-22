#include "ac1/ac1.h"
/* ovl201 0x80084884 - print the picked entry's name and cost and the capacity left
 * after everything already fitted, with singular / plural labels */
extern u16 D_80039CA0;
extern u8 D_80031A58;
extern u8 D_800B9482[][56];
extern u8 D_800BB11C[][44];
extern u8 D_8017EF04[];
extern u8 D_8004C8B0[];
extern u8 D_8004C8C0[];
extern u8 D_8004C8D0[];
extern u8 D_8004C8E0[];
u8 func_80084364(UiObj *o);
void func_800506EC(s32 a);
void func_800847B4(u8 *buf, s32 n);
UiObj *func_8005D80C(s32 a, u8 *b, s32 c, s32 d, s32 e, s32 f);


void func_80084884(UiObj *o)
{
    u16 f;
    u8 k;
    u8 left;
    s16 i;

    f = D_80039CA0;
    k = func_80084364(o);
    func_800506EC(8);
    left = D_800B9482[D_80031A58][0];
    for (i = 0; i < 11; i++) {
        if (f & 1) {
            left -= D_800BB11C[i][0x28];
        }
        f >>= 1;
    }
    func_800847B4(D_8017EF04, D_800BB11C[k][0x28]);
    func_800847B4(D_8017EF04 + 3, left);
    func_8005D80C(1, &D_800BB11C[k][0x12], 6, 14, 4, 8);
    func_8005D80C(1, D_8017EF04, 6, 8, 14, 8);
    func_8005D80C(1, D_8017EF04 + 3, 6, 8, 24, 8);
    if (D_800BB11C[k][0x28] == 1) {
        func_8005D80C(1, D_8004C8B0, 6, 22, 14, 8);
    } else {
        func_8005D80C(1, D_8004C8C0, 6, 22, 14, 8);
    }
    if (left < 2) {
        func_8005D80C(1, D_8004C8D0, 6, 22, 24, 8);
    } else {
        func_8005D80C(1, D_8004C8E0, 6, 22, 24, 8);
    }
}
