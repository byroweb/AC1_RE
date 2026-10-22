#include "ac1/ac1.h"

typedef struct {
    s32 w[0x39C / 4];
} PilotBlk;

typedef struct {
    u8 unk0[0x2F4];
    s32 fd_r;                   /* +0x2F4 */
    s32 fd_w;                   /* +0x2F8 */
    u8 rx[8];                   /* +0x2FC = 0x8004A28C */
    u8 pad304[4];
    u8 tx[8];                   /* +0x308 = 0x8004A298 */
} LinkState;

extern LinkState D_80049F90;
extern s32 D_8004A284;
extern s32 D_8004A288;
extern volatile u8 D_8004A28C;
extern volatile u8 D_8004A28D;
extern volatile u8 D_8004A293;
extern volatile u8 D_8004A298;
extern volatile u8 D_8004A299;
extern volatile u8 D_8004A29F;
extern volatile s32 D_8004A290;
extern volatile s32 D_8004A294;
extern volatile s32 D_8004A29C;
extern volatile s32 D_8004A2A0;
extern volatile s32 rx_w;          /* 0x8004A28C as a word */
extern volatile s32 tx_w;          /* 0x8004A298 as a word */
extern volatile s32 D_800895C4;
extern volatile u8 D_800895C8;
extern PilotBlk *D_80039D30;
extern PilotBlk *D_80039D34;
extern PilotBlk D_80040FA0[2];
extern u8 D_8019B481;
extern s32 read(s32 fd, void *buf, s32 n);
extern s32 write(s32 fd, void *buf, s32 n);
extern s32 func_8001628C(void);
extern s32 func_80016278(s32 t);
extern s32 func_800309A8(s32 a, s32 b, s32 c);
extern s32 func_8004BEB0(void);
extern void func_8004F3C4(u16 *src, u16 *dst, s32 n);
extern s32 rand(void);
extern void VSync(s32 n);

void func_8004F4E0(void)
{
retry:
    D_8004A28D = 0;
    D_8004A28C = D_8004A293 = 0;
    D_8004A299 = 'S';
    D_8004A298 = 'R';
    D_8004A29F = 'C';
    read(D_8004A284, (void *)&D_8004A28C, 8);
    D_800895C4 = func_8001628C();
    while (func_800309A8(3, 1, 0) == 0) {
        if (func_80016278(D_800895C4) >= 1800) {
            D_800895C8 = 1;
            return;
        }
        D_8004A299 = 'M';
    }
    D_800895C4 = func_8001628C();
    write(D_80049F90.fd_w, D_80049F90.tx, 8);
    if (D_800895C8 != 0 || func_8004BEB0() == 0) {
        return;
    }
    if ((D_8004A28D != 'M' && D_8004A28D != 'S') || D_8004A28D == D_8004A299) {
        VSync((rand() & 0xF) + 3);
        goto retry;
    }
    if (D_8004A299 == 'M') {
        D_80039D30 = &D_80040FA0[0];
        D_80039D34 = &D_80040FA0[1];
        D_8019B481 = 1;
    } else {
        D_80039D30 = &D_80040FA0[1];
        D_80039D34 = &D_80040FA0[0];
        D_80040FA0[1] = D_80040FA0[0];
        D_8019B481 = 0;
    }
    func_8004F3C4((u16 *)&D_80039D30->w[0x95], (u16 *)&D_80039D34->w[0x95], 0x4F);
    rx_w = D_8004A290 = D_8004A294 = tx_w = D_8004A29C = D_8004A2A0 = 0;
    D_8004A28C = 'R';
    D_8004A293 = 'C';
}
