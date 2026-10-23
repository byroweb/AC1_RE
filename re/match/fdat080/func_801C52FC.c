/* fdat080 0x801C52FC  size 716  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

typedef struct {
    u8 pad0[0x12];
    s16 ang;            /* +0x12 */
    u8 pad14[0x22];
    u16 f36;            /* +0x36 */
    u8 pad38[4];
    u8 mode;            /* +0x3C */
    u8 pad3d[4];
    u8 state;           /* +0x41 */
    u8 pad42[0xE];
    void (*fn)(void *, u8);     /* +0x50 */
    u8 pad54[0x78];
    u16 vx;             /* +0xCC */
    u16 ce;             /* +0xCE */
    u16 vz;             /* +0xD0 */
    s16 cnt;            /* +0xD2 */
} Ent;
extern s32 SquareRoot0();
extern void func_800143B8();
extern void func_80014728();
extern void func_8004C98C();
extern s32 func_8004F080();
extern void func_80075BC0();
extern void func_80075FC0();
extern void func_800784F8();
extern s32 D_801A5DC0;

void func_801C52FC(Ent *e)
{
  u16 v[4];
  s32 sum;
  s32 nz;
  func_800784F8(e, 0x190, 0x12C, 0x1F4);
  if (e->mode == 0x81)
  {
    if (e->cnt != 0)
    {
      e->cnt = e->cnt - 1;
      goto up;
    }
    e->f36 &= 0xFFEF;
  }
  func_80075FC0(e);
  func_80075BC0(e, v);
  if ((((s16) v[1]) <= 0) || ((*((s16 *) (&e->ce))) > 1200))
  {
    e->ce -= 0x40;
    if (((s16) e->ce) < (-2560))
    {
      *((s16 *) (&e->ce)) = -2560;
    }
  }
  else
  {
    up:
    e->ce += 0x100;

    if (((s16) e->ce) > 9600)
    {
      e->ce = 9600;
    }
  }
  switch (e->mode)
  {
    case 2:
      calc:
    func_80014728(*((s16 *) (((u8 *) e) + 0x12)), v);

      calc2:
    v[0] = ((s16) v[0]) >> 3;

      v[2] = ((s16) v[2]) >> 3;
      e->vx += v[0];
      e->vz += v[2];
      sum = ((*((s16 *) (&e->vx))) * (*((s16 *) (&e->vx)))) + ((*((s16 *) (&e->vz))) * (*((s16 *) (&e->vz))));
      if (sum > 0x6400000)
    {
      sum = SquareRoot0(sum, sum);
      e->vx = (((s16) e->vx) * 0x2800) / sum;
      e->vz = (((s16) e->vz) * 0x2800) / sum;
    }
      break;

    case 3:
      func_80014728((*((s16 *) (((u8 *) e) + 0x12))) + 0x800, v);
      goto calc2;

    case 4:
      func_80014728((*((s16 *) (((u8 *) e) + 0x12))) + 0x400, v);
      goto calc2;

    case 5:
      func_80014728((*((s16 *) (((u8 *) e) + 0x12))) - 0x400, v);
      goto calc2;

    default:
      func_800143B8(((u8 *) e) + 0xCC, 0x40, 0x2800, ((u8 *) e) + 0xCC);
      break;

  }

  func_8004C98C(0x80, D_801A5DC0 + 0x1000, ((u8 *) e) + 8, ((u8 *) e) + 0xCC, 0x64, 0x1F40, 0x2134, 0, 0x32);
  if (func_8004F080(((u8 *) e) + 0x64, ((u8 *) e) + 8) == 0)
  {
    if (e->fn != 0)
    {
      e->fn(e, e->state);
    }
  }
}
