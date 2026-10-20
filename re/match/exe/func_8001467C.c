/* exe 0x8001467C  size 172  permuter tier (tools/loop/permute.py, decomp-permuter): from ctx draft, 2026-10-20 */
#include "ac1/ac1.h"
#include "m2c_compat.h"

void func_8001467C(void *arg0, void *arg1, void *arg2, void *arg3)
{
  s16 temp_a0;
  s16 temp_t3;
  s16 temp_v1;
  s32 temp_t0;
  s32 temp_t1;
  s32 temp_t2;
  s32 temp_v0;
  s32 temp_v0_2;
  s32 temp_v1_2;
  temp_t3 = *((s16 *) (((s8 *) arg0) + 2));
  temp_v1 = *((s16 *) (((s8 *) arg0) + 4));
  temp_t2 = (*((s16 *) (((s8 *) arg1) + 2))) - temp_t3;
  temp_v0 = (*((s16 *) (((s8 *) arg1) + 4))) - temp_v1;
  temp_t1 = (*((s16 *) (((s8 *) arg2) + 4))) - temp_v1;
  temp_t0 = (*((s16 *) (((s8 *) arg2) + 2))) - temp_t3;
  temp_a0 = *((s16 *) (((s8 *) arg0) + 0));
  temp_v1_2 = (*((s16 *) (((s8 *) arg2) + 0))) - temp_a0;
  temp_v0_2 = (*((s16 *) (((s8 *) arg1) + 0))) - temp_a0;
  *((s16 *) (((s8 *) arg3) + 0)) = (s16) (((s32) ((temp_t2 * temp_t1) - (temp_v0 * temp_t0))) >> 6);
  *((s16 *) (((s8 *) arg3) + 2)) = (s16) (((s32) ((temp_v0 * temp_v1_2) - (temp_v0_2 * temp_t1))) >> 6);
  *((s16 *) (((s8 *) arg3) + 4)) = (s16) (((s32) ((temp_v0_2 * temp_t0) - (temp_t2 * temp_v1_2))) >> 6);
}
