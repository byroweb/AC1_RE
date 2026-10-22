#include "ac1/ac1.h"
/* ovl201 0x800511A4 - read nsec sectors at loc into buf (CdlSetloc + CdRead, up to 3 tries) */
s32 CdControl(u8 com, u8 *param, u8 *result);
s32 CdRead(s32 sectors, u32 *buf, s32 mode);
s32 CdReadSync(s32 mode, u8 *result);
void func_80015A48(void);

void func_800511A4(u8 *loc, u32 *buf, s32 nsec)
{
    s32 tries;
    s32 r;

    func_80015A48();
    tries = 0;
    do {
        while (CdControl(2, loc, 0) == 0) {
        }
        while (CdRead(nsec, buf, 0x80) == 0) {
        }
        while ((r = CdReadSync(1, 0)) > 0) {
        }
        tries++;
    } while (r != 0 && tries < 3);
}
