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

/* next thread slot, wrapping after the eighth */
ScrThread *func_800523BC(ScrThread *t)
{
    t++;
    if (t == &D_801ED7A0.th[8]) {
        t -= 8;
    }
    return t;
}
