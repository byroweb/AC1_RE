#include "ac1/ac1.h"

typedef struct LetterLine {
    u8 text[0x50];              /* +0x00 control codes + text */
    s32 pos;                    /* +0x50 read position, 0xFFFF = line done */
    s32 next;                   /* +0x54 */
    s32 len;                    /* +0x58 */
    s32 unk5C;                  /* +0x5C */
} LetterLine;

typedef struct LetterWork {
    u8 pad00[0x0A];
    u8 unkA;                    /* +0x0A */
    u8 pad0B[0x79];
    LetterLine line[10];        /* +0x84 */
    u8 pad444[0xDC];
    u8 text[1];                 /* +0x520 */
} LetterWork;

typedef struct { LetterWork *work; } LetterCtl;
extern LetterCtl D_80180860;

extern u32 D_801A2568;
extern u32 D_801A256C;
void func_8005BA2C();
void func_800506EC(s32);
void func_8005C2D0(s32, s32, s32, s32, s32);
void func_80097A08(LetterLine *l, u8 *src, s32 more);
void func_80097EE8(UiObj *o);

void func_80097D24(UiObj *o)
{
    s32 i;
    s16 sum;
    u8 *src;
    s32 fast;

    sum = 0;
    for (i = 1; i != -1; i--) {
        sum += D_80180860.work->line[i].next;
    }
    src = &D_80180860.work->text[sum];
    if (obj_find_by_handle(func_8005BA2C) == &g_ui_none) {
        if (D_80180860.work->unkA == 0) {
            fast = 0;
            if ((D_801A2568 & 0x40) || (D_801A256C & 0x40) || (D_801A2568 & 0x1000) ||
                (D_801A256C & 0x1000) || (D_801A2568 & 0x4000) || (D_801A256C & 0x4000)) {
                fast = 1;
            }
            for (i = 0; i < 2; i++) {
                if (D_80180860.work->line[i].pos != 0xFFFF) {
                    func_80097A08(&D_80180860.work->line[i], src, fast);
                    break;
                }
            }
        }
        if ((D_801A2568 & 0x20) && !(D_801A256C & 0x20)) {
            func_800506EC(12);
            obj_find_by_id(11)->kill = 1;
            func_8005C2D0(6, 76, 224, 48, 11);
            o->update = func_80097EE8;
        }
    }
}
