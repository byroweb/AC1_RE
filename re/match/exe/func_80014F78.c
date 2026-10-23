/* exe 0x80014F78  size 468  permuter tier (tools/loop/permute.py): decomp-permuter, from the file draft, 2026-10-22 */
/* NOT MATCHED: 3/117 words, disc register (retail a0, ours v0; preference list 2 3 4) */
#include "ac1/ac1.h"

s32 func_80014F78(s32 high, s32 dx, s32 dy, s32 g, s32 v, s32 *speed, s32 *angle)
{
  s32 v2;
  s32 g2;
  s32 q;
  s32 disc;
  s32 s;
  s32 b;
  s32 t1;
  s32 t2;
  s32 t;
  s32 sq;
  s32 x;
  s32 y;
  v2 = v * v;
  g2 = g * g;
  q = v2 >> 6;
  disc = ((q * q) - (((g2 * dx) >> 10) * (dx >> 2))) - (((g * q) >> 3) * (dy >> 2));
  if (disc < 0)
  {
    return -1;
  }
  s = SquareRoot0(disc << 4);
  s <<= 2;
  disc = s;
  b = (v2 >> 2) - ((dy * g) >> 2);
  t1 = ((b + disc) * 2) / g2;
  t2 = ((b - disc) * 2) / g2;
  if ((t1 <= 0) && (t2 <= 0))
  {
    return -1;
  }
  if (high == 0)
  {
    t = t2;
    if (t1 > 0)
    {
      if (!(t2 < t1))
      {
        t = t1;
      }
    }
  }
  else
  {
    t = t2;
    if (!(t1 < t2))
    {
      t = t1;
    }
  }
  sq = SquareRoot0(t << 14);
  *speed = sq;
  x = ((dx << 14) / v) / sq;
  y = (((dy + ((g * t) * 2)) << 12) / (v >> 1)) / (sq >> 1);
  *angle = -ratan2(y, x);
  return 0;
}
