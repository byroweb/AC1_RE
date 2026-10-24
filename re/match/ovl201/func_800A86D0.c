#include "ac1/ac1.h"

typedef struct LinkPkt {
    u8 tag;                     /* 'R' */
    u8 b1;
    u16 a;
    u16 b;
    u8 b6;
    u8 end;                     /* 'C' */
} LinkPkt;

typedef struct LinkIo {
    u8 pad[0x2F4];
    volatile s32 fd_r;                   /* 0x8004A284 */
    volatile s32 fd_w;                   /* 0x8004A288 */
    volatile LinkPkt rx;               /* 0x8004A28C */
    u8 pad304[4];
    LinkPkt tx;                 /* 0x8004A298 */
} LinkIo;

extern LinkIo D_80049F90;
extern volatile u8 D_8004A28C;
extern volatile u16 D_8004A28E;
extern volatile u16 D_8004A290;
extern volatile u8 D_8004A293;
extern volatile u8 D_8004A298;
extern volatile u16 D_8004A29A;
extern volatile u16 D_8004A29C;
extern volatile u8 D_8004A29F;
extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern u8 func_800A8618(void);
extern void func_800A4EF0(void);
extern void func_800A4F1C(void);
extern s32 func_800162A0(s32);
extern volatile s32 D_800BDCD0;
extern volatile u8 D_800BDCD4;

s32 func_800A86D0(u16 *src_, u16 *dst_, s32 n)
{
    u16 *src = src_;
    u16 *dst = dst_;
    LinkIo *io;
    s32 fd, t;

    VSync(0);
    if (func_800A8618() == 0) {
        return 0;
    }
    goto start;
fail:
    func_800309A8(2, 3, 0);
    func_800A4F1C();
    return 0;
start:
    func_800309A8(2, 0, 0);
    D_800BDCD4 = 0;
    func_800A4EF0();
    while (--n != -1) {
        io = &D_80049F90;
        fd = D_80049F90.fd_r;
        D_8004A293 = 0;
        D_8004A28C = D_8004A293;
        D_8004A298 = 'R';
        D_8004A29F = 'C';
        D_8004A29A = *src++;
        D_8004A29C = *src++;
        read(fd, (void *)&io->rx, 8);
        t = GetRCnt(0xF2000001);
        fd = D_80049F90.fd_w;
        D_800BDCD0 = t;
        write(fd, &io->tx, 8);
        if (D_800BDCD4 != 0) {
            goto fail;
        }
        D_800BDCD0 = GetRCnt(0xF2000001);
        while (D_80049F90.rx.tag != 'R' && D_80049F90.rx.end != 'C') {
            if (func_800162A0(D_800BDCD0) >= 7866) {
                goto fail;
            }
        }
        *dst++ = D_80049F90.rx.a;
        *dst++ = D_8004A290;
    }
    func_800A4F1C();
    func_800309A8(2, 0, 0);
    return 1;
}
