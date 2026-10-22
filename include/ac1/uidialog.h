/* include/ac1/uidialog.h - the overlay 201 yes/no dialog state at 0x801A2308
 * (TU ovl201.t57, func_800AE370 / func_800AE418 / func_800AE644), and the
 * pad words they read.
 * Field names from use: obj_id is the dialog's UiObj id, done is called when
 * it closes, vab is the SsUtKeyOn sound bank.  Include after ac1/ac1.h. */
#ifndef AC1_UIDIALOG_H
#define AC1_UIDIALOG_H

#include "types.h"

typedef struct UiDialog {
    s8 active;                  /* +0x00 */
    u8 unk01[3];
    u8 sel;                     /* +0x04 */
    u8 obj_id;                  /* +0x05 */
    u8 unk06[6];
    void (*done)(void);         /* +0x0C */
    s16 vab;                    /* +0x10 */
} UiDialog;

/* 0x801A2550: front-end pad words.  Read as a struct where a function
 * touches one word twice in a block (base register + 0 offsets); other
 * matched sources use the D_801A2568 / D_801A256C scalars. */
typedef struct UiPad {
    u32 unk00;                  /* +0x00 */
    u8 unk04[0x14];
    u32 trig;                   /* +0x18 0x801A2568 newly pressed */
    u32 held;                   /* +0x1C 0x801A256C */
} UiPad;

#endif
