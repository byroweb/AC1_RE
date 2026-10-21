#include "ac1/ac1.h"
#include "ac1/sndbank.h"

extern SndBanks D_801BC450;
extern short SsVabOpenHead(unsigned char *addr, short vabid);
extern short SsVabTransBody(unsigned char *addr, short vabid);
extern void func_8005D500(UiObj *);

/* Open the next sound bank once nothing is being transferred. */
void func_8005D430(UiObj *o) {
    s32 i;
    s16 v;

    if (D_801BC450.busy[2] != 0) {
        return;
    }
    if (D_801BC450.busy[1] != 0) {
        return;
    }
    if (D_801BC450.busy[0] != 0) {
        return;
    }
    i = D_801BC450.which == 0;
    v = SsVabOpenHead(D_801BC450.head[i], -1);
    D_801BC450.vab[i] = v;
    SsVabTransBody(D_801BC450.body[i], v);
    o->update = func_8005D500;
}
