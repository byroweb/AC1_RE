/* ovl201 0x800531B8  size 1576  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

/* save/load menu work area (UiObj +0x7C) */
typedef struct {
    UiObj *owner;       /* +0x7C */
    UiObj *arg;         /* +0x80 */
    UiObj *box;         /* +0x84 */
    UiObj *icon[2];     /* +0x88 per card slot */
    u8 pad90[8];
    s32 status[2];      /* +0x98 per card slot */
    s8 pick;            /* +0xA0 slot cursor 0..1 */
    s8 menu;            /* +0xA1 */
    s8 top;             /* +0xA2 */
    u8 mode;            /* +0xA3 */
    u8 slot;            /* +0xA4 */
} SaveWork;

typedef struct {
    u32 used;           /* +0x00 buttons consumed */
    s16 delay;          /* +0x04 repeat delay */
    s16 rate;           /* +0x06 repeat rate */
    u8 pad8[0x10];
    u32 press;          /* +0x18 */
    u32 held;           /* +0x1C */
} PadState;
extern PadState D_801A2550;
extern s32 D_801A2574;
extern s16 D_801A2318;
extern s32 D_800B6FC4;
extern u8 D_8004B03C[];
extern u8 D_8004B058[];
extern u8 D_8004B078[];
extern u8 D_8004B094[];
extern void SsUtKeyOn(s16 a, s32 b, s32 c, s32 d, s32 e, s32 f, s32 g);
extern void func_800506EC(s32 id);
extern void func_8005C2D0(s32 x, s32 y, s32 w, s32 h, s32 id);
extern void func_8005DED8(UiObj *o, void *fn, void *msg, s32 n);
extern void func_8005E23C(UiObj *o, void *fn, void *msg, s32 n);
extern void func_80051DF0(UiObj *o, void *fn, void *msg, s32 n, s32 x, s32 y, s32 w, s32 h);
extern s32 func_800684A0(u8 slot);
extern s32 func_800685A8(u8 slot);
extern void func_800A2D0C(UiObj *o);
extern void func_8005571C(UiObj *o);
extern void func_80053924(UiObj *o);
extern void func_80053878(UiObj *o);
extern void func_80055C2C(UiObj *o);
extern void func_800537E0(UiObj *o);

void func_800531B8(UiObj *o)
{
  SaveWork *w = (SaveWork *) (&o->w);
  u8 n;
  if (!(D_800B6FC4 & 1))
  {
    w->status[0] = func_800685A8(0);
  }
  else
  {
    w->status[1] = func_800685A8(1);
  }
  D_800B6FC4++;
  if (D_801A2550.press & 0x1000)
  {
    D_801A2550.used |= 0x1000;
    if (D_801A2574 != 0)
    {
      D_801A2550.delay = 6 / D_801A2574;
      if (!(D_801A2550.held & 0x1000))
      {
        D_801A2550.rate = 12 / D_801A2574;
      }
    }
    else
    {
      D_801A2550.delay = 6;
      if (!(D_801A2550.held & 0x1000))
      {
        D_801A2550.rate = 12;
      }
    }
    SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
    if (w->pick == 0)
    {
      w->pick = 1;
    }
    else
    {
      w->pick--;
    }
  }
  else
    if (D_801A2550.press & 0x4000)
  {
    D_801A2550.used |= 0x4000;
    if (D_801A2574 != 0)
    {
      D_801A2550.delay = 6 / D_801A2574;
      if (!(D_801A2550.held & 0x4000))
      {
        D_801A2550.rate = 12 / D_801A2574;
      }
    }
    else
    {
      D_801A2550.delay = 6;
      if (!(D_801A2550.held & 0x4000))
      {
        D_801A2550.rate = 12;
      }
    }
    SsUtKeyOn(D_801A2318, 0, 1, 0x3C, 0, 0x7F, 0x7F);
    if (w->pick == 1)
    {
      w->pick = 0;
    }
    else
    {
      w->pick++;
    }
  }
  else
    if (D_801A2550.press & 0x40)
  {
    D_801A2550.used |= 0x40;
    SsUtKeyOn(D_801A2318, 0, 2, 0x3C, 0, 0x7F, 0x7F);
    if (w->pick == 0)
    {
      w->slot = 0;
    }
    else
    {
      w->slot = 1;
    }
    w->status[w->slot] = func_800684A0(w->slot);
    if (w->status[w->slot] == 1)
    {
      n = 3;
      do
      {
        w->status[w->slot] = func_800684A0(0);
      }
      while ((w->status[w->slot] == 1) && ((--n) != 0));
    }
    if (w->status[w->slot] == 1)
    {
      func_80051DF0(o, func_800531B8, D_8004B03C, 0xC, 0x14, 0x68, 0x118, 0x20);
      o->update = ui_obj_nop2;
      SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
      return;
    }
    if (w->status[w->slot] == 4)
    {
      func_8005E23C(o, func_8005571C, D_8004B058, 0xC);
      o->update = ui_obj_nop2;
      return;
    }
    else
      if (w->status[w->slot] == 2)
    {
      func_8005DED8(o, func_800531B8, D_8004B078, 0xC);
      o->update = ui_obj_nop2;
      SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
      return;
    }
    else
      if (w->status[w->slot] == 3)
    {
      func_8005DED8(o, func_800531B8, D_8004B094, 0xC);
      o->update = ui_obj_nop2;
      SsUtKeyOn(D_801A2318, 0, 7, 0x3C, 0, 0x7F, 0x7F);
      return;
    }
    else
      if (w->slot)
    {
      o->update = func_80053924;
    }
    else
    {
      o->update = func_80053924;
    }
  }
  else
    if (D_801A2550.press & 0x20)
  {
    D_801A2550.used |= 0x20;
    if (w->owner->ctor == func_800A2D0C)
    {
      SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
      func_800506EC(4);
      obj_find_by_id(3)->kill = 1;
      w->box->kill = 1;
      func_8005C2D0(0x60, 0x60, 0x80, 0x30, 3);
      o->update = func_80053878;
      return;
    }
    if (w->arg != 0)
    {
      SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
      func_800506EC(4);
      obj_find_by_id(3)->kill = 1;
      func_8005C2D0(0x60, 0x60, 0x80, 0x30, 3);
      o->update = func_80055C2C;
      return;
    }
    SsUtKeyOn(D_801A2318, 0, 0, 0x3C, 0, 0x7F, 0x7F);
    func_800506EC(4);
    obj_find_by_id(3)->kill = 1;
    func_8005C2D0(0x60, 0x60, 0x80, 0x30, 3);
    o->update = func_800537E0;
    return;
  }
  if ((w->status[0] != 1) && (w->status[0] != 2))
  {
    w->icon[0]->light = 0x15;
  }
  else
  {
    w->icon[0]->light = 0x10;
  }
  if ((w->status[1] != 1) && (w->status[1] != 2))
  {
    w->icon[1]->light = 0x15;
  }
  else
  {
    w->icon[1]->light = 0x10;
  }
}
