#include "ac1/ac1.h"

typedef struct {
    u16 busy;       /* +0x00 */
    s8 voice;       /* +0x02 */
    u8 pad[7];
} SndChan;

extern SndChan D_80041DE0[23];
extern void SpuGetAllKeysStatus(u8 *status);

void func_8004CE88(void)
{
    u8 *status = (u8 *)0x1F800000;
    s32 i;
    SndChan *c;

    SpuGetAllKeysStatus(status);
    c = D_80041DE0;
    i = 22;
    do {
        if (c->voice != -1 && c->busy == 0 && status[c->voice] == 0) {
            c->voice = -1;
        }
        c++;
    } while (--i != -1);
}
