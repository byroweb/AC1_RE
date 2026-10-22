#include "ac1/ac1.h"
s32 _card_info(s32 chan);
s32 _card_clear(s32 chan);
s32 _card_load(s32 chan);
s32 func_80068828(void);
s32 func_800688AC(void);
void func_80068930(void);
void func_80068988(void);

s32 func_800684A0(u8 port)
{
    s32 chan = (port != 0) << 4;
    s32 r;

    func_80068930();
    _card_info(chan);
    r = func_80068828();
    if (r == 1 || r == 2)
        return r;
    if (r == 3) {
        func_80068988();
        _card_clear(chan);
        r = func_800688AC();
        if (r != 0)
            return r;
        func_80068930();
        _card_load(chan);
        r = func_80068828();
        if (r == 1 || r == 2)
            return r;
        if (r == 4)
            return 3;
    }
    func_80068930();
    _card_load(chan);
    r = func_80068828();
    switch (r) {
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        r = 4;
        break;
    }
    return r;
}
