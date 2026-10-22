#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 r, g, b, code;           /* +0x00 */
    u8 pad04[0x4C];
    s32 unk50;                  /* +0x50 */
    s32 unk54;                  /* +0x54 */
    s32 unk58;                  /* +0x58 */
    s32 unk5C;                  /* +0x5C */
} LetterLine;

typedef struct LetterWork {
    u8 pad00[0x84];
    LetterLine line[10];        /* +0x84 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

void func_8009795C(s32 i)
{
    D_80180860.work->line[i].unk50 = 0;
    D_80180860.work->line[i].unk54 = 0;
    D_80180860.work->line[i].r = 0x40;
    D_80180860.work->line[i].g = 0x30;
    D_80180860.work->line[i].b = 0x3E;
    D_80180860.work->line[i].code = 0;
    D_80180860.work->line[i].unk58 = 2;
    D_80180860.work->line[i].unk5C = 0;
}
