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

extern void func_8005239C();

/* relocate the script blob, clear the eight threads and start thread 0 */
void func_800523DC(void)
{
    ScrThread *t;
    s32 i;

    i = 8;
    D_801ED7A0.blob += (s32)&D_801ED7A0.blob;
    t = D_801ED7A0.th;
    for (; i != 0; i--) {
        t->state = 0;
        t++;
    }
    D_801ED7A0.cur = D_801ED7A0.last = D_801ED7A0.th;
    func_8005239C(0);
}
