#include "ac1/ac1.h"

typedef struct {
    u8 unk0[0x10];
    s32 state;                  /* +0x10 */
} ScrThread;

/* mission script threads at 0x801ED7A0 */
typedef struct {
    ScrThread th[8];            /* +0x000 */
    ScrThread *cur;             /* +0x0A0 (0x801ED840) */
    ScrThread *last;            /* +0x0A4 */
    u8 unka8[8];
    u8 *blob;                   /* +0x0B0 (0x801ED850) script blob, relocated at init */
} ScrVM;

extern ScrVM D_801ED7A0;

extern void func_8005277C(s32 a, s32 b, u8 *code);

/* start script routine n of the blob (u16 offset table at its head) */
void func_800529F0(s32 a, s32 b, s32 n)
{
    func_8005277C(a, b, (u8 *)&D_801ED7A0.blob + ((u16 *)D_801ED7A0.blob)[n]);
}
