/* fdat016 0x801C62CC  size 1040  permuter tier (tools/loop/permute.py): decomp-permuter after levers swap M2C_FIELD(e, u16 *, 0x10 / e->pos.z = ws->org[2];, from the file draft, 2026-10-22 */
/* NOT MATCHED: 36/260 words; only the tail block differs: retail loads q=(s16)+0x108 once (lh v0, move a1,v0 for the subtract), ours reloads it as lhu; also needs the pad[4] + s16 q for the 80-byte frame */
#include "ac1/ac1.h"
#include "m2c_compat.h"

typedef struct {
    s16 x, y, z;        /* +0x00 */
    u8 pad6[4];
    u16 ha;             /* +0x0A */
    u8 pad0c[2];
    u16 he;             /* +0x0E */
    u8 pad10[0x0C];
    u16 flags;          /* +0x1C */
    u8 pad1e[0x16];
} Slot34;               /* 0x34 */
typedef struct {
    u8 pad0[8];
    s16 vx;             /* +0x08 */
    s16 pad0a;
    s16 vz;             /* +0x0C */
} Hit;

extern s32 func_80014D78(s32);
extern void func_80014728(s32, void *);
extern void func_8004CC30(s32, s32, void *, s32, s32, s32, s32);
extern void func_8006D39C(WorldStruct *);
extern s16 func_8006E6B4(s32, void *);
extern void func_80072548(WorldStruct *);
extern void func_80078E1C(void *);
extern void func_80086854(s32, s16 *, void *);
extern void func_800869F0(s32, void *, s32, s32, void *);
extern void func_800873F8(WorldStruct *, void *, s32, s32, s32, s32);
extern Slot34 D_80198B08[];
extern u8 D_801CDC6C[];
extern u8 D_801CDC70[];

void func_801C62CC(AcEntity *e)
{
  WorldStruct *ws;
  Slot34 *sl;
  Hit *h;
  s16 pad[4];
  s16 v[3];
  u16 t;
  u16 *new_var;
  s16 t16;
  s32 q32;
  if (e->ai_mode != 0xFF)
  {
    ws = &g_world_structs[*((u16 *) (((s8 *) e) + 0xFC))];
    if (e->ai_mode == 0)
    {
      func_800869F0(0, &e->pos, 400, 8, D_801CDC6C);
      func_800873F8(ws, &e->pos, 30, 20, 40, 0);
      sl = &D_80198B08[*((u8 *) (((s8 *) e) + 0xFF))];
      sl->flags &= 0x7FFF;
      sl->x = e->pos.x;
      sl->y = e->pos.y;
      sl->z = e->pos.z;
      sl->ha = e->yaw;
      sl->he = func_8006E6B4(0, &e->pos);
      kill:
      func_80078E1C(e);

      func_8006D39C(ws);
      return;
    }
    if (e->ai_mode & 1)
    {
      v[0] = (e->pos.x + func_80014D78(800)) - 400;
      v[2] = (e->pos.z + func_80014D78(800)) - 400;
      v[1] = e->pos.y - func_80014D78(200);
      func_80086854(0, v, D_801CDC70);
      func_800873F8(ws, &e->pos, 30, 10, 5, 0);
    }
    e->ai_mode--;
  }
  switch (e->unk42[0])
  {
    case 0:
      t16 = *((s16 *) (((s8 *) e) + 0x10A));
      if (t16 <= 0)
    {
      e->unk42[0] = 1;
      *((s16 *) (((s8 *) e) + 0x108)) = 0;
      *((s16 *) (((s8 *) e) + 0x10C)) = 440;
    }
    else
      if (t16 < 2048)
    {
      *((s16 *) (((s8 *) e) + 0x108)) = ((t16 + 51) * 5) >> 8;
    }
      *((u16 *) (((s8 *) e) + 0x10C)) = (*((u16 *) (((s8 *) e) + 0x10C))) + 1;
      break;

    case 1:
      t16 = (*((u16 *) (((s8 *) e) + 0x10C))) - 1;
      *((u16 *) (((s8 *) e) + 0x10C)) = t16;
      if (t16 == 0)
    {
      e->unk42[0] = 2;
    }
      break;

    case 2:
      if ((((u16) (e->pos.x + 30000)) > 0xEA60) || (((u16) (e->pos.z + 30000)) > 0xEA60))
    {
      ws = &g_world_structs[*((u16 *) (((s8 *) e) + 0xFC))];
      goto kill;
    }
      new_var = (u16 *) (((s8 *) e) + 0x10C);
      if (((*new_var)++) & 1)
    {
      *((u16 *) (((s8 *) e) + 0x108)) = (*((u16 *) (((s8 *) e) + 0x108))) + 1;
      if ((*((s16 *) (((s8 *) e) + 0x108))) >= 41)
      {
        *((s16 *) (((s8 *) e) + 0x108)) = 40;
      }
    }
      break;

    case 3:
      *((u16 *) (((s8 *) e) + 0x108)) = (*((u16 *) (((s8 *) e) + 0x108))) - 2;
      if ((*((s16 *) (((s8 *) e) + 0x108))) < 0)
    {
      *((s16 *) (((s8 *) e) + 0x108)) = 0;
    }
      break;

  }

  t16 = *((s16 *) (((s8 *) e) + 0x108));
  if (t16 > 0)
  {
    ws = &g_world_structs[*((u16 *) (((s8 *) e) + 0xFC))];
    *((u16 *) (((s8 *) e) + 0x10A)) = (*((u16 *) (((s8 *) e) + 0x10A))) - t16;
    h = (Hit *) ws->on_hit;
    func_80014728(e->yaw, &h->vx);
    h->vx = (h->vx * (*((s16 *) (((s8 *) e) + 0x108)))) >> 12;
    h->vz = (h->vz * (*((s16 *) (((s8 *) e) + 0x108)))) >> 12;
    func_80072548(ws);
    e->pos.x = ws->org[0];
    t = (*((u16 *) (((s8 *) e) + 0x10E))) - (*((s16 *) (((s8 *) e) + 0x108)));
    e->pos.z = ws->org[2];
    *((u16 *) (((s8 *) e) + 0x10E)) = t;
    if (((s16) t) <= 0)
    {
      *((u16 *) (((s8 *) e) + 0x10E)) = t + 800;
      func_8004CC30(0x8B, 0, &e->pos, 120, 12000, 13000, 0);
    }
  }
}
