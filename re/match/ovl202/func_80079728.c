#include "ac1/ac1.h"

typedef struct {
    u16 a;              /* 0x80041228 */
    u16 b;              /* 0x8004122A */
    u16 r;              /* 0x8004122C */
    u16 d;              /* 0x8004122E */
    u8 pad08[0x1C];
    u8 e;               /* 0x8004124C */
    u8 f;               /* 0x8004124D */
    u8 pad26[0x12];
    u8 tail[1];         /* 0x80041260 */
} RadarCfg;

extern RadarCfg D_80041228;
extern u16 D_8004122A;
extern u16 D_8004122C;
extern u16 D_8004122E;
extern u8 D_8004124C;
extern u8 D_8004124D;
extern void func_80082CAC(s32 a, s32 b, s32 r, s32 rr, s32 d, s32 e, s32 f, void *tail);

void func_80079728(void)
{
    func_80082CAC(D_80041228.a, D_8004122A, D_8004122C, D_8004122C * D_8004122C, D_8004122E,
                  D_8004124C, D_8004124D, D_80041228.tail);
}
