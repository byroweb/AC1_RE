#include "ac1/ac1.h"

typedef struct {
    s16 tracks[16];
    u8 loc[4];
} CdaTrackRow;

typedef struct {
    u16 track;          /* 0x80041BBE */
    u8 playing;         /* 0x80041BC0 */
    u8 flags;           /* 0x80041BC1 */
    volatile u8 state;  /* 0x80041BC2 */
} CdaState;

extern s32 CdControl(u8 com, u8 *param, u8 *result);
extern void *CdSyncCallback(void *func);
extern void func_8004D948(s32 a);
extern void func_8004DA1C(u8 status);
extern CdaTrackRow D_80031764[];
extern u8 D_80041BBA;
extern u8 D_80041BBD;
extern CdaState D_80041BBE;
extern void *D_8008D924;

void func_8004DB18(s32 id)
{
    u8 filter[8];
    s32 idx;
    s32 hi;
    CdaTrackRow *row;

    D_80041BBE.track = id;
    D_80041BBE.flags = 0;
    if (id == 0xFFFF) {
        D_80041BBE.playing = 0;
        return;
    }
    if (D_80041BBE.state == 0xFF) {
        do {
        } while (D_80041BBE.state == 0xFF);
    }
    hi = id >> 4;
    idx = id & 0xF;
    row = &D_80031764[hi];
    if (row->tracks[idx] & 0x8000) {
        D_80041BBE.flags |= 1;
    }
    filter[0] = 1;
    filter[1] = idx;
    D_80041BBE.state = 0xFF;
    func_8004D948(0);
    while (CdControl(0xD, filter, 0) == 0) {
    }
    while (CdControl(2, row->loc, 0) == 0) {
    }
    if (D_8008D924 == 0) {
        D_8008D924 = CdSyncCallback(func_8004DA1C);
    }
    while (CdControl(0x16, row->loc, 0) == 0) {
    }
    if (row->tracks[idx] & 0x2000) {
        D_80041BBD = 127;
    } else {
        D_80041BBD = D_80041BBA;
    }
    D_80041BBE.playing = 1;
}
