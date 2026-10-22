#include "ac1/ac1.h"

void func_80015A48(void);
void func_80015DC0(s32);

void func_80016328(u8 *loc, u32 *buf, s32 nsec)
{
    s32 tries;
    s32 r;
    s32 failed;

    func_80015A48();
    tries = 0;
    failed = 1;
    do {
        while (CdControl(2, loc, 0) == 0)
            ;
        while (CdRead(nsec, buf, 0x80) == 0)
            ;
        while ((r = CdReadSync(1, 0)) > 0)
            ;
        if (r == 0) {
            failed = 0;
            break;
        }
        tries++;
    } while (tries < 3);
    if (failed == 1)
        func_80015DC0(1);
}
