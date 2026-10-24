/* fdat044: mission-end sequence step, run once per frame.  Before frame 110
 * it sprays effects around the target point; after it the object sinks,
 * the screen fades (frame 158/159+) and the master volume ramps down (200+).
 * Retail writes +0xBA through the effect pointer before it is assigned
 * (s0 is read uninitialised); kept as is. */
#include "ac1/ac1.h"
typedef struct { u8 r, g, b, cd; } CVECTOR;
extern void SsSetMVol(s32 l, s32 r);
extern s32 rsin(s32 a);
typedef struct {
    u8 pad0[0x14];
    void (*think)();    /* +0x14 */
    u8 pad18[0x26 - 0x18];
    s16 h26;            /* +0x26 */
    u8 pad28[0xBA - 0x28];
    s16 hba;            /* +0xBA */
} Func801C67A0Fx;
typedef struct {
    s16 x, y, z;        /* +0x00 */
    u8 pad6[0x0E - 0x06];
    s16 sect;           /* +0x0E */
    u8 pad10[2];
    s16 h12;            /* +0x12 */
    u8 pad14[0xBA - 0x14];
    s16 hba;            /* +0xBA */
} Func801C67A0Obj;
extern CVECTOR D_801C4C9C;
extern s32 D_801CCA48;
extern s32 D_801AC81C;
extern u8 D_801AC8A0;
extern u8 D_801AC8A1;
extern u8 D_801AC8A2;
extern u8 D_801AC8A3;
extern u8 D_80041BBB;
extern s16 D_80198DAC;
extern s16 D_80198DAE;
extern s16 D_80198DB0;
extern s32 func_80014D78(s32 a);
extern s32 func_80014DAC(s32 a);
extern void func_80056B54(s32 a, Func801C67A0Obj *o, s32 b, s32 sect, s32 c, CVECTOR *col, s32 d);
extern s16 func_8006E6B4(s32 sect, void *pos);
extern Func801C67A0Fx *func_80086BC4(s32 a, SVECTOR *pos, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void func_801C6718();
void func_801C67A0(Func801C67A0Obj *o)
{
    CVECTOR c;
    SVECTOR p;
    Func801C67A0Fx *fx;
    s32 v;
    u8 *vol;

    c = D_801C4C9C;
    if (D_801CCA48 >= 110) {
        o->hba = 2000;
        o->z -= 750;
        o->sect = func_8006E6B4(o->sect, o);
        func_80056B54(0, o, 0, o->sect, 0xFFFF, &c, 5000);
        if (D_801CCA48 == 158) {
            D_801AC8A2 = 0;
            D_801AC8A1 = 0;
            D_801AC8A0 = 0;
        } else if (D_801CCA48 >= 159) {
            D_801AC8A3 = 1;
            v = D_801AC8A2 + 8;
            if (v >= 256)
                v = 255;
            D_801AC8A0 = D_801AC8A1 = D_801AC8A2 = v;
        }
        if (D_801CCA48 >= 200) {
            vol = &D_80041BBB;
            *vol -= 6;
            if (*vol > 128)
                *vol = 0;
            SsSetMVol(*vol, *vol);
        }
        if (o->x < -31000)
            o->h12 = 0;
    } else {
        fx->hba = (rsin(D_801AC81C << 8) + 1000) >> 2;
        p.vx = D_80198DAC + func_80014DAC(1500);
        p.vy = D_80198DAE + func_80014DAC(1500);
        p.vz = D_80198DB0 + func_80014D78(6000);
        fx = func_80086BC4(0, &p, 35, 20, 15, -1, 20);
        fx->think = func_801C6718;
        fx->h26 = 50;
    }
    D_801CCA48++;
}
