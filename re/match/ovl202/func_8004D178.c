#include "ac1/ac1.h"

typedef struct {
    u16 busy;       /* +0x00 key | 0x8000 */
    s8 voice;       /* +0x02 */
    u8 vol;         /* +0x03 */
    u8 note;        /* +0x04 */
    u8 fine;        /* +0x05 */
    s16 id;         /* +0x06 */
    s16 prio;       /* +0x08 */
} SndChan;

typedef struct {
    s16 vab;        /* +0x00 */
    s16 prog;       /* +0x02 */
    s16 tone;       /* +0x04 */
    s16 note;       /* +0x06 */
    u16 prio;       /* +0x08 */
} SndDef;

typedef struct {
    s16 id;
    u8 pad[6];
} SndVab;

extern SndChan D_80041DE0[23];
typedef struct {
    SndVab vabs[150];
    u8 pad[6];
    SndDef defs[1];
} SndBank;

extern SndBank D_80041BD0;
extern SndChan *func_8004D044(s32 id, s32 vl, s32 vr);
extern short SsUtKeyOnV(short voice, short vabId, short prog, short tone, short note, short fine, short voll, short volr);
extern short SsUtChangePitch(short voice, short vabId, short prog, short old_note, short old_fine, short new_note, short new_fine);
extern short SsUtSetVVol(short voice, short voll, short volr);

void func_8004D178(s32 id, s32 key, s32 vl, s32 vr, s32 pan)
{
    SndDef *def;
    SndVab *vab;
    SndChan *c;
    s32 note;
    s32 fine;
    s32 i;
    s32 v;
    s32 m;

    if (id == 0xFF) {
        return;
    }
    def = &D_80041BD0.defs[(u8)id];
    if (def->vab == -1) {
        return;
    }
    vab = &D_80041BD0.vabs[def->vab];
    if (vab->id == -1 || vab->id == 0xFE) {
        return;
    }
    pan += def->note * 16;
    note = pan >> 4;
    if (note < 0) {
        note = 0;
    } else if (note >= 128) {
        note = 127;
    }
    fine = (pan & 0xF) * 8;
    if (key != 0) {
        c = D_80041DE0;
        i = 22;
        do {
            if (c->voice != -1 && (c->busy & ~0x8000) == key) {
                goto found;
            }
            c++;
        } while (--i != -1);
        c = func_8004D044(id, vl, vr);
        if (c == 0) {
            return;
        }
        c->busy = key | 0x8000;
    } else {
        c = func_8004D044(id, vl, vr);
        if (c == 0) {
            return;
        }
        c->busy = 0;
    }
    v = c - D_80041DE0;
    c->voice = SsUtKeyOnV(v, vab->id, def->prog, def->tone, note, fine, vl, vr);
    if (v != c->voice) {
        c->voice = -1;
        return;
    }
    goto keyed;
found:
    SsUtSetVVol(c->voice, vl, vr);
    SsUtChangePitch(c->voice, vab->id, def->prog, c->note, c->fine, note, fine);
    c->busy |= 0x8000;
    goto common;
keyed:
    m = vl;
    if (vl < vr) {
        m = vr;
    }
    c->vol = m;
    c->note = note;
    c->fine = fine;
    c->id = id;
common:
    c->prio = (def->prio * (vl + vr)) >> 8;
}
