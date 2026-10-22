#include "ac1/ac1.h"
/* ovl204: draw a 2-state indicator (pilot flags bits 4/5) unless D_801AB684 bit 6 */
typedef struct Pilot {
    u8 unk000[0x10];
    u16 flags;                  /* +0x10 */
} Pilot;
typedef struct Pt16 {
    s16 x, y;
} Pt16;
extern Pilot *D_80039D18;
extern s32 D_801AB684;
extern Pt16 D_800894A4;
extern u8 D_800894A8[];
extern u8 D_800894B4[];
extern u8 D_800894D4[];
extern u8 D_800894D8[];
void func_8005FB00(Pt16 *p, u8 *a, s32 b, u8 *c, s32 d, s32 e);

void func_800513B4(void)
{
    if (!(D_801AB684 & 0x40)) {
        switch (D_80039D18->flags & 0x30) {
        case 0x10:
            func_8005FB00(&D_800894A4, D_800894A8, 0, D_800894D4, 14, 2);
            break;
        case 0x20:
            func_8005FB00(&D_800894A4, D_800894B4, 0, D_800894D8, 14, 2);
            break;
        }
    }
}
