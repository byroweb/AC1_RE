#include "ac1/ac1.h"
/* ovl201 0x80050E94 - stop and close the current sequence and its VAB, all keys off,
 * free the sound buffers */
typedef struct SndCtl {
    s16 vab[2];                 /* +0x00 */
    s16 seq[2];                 /* +0x04 */
    u8 *a[2];                   /* +0x08 */
    u8 *b[2];                   /* +0x10 */
    u8 *c[2];                   /* +0x18 */
    u8 cur;                     /* +0x20 */
} SndCtl;
extern SndCtl D_801BC450;
void SsSeqStop(s16 id);
void SsSeqClose(s16 id);
void SsVabClose(s16 id);
void SsUtAllKeyOff(s32 mode);
void func_80051078(u8 *p);

void func_80050E94(void)
{
    s16 i;

    SsSeqStop(D_801BC450.seq[D_801BC450.cur]);
    SsSeqClose(D_801BC450.seq[D_801BC450.cur]);
    SsVabClose(D_801BC450.vab[D_801BC450.cur]);
    SsUtAllKeyOff(0);
    for (i = 0; i < 2; i++) {
        if (D_801BC450.a[i] != 0) {
            func_80051078(D_801BC450.a[i]);
            D_801BC450.a[i] = 0;
        }
        if (D_801BC450.b[i] != 0) {
            func_80051078(D_801BC450.b[i]);
            D_801BC450.b[i] = 0;
        }
        if (D_801BC450.c[i] != 0) {
            func_80051078(D_801BC450.c[i]);
            D_801BC450.c[i] = 0;
        }
    }
}
