#include "ac1/ac1.h"
/* ovl201 0x800517C8 - is the queue busy: 1 unless read == write and the slot is empty.
 * The write pointer is volatile (re-read), the result a u8. */
extern u8 *D_8004A260;
extern u8 *volatile D_8004A264;
void EnterCriticalSection(void);
void ExitCriticalSection(void);

s32 func_800517C8(void)
{
    u8 r;

    EnterCriticalSection();
    r = 1;
    if (D_8004A264 == D_8004A260) {
        r = *D_8004A264 != 0;
    }
    ExitCriticalSection();
    return r;
}
