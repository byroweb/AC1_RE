#include "ac1/ac1.h"
/* ovl202 0x8004EDAC - step a sound-event sequence at pos. n == 0xFF: play the
 * current event as a 3D effect from the bank (key * 256 into the sound buffer);
 * else, when the frame counter reaches the event's key, play it (plain sound
 * for d < 64, else an effect with range/len from d - 64) and advance. */

typedef struct {
    u8 key;     /* +0 frame or bank slot */
    u8 id;      /* +1 */
    u8 vol;     /* +2 */
    s8 d;       /* +3 <64: pitch; else distance class + 64 */
} SndEv;

typedef struct {
    u8 pad0[0xA];
    s16 frame;          /* +0x0A */
    u8 padC[0x54 - 0xC];
    u8 n;               /* +0x54 events, 0xFF = bank effect */
    u8 idx;             /* +0x55 */
    SndEv ev[1];        /* +0x56 */
} SndSeq;

extern s32 D_801A5DC0;
extern void func_8004CC30(s32 id, s32 buf, SVECTOR *pos, s32 vol, s32 range, s32 len, s32 pitch);
extern void func_8004CE44(s32 id, SVECTOR *pos, s32 vol, s32 d);

void func_8004EDAC(SndSeq *s, SVECTOR *pos)
{
    SndEv *e;
    s32 t;

    if (s->n == 0) {
        return;
    }
    e = &s->ev[s->idx];
    if (s->n == 0xFF) {
        if (e->d < 64) {
            func_8004CC30(e->id, (e->key << 8) + 0x1000 + D_801A5DC0, pos, e->vol, 8000, 10000, e->d);
        } else {
            t = e->d - 64;
            func_8004CC30(e->id, (e->key << 8) + 0x1000 + D_801A5DC0, pos, e->vol, t * 250, (t * 625) >> 1, 0);
        }
    } else if (s->frame == e->key) {
        if (e->d < 64) {
            func_8004CE44(e->id, pos, e->vol, e->d);
        } else {
            t = e->d - 64;
            func_8004CC30(e->id, 0, pos, e->vol, t * 250, (t * 625) >> 1, 0);
        }
        s->idx++;
        if (s->idx >= s->n) {
            s->idx = 0;
        }
    }
}
