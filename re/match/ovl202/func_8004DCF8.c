#include "ac1/ac1.h"

typedef struct {
    u8 type;
    u8 depth_l;
    u8 depth_r;
    u8 vol_l;
    u8 vol_r;
    u8 feedback;
    u8 delay;
} RevPreset;

typedef struct {
    s16 id;
    u16 sub;
    u8 a;
    u8 b;
    s8 c;
    u8 pad;
} SndSlot;

typedef struct {
    u8 pad[0x30];
    u32 lo : 18;
    u32 reverb : 6;
    u32 hi : 8;
} RoomDef;

extern void func_8004DCBC(void);
extern void func_8004D178(s32 id, s32 sub, s32 a, s32 b, s32 c);
extern short SsUtSetReverbType(short type);
extern void func_800228E4(void);
extern void SsUtSetReverbDepth(short l, short r);
extern void SsUtSetReverbFeedback(short fb);
extern void SsUtSetReverbDelay(short delay);
extern void SsSetRVol(short l, short r);
extern RoomDef *D_801AD668;
extern RoomDef *D_80041BA0;
extern RevPreset D_80041EC6[];
extern RevPreset D_80041BC8;
extern u8 D_80041BC8_type;
extern SndSlot D_80042A86[4];

void func_8004DCF8(void)
{
    RoomDef *cur;
    RevPreset *r;
    SndSlot *s;
    s32 i;

    func_8004DCBC();
    cur = D_801AD668;
    if (cur != D_80041BA0) {
        if (D_80041BA0 == 0) {
set:
            r = &D_80041EC6[cur->reverb];
            if (D_80041BC8_type != r->type) {
                SsUtSetReverbType(r->type);
                func_800228E4();
            }
            SsUtSetReverbDepth(r->depth_l, r->depth_r);
            SsUtSetReverbFeedback(r->feedback);
            SsUtSetReverbDelay(r->delay);
            SsSetRVol(r->vol_l, r->vol_r);
            D_80041BC8 = *r;
        } else if (cur->reverb != D_80041BA0->reverb) {
            goto set;
        }
        D_80041BA0 = cur;
    }
    for (s = D_80042A86, i = 4; i != 0; s++, i--) {
        if (s->id != -1) {
            func_8004D178(s->id, s->sub, s->a, s->b, s->c);
        }
        s->id = -1;
    }
}
