#include "ac1/ac1.h"

typedef struct MailItem {
    u8 unk0[2];
    u8 visible;
} MailItem;

typedef struct MailView {
    u8 pad0[0x3C];
    MailItem *items[1];
} MailView;

typedef struct MailState {
    u8 flags[64];
} MailState;

extern MailState D_8017F6D0;
extern MailView *D_80180860;

void func_800971AC(u8 mail, u8 slot)
{
    if (!(D_8017F6D0.flags[mail] & 2)) {
        D_80180860->items[slot]->visible = 0;
    } else {
        D_80180860->items[slot]->visible = 1;
    }
}
