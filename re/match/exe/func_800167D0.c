#include "ac1/ac1.h"

typedef struct {
    u8 a;
    u8 b;
    u8 pad[0x26];
} SioSlot;      /* 0x28 */

typedef struct {
    s32 ev;             /* +0x000 */
    s32 ev2;            /* +0x004 */
    volatile s32 cnt;   /* +0x008 */
    u8 pad0C[0x30];
    s32 n;              /* +0x03C */
    s32 ev_a;           /* +0x040 */
    s32 ev_b;           /* +0x044 */
    u8 pad48[4];
    s32 ev_c;           /* +0x04C */
    SioSlot slot[16];   /* +0x050 */
    u8 *rd;             /* +0x2D0 */
    u8 *wr;             /* +0x2D4 */
    u8 pad2D8[0x10];
    s32 ev_d;           /* +0x2E8 */
    s32 ev_e;           /* +0x2EC */
    u8 pad2F0[4];
    s32 fd_r;           /* +0x2F4 */
    s32 fd_w;           /* +0x2F8 */
} SysCtl;

extern SysCtl D_80049F90;
extern u8 *D_8004A394;
extern u8 D_80043508[];
extern char D_80039C98[];
extern void func_80016220(void);
extern void func_8001585C(void);
extern void func_80015710(void);
extern void func_80015854(void);
extern void func_80015864(void);
extern void func_80030368(void);
extern s32 func_800309A8(s32 cmd, s32 arg, s32 param);
extern s32 OpenEvent(u32 desc, s32 spec, s32 mode, void (*func)(void));
extern s32 EnableEvent(s32 ev);
extern void SetRCnt(u32 spec, u16 target, s32 mode);
extern void StartRCnt(u32 spec);
extern s32 open(char *name, s32 mode);
extern s32 VSync(s32 mode);
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);

void func_800167D0(void)
{
    u8 param[16];
    SioSlot *s;
    s32 i;
    s32 ev;
    SysCtl *g;

    EnterCriticalSection();
    D_80049F90.cnt = 0;
    ev = OpenEvent(0xF2000003, 2, 0x1000, func_80016220);
    g = &D_80049F90;
    g->ev = ev;
    D_80049F90.ev2 = ev;
    EnableEvent(ev);
    s = g->slot;
    SetRCnt(0xF2000001, 0xFFFF, 0x2000);
    StartRCnt(0xF2000001);
    D_80049F90.n = 0;
    i = 0;
    do {
        i++;
        s->a = 0;
        s->b = 0;
        s++;
    } while (i < 16);
    D_80049F90.rd = D_80049F90.wr = (u8 *)&D_80049F90.slot[0];
    D_80049F90.ev_a = OpenEvent(0xF0000003, 0x8000, 0x1000, func_8001585C);
    D_80049F90.ev_b = OpenEvent(0xF0000003, 0x20, 0x1000, func_80015710);
    D_80049F90.ev_c = OpenEvent(0xF0000003, 0x40, 0x1000, func_80015854);
    EnableEvent(D_80049F90.ev_a);
    EnableEvent(D_80049F90.ev_b);
    EnableEvent(D_80049F90.ev_c);
    D_80049F90.ev_d = OpenEvent(0xF000000B, 0x400, 0x2000, 0);
    D_80049F90.ev_e = OpenEvent(0xF000000B, 0x8000, 0x1000, func_80015864);
    EnableEvent(D_80049F90.ev_d);
    EnableEvent(D_80049F90.ev_e);
    func_80030368();
    D_80049F90.fd_w = open(D_80039C98, 2);
    D_80049F90.fd_r = open(D_80039C98, 0x8001);
    func_800309A8(1, 3, 0x1FA400);
    func_800309A8(1, 4, 8);
    ExitCriticalSection();
    D_8004A394 = D_80043508;
    param[0] = 0x80;
    while (CdControl(0xE, param, 0) == 0) {
    }
    VSync(0);
    VSync(3);
}
