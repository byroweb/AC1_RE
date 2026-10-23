/* ovl202 0x8007863C  size 820  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

typedef struct {
    u32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 u0, v0;
    u16 clut;
    s16 x1, y1;
    u8 u1, v1;
    u16 tpage;
    s16 x2, y2;
    u8 u2, v2;
    u16 pad1;
    s16 x3, y3;
    u8 u3, v3;
    u16 pad2;
} PolyFT4;

typedef struct {
    s32 state;              /* 0x80040FA8 pause / menu state bits */
    u8 pad[0x248];
    s32 flags;              /* 0x800411F4 */
} PauseCtl;

typedef struct {
    s32 unk0;
    u8 env[0x18];           /* +0x04 DRAWENV */
    u8 isbg;                /* +0x1C (DRAWENV.isbg) */
    u8 pad[0x80 - 0x1D];
} EnvBuf;

extern PauseCtl D_80040FA8;
extern u16 D_801987D8[];
extern s32 D_801A5E18;
extern EnvBuf D_801A5E1C[2];
extern volatile u8 D_80041BC2[];
extern u16 D_80041BBE;
extern u8 D_80041BBB;
extern s32 D_80041BC4[];

extern void DrawSync(s32 mode);
extern void PutDrawEnv(void *env);
extern void DrawPrim(void *p);
extern void SsSetMVol(s16 l, s16 r);
extern s32 VSync(s32 mode);
extern s32 CdControl(u8 com, u8 *param, u8 *result);
extern s32 CdRead2(s32 mode);
extern s32 func_8001628C(void);
extern void func_80079524(void);
extern s32 func_80052A2C(s32 a);
extern void func_8004C318(s32 a);

void func_8007863C(void)
{
  PolyFT4 p;
  PolyFT4 *pp = &p;
  EnvBuf *env;
  s32 t0;
  if (D_80040FA8.state & 0x800)
  {
    D_80040FA8.flags &= ~0x10000;
    D_801987D8[0] &= ~1;
    DrawSync(0);
    env = &D_801A5E1C[0];
    if (D_801A5E18 == 0)
    {
      env = &D_801A5E1C[1];
    }
    env->isbg = 0;
    PutDrawEnv(env->env);
    env->isbg = 1;
    pp->r0 = 0;
    pp->g0 = 250;
    pp->b0 = 0;
    pp->clut = 0x7D10;
    pp->tpage = 30;
    pp->x0 = 121;
    pp->y0 = 101;
    pp->x1 = 199;
    pp->y1 = 101;
    pp->x2 = 121;
    pp->y2 = 139;
    pp->x3 = 199;
    pp->y3 = 139;
    pp->u0 = 49;
    pp->v0 = 201;
    pp->u1 = 127;
    pp->v1 = 201;
    pp->u2 = 49;
    pp->v2 = 239;
    pp->u3 = 127;
    pp->v3 = 239;
    ((u8 *) (&pp->tag))[3] = 9;
    pp->code = 0x2E;
    DrawPrim(pp);
    DrawSync(0);
    SsSetMVol(0, 0);
    while (D_80041BC2[0] == 0xFF)
    {
    }

    if (D_80041BBE != 0xFFFF)
    {
      t0 = func_8001628C();
      while (CdControl(9, 0, 0) == 0)
      {
      }

    }
    for (;;)
    {
      VSync(2);
      func_80079524();
      if (!(D_80040FA8.state & 0x800))
      {
        break;
      }
      if (D_80040FA8.state == 0x890)
      {
        D_80040FA8.flags |= 0x10000;
      }
      if (D_80040FA8.state == 0x860)
      {
        D_801987D8[0] |= 1;
      }
    }

    do
    {
      VSync(2);
      func_80079524();
      if (D_80040FA8.state == 0x100)
      {
        SsSetMVol(D_80041BBB, D_80041BBB);
        if (func_80052A2C(99) == 1)
        {
          g_mission_timer = 0;
          func_8004C318(0x100);
        }
        goto done;
      }
 do { } while (0);
    }
    while (!(D_80040FA8.state & 0x800));
    do
    {
      VSync(2);
      func_80079524();
    }
    while (D_80040FA8.state & 0x800);
    done:
    if (D_80041BBE != 0xFFFF)
    {
      while (CdRead2(200) == 0)
      {
      }

      D_80041BC4[0] += func_8001628C() - t0;
    }

    SsSetMVol(D_80041BBB, D_80041BBB);
    D_80040FA8.state = 0;
  }
}
