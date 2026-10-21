#include "ac1/ac1.h"

typedef struct Part7C {
    u8 pad00[0x12];
    s16 ang;                    /* +0x12 */
    u8 pad14[0x12];
    u16 flags;                  /* +0x26 */
    u8 pad28[0x20];
    struct Part7C *target;      /* +0x48 */
    u8 pad4c[0x30];
} Part7C;

void func_80053950(u16 *p, Part7C *parts) {
    s32 n;
    s32 cmd;
    s32 sub;
    Part7C *part;
    s32 v;

    n = *p++;
    while (--n != -1) {
        cmd = *p++;
        {
            s32 idx = cmd >> 8;

            sub = (cmd >> 4) & 0xF;
            part = &parts[idx];
        }
        cmd &= 0xF;
        switch (cmd) {
        case 2:
            part->ang = *p++;
            break;
        case 3:
            v = *p++;
            if (v != 0xFFFF) {
                part->target = &parts[v];
            }
            break;
        case 8:
            switch (sub) {
            case 0:
                part->ang = -2;
                part->target = 0;
                part->flags &= ~3;
                break;
            case 1:
                part->ang = -1;
                part->flags = 0;
                break;
            }
            break;
        }
    }
}
