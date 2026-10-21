/* include/ac1/ovl204_sound.h - overlay 204 positional sound: the two-ear listener at
 * 0x80041B90 (func_8004D004 sets it, func_8004CA24 / func_8004CB3C read it).
 * Include after "ac1/ac1.h". */
#ifndef AC1_OVL204_SOUND_H
#define AC1_OVL204_SOUND_H

typedef struct {                /* 0x80041B90: sound listener */
    SVECTOR pos;                /* +0x00 */
    u8 rot08[0x0C];             /* +0x08 written by func_80014B74 */
    SVECTOR pos2;               /* +0x14 */
    u8 rot1c[0x0C];             /* +0x1C */
} Listener;

extern Listener D_80041B90;

#endif
