#include "ac1/ac1.h"
#include "ac1/ovl202_camera.h"
#include "ac1/gfxframe.h"

typedef struct {
    SVECTOR pos;                /* +0x00 */
    SVECTOR rot;                /* +0x08 */
} Pose;

typedef struct {
    s32 ws0;                    /* +0x00 (0x801B4F60) world structure of view 0 */
    Pose p0;                    /* +0x04 */
    s32 ws1;                    /* +0x14 (0x801B4F74) */
    Pose p1;                    /* +0x18 */
} ViewSet;

typedef struct {
    u32 *ot[2];                 /* +0x00 the two viewport ordering tables */
    DRAWENV env[3];             /* +0x08 left, right, full screen */
} OtSet;

typedef struct {
    u8 b[64];
} DrEnv;

/* frame globals from 0x801ADCC4 */
typedef struct {
    OtSet *ots;                 /* +0x0000 (0x801ADCC4) */
    u32 unk4;
    PrimBuf *buf;               /* +0x0008 (0x801ADCCC) */
    u32 *ot;                    /* +0x000C (0x801ADCD0) */
    u8 unk10[0x64D4 - 0x10];
    u8 fade[4];                 /* +0x64D4 (0x801B4198) */
    u8 unk64d8[0x72C4 - 0x64D8];
    s32 ws;                     /* +0x72C4 (0x801B4F88) current world structure */
    Pose view;                  /* +0x72C8 (0x801B4F8C) */
} FrameG;

typedef struct {
    u8 unk0[0xE];
    s16 model;                  /* +0x0E */
    u8 unk10[0x60 - 0x10];
    s32 obj;                    /* +0x60 */
    u8 a;                       /* +0x64 */
    u8 b;                       /* +0x65 */
    u8 unk66[0x6E - 0x66];
    s16 c;                      /* +0x6E */
    u8 unk70[2];
    s16 d;                      /* +0x72 */
    s16 e;                      /* +0x74 */
    u8 f[0x170 - 0x76];         /* +0x76 */
} VsEnt;

typedef struct {
    u8 unk0[0x10];
    u16 btn;                    /* +0x10 */
    u8 unk12[0x254 - 0x12];
    u32 flags;                  /* +0x254 */
    u8 unk258[0x398 - 0x258];
    CamParm *cam;               /* +0x398 */
    u8 unk39c[0];
} PadRec;

extern ViewSet D_801B4F60;
extern FrameG D_801ADCC4;
extern VsEnt D_801AB748[2];
extern s32 D_801AB7A8;
extern s32 D_801AB918;
extern PadRec D_80040FA0[2];
extern PadRec *D_80039D18;
extern s32 D_800AD958;
extern u8 D_801B419B;
extern u16 D_801AB68A;
extern u16 D_801AB68C;
extern u32 D_801AB684;
extern u32 D_801B4114;
extern u8 D_80089F84[], D_80089F90[], D_80089F9C[], D_80089FA8[];
extern u8 D_80089FB4[], D_80089FB8[], D_80089FBC[], D_80089FC0[];
extern u8 D_80089FC4[], D_80089FD0[], D_80089FD4[], D_80089FE0[];
extern u8 D_80089FE4[], D_80089FF0[], D_80089FF4[], D_8008A000[];
extern u8 D_8008A004[], D_8008A010[], D_8008A014[], D_8008A020[];
extern u8 D_8008A024[], D_8008A030[], D_800894D4[];
extern s32 func_8006D65C(s32 ws, Pose *p);
extern void func_800614C8(void);
extern void func_80055E20(void);
extern void func_800561A8(void);
extern void func_800548B8(void);
extern void func_8005379C(s32 *o, s32 model, s32 a, s32 b, s32 c, s32 d, s32 e, void *f);
extern void func_80054E04(s32 obj);
extern void func_80054E6C(s32 obj, s32 a);
extern void func_8006176C(void);
extern void SetDrawEnv(DrEnv *p, DRAWENV *env);
extern void AddPrim(void *ot, void *p);
extern void func_8002B4A8(u32 *a, u32 *b, u32 *c);
extern void func_80050E60(void);
extern void func_8005D784(s32 x, s32 y, s32 w, s32 h, void *c, s32 z, s32 abe);
extern void func_80012670(char *buf, s32 v, s32 digits);
extern void func_8005145C(s32 x, s32 y, s32 a, s32 b, s32 c, char *s);
extern void func_8005FB00(void *a, void *b, s32 c, void *d, s32 e, s32 f);
extern void func_8005625C(void);
extern void func_800549B0(void);

#define G ((FrameG *)((u8 *)otp - 12))

/* 2-player frame: update both views, draw each half screen, then the shared HUD */
void func_80061AD4(Pose *p0, Pose *p1)
{
    char buf[12];
    VsEnt *e;
    DrEnv *d0;
    DrEnv *d1;
    DrEnv *d2;
    DrEnv *d3;
    s32 i;
    s32 t;
    s32 m;
    u32 fl;
    u8 *a;
    u8 *b;
    u8 c;
    u32 **otp;
    u32 *o;
    u8 *fp;
    PadRec *pf1;
    u32 mk;
    PadRec *pl;
    u8 *q;

    if (p0 != 0) {
        D_801B4F60.p0 = *p0;
    }
    if (p1 != 0) {
        D_801B4F60.p1 = *p1;
    }
    D_801B4F60.ws0 = func_8006D65C(D_801B4F60.ws0, &D_801B4F60.p0);
    D_801B4F60.ws1 = func_8006D65C(D_801B4F60.ws1, &D_801B4F60.p1);
    func_800614C8();
    func_80055E20();
    func_800561A8();
    func_800548B8();
    e = D_801AB748;
    for (i = 2; i != 0; e++, i--) {
        if (*(void **)e != 0) {
            func_8005379C(&e->obj, e->model + 128, e->a, e->c, e->d, e->b, e->e, e->f);
        }
    }
    d0 = (DrEnv *)D_801ADCC4.buf->cur;
    *(u8 **)((u8 *)D_801ADCC4.buf + 8) = (u8 *)(d0 + 1);
    d1 = (DrEnv *)D_801ADCC4.buf->cur;
    *(u8 **)((u8 *)D_801ADCC4.buf + 8) = (u8 *)(d1 + 1);
    d2 = (DrEnv *)D_801ADCC4.buf->cur;
    *(u8 **)((u8 *)D_801ADCC4.buf + 8) = (u8 *)(d2 + 1);
    d3 = (DrEnv *)D_801ADCC4.buf->cur;
    *(u8 **)((u8 *)D_801ADCC4.buf + 8) = (u8 *)(d3 + 1);
    mk = 0x10000;
    func_80054E04(D_801AB7A8);
    func_80054E04(D_801AB918);
    do { fp = (u8 *)&D_80040FA0[0].flags; } while (0);
    if (*(u32 *)fp & mk) {
        func_80054E6C(D_801AB7A8, *(s32 *)((u8 *)D_80040FA0[0].cam + 4));
    }
    o = D_801ADCC4.ots->ot[0];
    if ((s32)o < 0) {
        otp = &D_801ADCC4.ot;
        *otp = o;
    } else {
        otp = &D_801ADCC4.ot;
        *otp = o;
    }
    G->view = D_801B4F60.p0;
    G->ws = D_801B4F60.ws0;
    D_80039D18 = pl = (PadRec *)(fp - 0x254);
    func_8006176C();
    SetDrawEnv(d0, &G->ots->env[0]);
    AddPrim(G->ot + 8191, d0);
    do {
        if (*(u32 *)fp & mk) {
            func_80054E04(D_801AB7A8);
        }
    } while (0);
    if (D_80040FA0[1].flags & mk) {
        func_80054E6C(D_801AB918, *(s32 *)((u8 *)D_80040FA0[1].cam + 4));
    }
    G->ot = D_801ADCC4.ots->ot[1];
    G->view = D_801B4F60.p1;
    G->ws = D_801B4F60.ws1;
    pf1 = (PadRec *)(fp + 0x148);
    D_80039D18 = pf1;
    func_8006176C();
    SetDrawEnv(d1, &G->ots->env[1]);
    AddPrim(G->ot + 8191, d1);
    D_80039D18 = pl;
    D_800AD958 = 0;
    func_80050E60();
    D_80039D18 = pf1;
    D_800AD958 = 144;
    func_80050E60();
    if (D_801B419B != 0) {
        func_8005D784(0, 0, 320, 240, G->fade, 2, 2);
    }
    if (D_801AB68A >= 16 && D_801AB68C != 0) {
        t = (D_801AB68C - 1) * 100 / 22;
        m = t / 6000;
        func_80012670(buf, m, 2);
        func_80012670(buf + 3, t - m * 6000, 4);
        buf[2] = 65;
        buf[8] = 0;
        c = buf[5];
        buf[5] = 0;
        buf[7] = buf[6];
        buf[6] = c;
        func_8005145C(124, 18, 4, 240, 0, buf);
        func_8005145C(197, 26, 4, 128, 0, buf + 6);
    }
    func_8005FB00(D_80089FD0, D_80089FC4, 0, D_80089FC4 + 8, 4, 0);
    func_8005FB00(D_80089FE0, D_80089FD4, 0, D_80089FD4 + 8, 4, 0);
    fl = D_801AB684;
    if (fl & 0x1000) {
        if (D_801AB68A < 23) {
            func_8005FB00(D_80089FF0, D_80089FE4, 0, D_800894D4, 4, 2);
        } else {
            func_8005FB00(D_8008A000, D_80089FF4, 0, D_800894D4, 4, 2);
        }
    } else if ((D_80040FA0[0].btn & 0x30) == 0x30) {
        func_8005FB00(D_8008A010, D_8008A004, 0, D_800894D4, 4, 2);
    } else if (fl & 0x40) {
        func_8005FB00(D_8008A020, D_8008A014, 0, D_800894D4, 4, 2);
    } else if (fl & 0x80) {
        func_8005FB00(D_8008A030, D_8008A024, 0, D_800894D4, 4, 2);
    }
    func_8005FB00(D_80089FB4, D_80089F84, 0, D_80089F84 + 8, 4, 0);
    func_8005FB00(D_80089FB8, D_80089F90, 0, D_80089F90 + 8, 4, 0);
    func_8005FB00(D_80089FBC, D_80089F9C, 0, D_80089F9C + 8, 4, 0);
    func_8005FB00(D_80089FC0, D_80089FA8, 0, D_80089FA8 + 8, 4, 0);
    SetDrawEnv(d2, &D_801ADCC4.ots->env[2]);
    AddPrim(D_801ADCC4.ot + 5, d2);
    D_801ADCC4.ot = D_801ADCC4.ots->ot[0];
    SetDrawEnv(d3, &D_801ADCC4.ots->env[2]);
    AddPrim(D_801ADCC4.ot + 8191, d3);
    func_8002B4A8(D_801ADCC4.ots->ot[0], D_801ADCC4.ots->ot[1] + 8191, D_801ADCC4.ots->ot[1]);
    func_8005625C();
    func_800549B0();
    D_801B4114++;
}
