/* exe 0x80014C40  size 240  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
/* NOT MATCHED: 5/60 words at the tail: retail keeps d in v1 with v0 = -1 in the bnez slot then move v0,v1; ours d in v0 (sched1 hoists the -1) */
#include "ac1/ac1.h"

s32 func_80014C40(VECTOR *p, s32 x, s32 y, s32 z, s32 r, s32 up, s32 down)
{
  s32 dx;
  s32 dz;
  s32 d;
  dx = p->vx - x;
  if ((dx < (-r)) || ((d = dx) > r))
  {
    return -1;
  }
  dz = p->vz - z;
  if ((dz < (-r)) || (dz > r))
  {
    return -1;
  }
  if (y != 0xFFFF)
  {
    if (p->vy < y)
    {
      if (p->vy >= (y - down))
      {
        goto ok;
      }
      return -1;
    }
    else
      if (y < (p->vy - up))
    {
      return -1;
    }
  }
  ok:
  dx >>= 3;

  dz >>= 3;
  d = (4 * SquareRoot0((dx * dx) + (dz * dz))) * 2;
  if (r >= d)
  {
    return d;
  }
  else
  {
    dx = -1;
    return dx;
  }
}
