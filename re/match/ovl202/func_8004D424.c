#include "ac1/ac1.h"

typedef struct {
    s16 id;
    u8 pad[6];
} SndVab;

extern SndVab D_80041BD0[];
extern short SsUtKeyOnV(short voice, short vabId, short prog, short tone, short note, short fine, short voll, short volr);

void func_8004D424(s32 prog, s32 vl, s32 vr, s32 pitch, s32 vab)
{
    s32 note;

    pitch += 960;
    note = pitch >> 4;
    if (note < 0) {
        note = 0;
    } else if (note >= 128) {
        note = 127;
    }
    SsUtKeyOnV(23, D_80041BD0[vab].id, (prog >> 4) + 4, prog & 0xF, note, (pitch & 0xF) * 8, vl, vr);
}
