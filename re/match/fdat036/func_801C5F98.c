/* fdat036 0x801C5F98  size 532  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
#include "ac1/ac1.h"

typedef struct {
    u8 pad0[8];
    s16 f8;
} Sub;
typedef struct {
    u8 pad0[4];
    Sub *sub;           /* +0x04 */
    u8 pad8[0xA];
    s16 ang;            /* +0x12 */
    u8 pad14[0x28];
    u8 mode;            /* +0x3C */
    u8 pad3d[4];
    u8 state;           /* +0x41 */
    u8 pad42[0xE];
    void (*fn)(void *, u8);     /* +0x50 */
    u8 pad54[0x78];
    u16 vx;             /* +0xCC */
    u16 ce;             /* +0xCE */
    u16 vz;             /* +0xD0 */
} Ent;
extern s32 SquareRoot0();
extern s16 func_800132CC();
extern void func_800143B8();
extern void func_80014728();
extern s32 func_8004F024();
extern void func_80075BC0();
extern s16 func_80075EB4();

void func_801C5F98(Ent *e)
{
  u16 v[4];
  s16 a;
  s16 r;
  s32 sum;
  s32 nz;
  func_80075BC0(e, v);
  if (((s16) v[1]) <= 0)
  {
    e->ce -= 12;
    if (((s16) e->ce) < (-128))
    {
      *((s16 *) (&e->ce)) = -128;
    }
  }
  else
  {
    e->ce += 12;
    if (((s16) e->ce) > 128)
    {
      *((s16 *) (&e->ce)) = 128;
    }
  }
  a = func_80075EB4(e);
  switch (e->mode)
  {
    case 2:
      calc:
    r = func_800132CC(e->ang, a, e->sub->f8);

      e->ang = r;
      func_80014728(r, v);
      v[0] = ((s16) v[0]) >> 1;
      v[2] = ((s16) v[2]) >> 1;
      e->vx += v[0];
      e->vz += v[2];
      sum = ((*((s16 *) (&e->vx))) * (*((s16 *) (&e->vx)))) + ((*((s16 *) (&e->vz))) * (*((s16 *) (&e->vz))));
      if (sum > 0x10000)
    {
      sum = SquareRoot0(sum);
      e->vx = (((s16) e->vx) << 8) / sum;
      e->vz = (((s16) e->vz) << 8) / sum;
    }
      break;

    case 3:
      a += 0x800;
      goto calc;

    case 4:
      a += 0x400;
      goto calc;

    case 5:
      a -= 0x400;
      goto calc;

    default:
      func_800143B8(((u8 *) e) + 0xCC, 0x19, 0x100, ((u8 *) e) + 0xCC);
      break;

  }

  if (func_8004F024(((u8 *) e) + 0x64, ((u8 *) e) + 8) == 0)
  {
    if (e->fn != 0)
    {
      e->fn(e, e->state);
    }
  }
}
