#include "ac1/ac1.h"
/* ovl204 (port of ovl203 0x80052BF4) 0x80052244 - draw the target marker at the projected marker position when it is on screen.
 * sv: unused; the retail frame has an 8-byte slot below the VECTOR */
typedef struct Pilot {
    u8 unk000[0x2E];
    s16 marker;                 /* +0x2E */
    SVECTOR mpos;               /* +0x30 */
} Pilot;
typedef struct Pt16 {
    s16 x, y;
} Pt16;
extern Pilot *D_80039D18;
extern u8 D_8008AB18[];
extern u8 D_800894C4[];
void RotTrans(SVECTOR *v0, VECTOR *v1, s32 *flag);
void func_8005FB00(Pt16 *p, u8 *a, s32 b, u8 *c, s32 d, s32 e);


void func_80052244(s32 *on)
{
    SVECTOR sv;
    VECTOR v;
    s32 flag;
    s32 x;
    s32 y;

    if (*on != 0 && D_80039D18->marker != 0) {
        RotTrans(&D_80039D18->mpos, &v, &flag);
        if (v.vz > 0) {
            x = v.vx * 160 / v.vz;
            y = v.vy * 160 / v.vz;
            if ((u32)(x + 87) < 175 && (u32)(y + 68) < 137) {
                Pt16 pt;

                pt.x = x + 71;
                pt.y = y + 52;
                func_8005FB00(&pt, D_8008AB18, 0, D_800894C4, 16, 2);
            }
        }
    }
}
