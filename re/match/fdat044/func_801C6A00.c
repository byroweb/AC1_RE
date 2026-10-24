/* fdat044: mission-end script driver (frame task).  State D_801CCA40 plays
 * the radio lines (func_800529F0) on timer D_8019F52C thresholds; from
 * state 3 on it sprays six debris effects per frame around D_801D0E38
 * until the timer reaches 2, then spawns the end effect (func_801C67A0). */
#include "ac1/ac1.h"
typedef struct {
    u8 pad0[0x12];
    s16 life;           /* +0x12 */
    void (*think)();    /* +0x14 */
} Func801C6A00Fx;
extern s32 rand(void);
extern s32 rsin(s32 a);
extern s32 rcos(s32 a);
extern void func_80014774(SVECTOR *ang, SVECTOR *dir);
extern s32 func_80052338(void);
extern void func_800529F0(s32 a, s32 b, s32 c);
extern Func801C6A00Fx *func_80085A14(s32 a, SVECTOR *p, SVECTOR *q, s32 b, void *c, s32 d);
extern Func801C6A00Fx *func_80086BC4(s32 a, SVECTOR *pos, s32 b, s32 c, s32 d, s32 e, s32 f);
extern void func_8008A778(s32 a, s32 b, s32 c);
extern void func_8008A80C(void);
extern void func_8008C2FC(s32 a);
extern void func_801C67A0();
extern s32 D_8019F524;
extern u16 D_8019F528;
extern u16 D_8019F52C;
extern s32 D_801CCA40;
extern s32 D_801CCA44;
extern s32 D_801CCA48;
extern s32 D_801CCA4C;
extern s32 D_801CCA50;
extern s32 D_801CCA54[];
extern SVECTOR D_801D0E38;
void func_801C6A00(void)
{
    SVECTOR ang;
    s16 spare[4];
    SVECTOR p;
    SVECTOR q;
    s32 i;
    s32 r;
    s32 s;
    s32 k;
    u16 *tp;

    func_8008A80C();
    if (D_8019F528 != 0)
        return;
    switch (D_801CCA40) {
    case 0:
        if (D_801CCA44 == 44) {
            func_800529F0(0, 0, 48);
        } else if (D_801CCA44 == 154) {
            func_800529F0(0, 0, 47);
        } else if (D_801CCA44 == 286) {
            func_800529F0(0, 0, 50);
        } else if (D_801CCA44 >= 287 && func_80052338()) {
            func_8008A778(3960, 256, 139);
            D_801CCA40++;
        }
        D_801CCA44++;
        break;
    case 1:
        tp = &D_8019F52C;
        if (*tp < 5) {
            *tp = 0;
            func_800529F0(0, 0, 51);
            func_800529F0(0, 0, 52);
            D_801CCA40++;
        }
        break;
    case 2:
        if (func_80052338()) {
            func_8008A778(1320, 256, 139);
            D_801CCA40++;
        }
        break;
    case 3:
        if (D_8019F52C < 705) {
            func_800529F0(0, 0, 53);
            D_801CCA40++;
        }
        break;
    case 4:
        if (D_8019F52C < 265) {
            func_800529F0(0, 0, 54);
            D_801CCA40++;
        }
        break;
    case 5:
        if (D_8019F52C < 155) {
            func_800529F0(0, 0, 55);
            D_801CCA40++;
        }
        break;
    }
    if (D_801CCA40 >= 3) {
        if (D_8019F52C < 3) {
            if (D_8019F52C == 2) {
                D_801CCA48 = 0;
                func_80086BC4(0, &D_801D0E38, 35, 2000, 0, 0, 0)->think = func_801C67A0;
                func_8008C2FC(516);
                D_8019F528 = 1;
                D_8019F524 = 512;
            }
        } else if (D_8019F52C < 1321) {
            r = 1318 - D_801CCA4C;
            D_801CCA50 += D_801CCA4C >> 2;
            for (i = 6; i != 0; i--) {
                s = rsin(D_801CCA50);
                k = rcos(D_801CCA50);
                ang.vy = ((r * s) >> 13) + 2048;
                ang.vx = (r * k) >> 13;
                func_80014774(&ang, &p);
                q = p;
                p.vx = D_801D0E38.vx + (p.vx >> 4);
                p.vy = D_801D0E38.vy + (p.vy >> 4);
                p.vz = D_801D0E38.vz + (p.vz >> 4) - 2000;
                k = D_801CCA4C + (rand() & 0x7F);
                k = (k * k) >> 8;
                q.vx = D_801D0E38.vx + ((q.vx * k) >> 11);
                q.vy = D_801D0E38.vy + ((q.vy * k) >> 11);
                q.vz = D_801D0E38.vz + ((q.vz * k) >> 11) - 2000;
                func_80085A14(0, &p, &q, 4, &D_801CCA54[rand() & 3], 10)->life = 0;
                D_801CCA50 += 682;
            }
            D_801CCA4C++;
        }
    }
}
