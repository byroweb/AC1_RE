#include "ac1/ac1.h"
#include "ac1/sndbank.h"

extern SndBanks D_801BC450;
extern u8 D_801BC44C;
extern u8 D_80048662;
extern void SsSeqStop(short);
extern void SsSeqClose(short);
extern void SsVabClose(short);
extern short SsSeqOpen(unsigned long *addr, short vab);
extern void SsSeqSetVol(short, short, short);
extern void SsSeqPlay(short, char, int);
extern void func_80051078(void *p);

/* Swap to the other music slot: close the playing one, open and play the next. */
void func_8005D544(UiObj *o)
{
    s16 next;
    u8 nw;
    s32 i;
    s16 v;
    s16 *sp;

    next = D_801BC450.which == 0;
    nw = next;
    if (D_801BC450.head[D_801BC450.which] != 0) {
        SsSeqStop(D_801BC450.seq[D_801BC450.which]);
        SsSeqClose(D_801BC450.seq[D_801BC450.which]);
        SsVabClose(D_801BC450.vab[D_801BC450.which]);
        func_80051078(D_801BC450.head[D_801BC450.which]);
        func_80051078(D_801BC450.body[D_801BC450.which]);
        func_80051078(D_801BC450.seqdat[D_801BC450.which]);
        D_801BC450.head[D_801BC450.which] = 0;
        D_801BC450.body[D_801BC450.which] = 0;
        D_801BC450.seqdat[D_801BC450.which] = 0;
    }
    i = next;
    v = SsSeqOpen((unsigned long *)D_801BC450.seqdat[i], D_801BC450.vab[i]);
    sp = &D_801BC450.seq[i];
    *sp = v;
    SsSeqSetVol(v, D_80048662 * 2, D_80048662 * 2);
    SsSeqPlay(*sp, 1, 0);
    D_801BC450.which = nw;
    D_801BC44C = 1;
    D_801BC450.unk24 = 0;
    o->kill = 1;
}
