#include "ac1/ac1.h"

typedef struct {
    u8 unk0[0x0C];
    u8 c;                       /* +0x0C bit 0: window open */
    u8 unkd[7];
} ScrThread;

/* mission script threads at 0x801ED7A0 */
typedef struct {
    ScrThread th[8];            /* +0x000 */
    ScrThread *cur;             /* +0x0A0 */
    ScrThread *last;            /* +0x0A4 (0x801ED844) */
    u8 unka8[8];
    u8 *blob;                   /* +0x0B0 (0x801ED850) */
} ScrVM;

extern s32 func_80016A84(s32 pad);
extern void func_8004CFC8(s32 sfx, s32 vol);
extern s32 func_80052338(void);
extern void func_8005277C(s32 a, s32 b, u8 *code);
extern void func_80062BD8(s32 a);
extern ScrVM D_801ED7A0;

/* run script routine n as a yes/no prompt: 1 for yes (0x40), 0 for no
 * (0x20); waits for the buttons to be released and the window to close */
s32 func_80052A2C(s32 n)
{
    s32 prev;
    s32 cur;
    s32 r;
    s32 old;

    func_8005277C(7, 100, (u8 *)&D_801ED7A0.blob + ((u16 *)D_801ED7A0.blob)[n]);
    prev = func_80016A84(1);
    for (;;) {
        func_80062BD8(0);
        cur = func_80016A84(1);
        old = prev;
        prev = cur;
        if (old != 0) {
            continue;
        }
        if (prev & 0x40) {
            func_8004CFC8(55, 100);
            r = 1;
            break;
        }
        if (prev & 0x20) {
            func_8004CFC8(54, 100);
            r = 0;
            break;
        }
    }
    while (func_80016A84(1) != 0) {
        func_80062BD8(0);
    }
    D_801ED7A0.last->c &= ~1;
    do {
        func_80062BD8(0);
    } while (func_80052338() == 0);
    return r;
}
