#include "ac1/ac1.h"

typedef struct Part7C {
    s16 pos[3];                 /* +0x00 */
    s16 rot[3];                 /* +0x06 */
    s16 scale[3];               /* +0x0C 4096 = 1.0 */
    u8 pad12[0x14];
    u16 flags;                  /* +0x26 bit 0 = local matrix stale */
    u8 pad28[0x54];
} Part7C;

/* Part animation stream: for each frame from..to, a count, then per command
 * a word (part << 8 | pos:2 scale:2 rot:2 | relative:1) and its operands. */
u16 *func_80053A54(u16 *p, Part7C *parts, s32 from, s32 to, s32 mask) {
    Part7C *pt;
    s32 n;
    s32 cmd;
    s32 c;

    for (; from <= to; from++) {
        n = *p++;
        while (--n != -1) {
            cmd = *p++;
            pt = &parts[cmd >> 8];
            if (cmd & 1) {
                if (pt->flags & mask) {
                    goto skip;
                }
                c = cmd & 0xC;
                if (c != 0) {
                    if (c == 4) {
                        pt->rot[0] += *p++;
                        pt->rot[1] += *p++;
                        pt->rot[2] += *p++;
                    } else if (c == 8) {
                        pt->rot[0] += ((s8 *)p)[0];
                        pt->rot[1] += ((s8 *)p)[1];
                        pt->rot[2] += ((s8 *)p)[2];
                        p += 2;
                    }
                    pt->flags |= 1;
                }
                c = cmd & 0x30;
                if (c != 0) {
                    if (c == 0x10) {
                        pt->scale[0] = (pt->scale[0] * *p++) >> 12;
                        pt->scale[1] = (pt->scale[1] * *p++) >> 12;
                        pt->scale[2] = (pt->scale[2] * *p++) >> 12;
                    } else if (c == 0x20) {
                        pt->scale[0] = (pt->scale[0] * ((s8 *)p)[0]) >> 12;
                        pt->scale[1] = (pt->scale[1] * ((s8 *)p)[1]) >> 12;
                        pt->scale[2] = (pt->scale[2] * ((s8 *)p)[2]) >> 12;
                        p += 2;
                    }
                    pt->flags |= 1;
                }
                c = cmd & 0xC0;
                if (c != 0) {
                    if (c == 0x40) {
                        pt->pos[0] += *p++;
                        pt->pos[1] += *p++;
                        pt->pos[2] += *p++;
                    } else if (c == 0x80) {
                        pt->pos[0] += ((s8 *)p)[0];
                        pt->pos[1] += ((s8 *)p)[1];
                        pt->pos[2] += ((s8 *)p)[2];
                        p += 2;
                    }
                }
            } else {
                if (pt->flags & mask) {
                    goto skip;
                }
                c = cmd & 0xC;
                if (c != 0) {
                    if (c == 4) {
                        pt->rot[0] = *p++;
                        pt->rot[1] = *p++;
                        pt->rot[2] = *p++;
                    } else if (c == 8) {
                        pt->rot[0] = ((s8 *)p)[0];
                        pt->rot[1] = ((s8 *)p)[1];
                        pt->rot[2] = ((s8 *)p)[2];
                        p += 2;
                    } else if (c == 0xC) {
                        pt->rot[0] = pt->rot[1] = pt->rot[2] = 0;
                    }
                    pt->flags |= 1;
                }
                c = cmd & 0x30;
                if (c != 0) {
                    if (c == 0x10) {
                        pt->scale[0] = *p++;
                        pt->scale[1] = *p++;
                        pt->scale[2] = *p++;
                    } else if (c == 0x20) {
                        pt->scale[0] = ((s8 *)p)[0];
                        pt->scale[1] = ((s8 *)p)[1];
                        pt->scale[2] = ((s8 *)p)[2];
                        p += 2;
                    } else if (c == 0x30) {
                        pt->scale[0] = pt->scale[1] = pt->scale[2] = 4096;
                    }
                    pt->flags |= 1;
                }
                c = cmd & 0xC0;
                if (c != 0) {
                    if (c == 0x40) {
                        pt->pos[0] = *p++;
                        pt->pos[1] = *p++;
                        pt->pos[2] = *p++;
                    } else if (c == 0x80) {
                        pt->pos[0] = ((s8 *)p)[0];
                        pt->pos[1] = ((s8 *)p)[1];
                        pt->pos[2] = ((s8 *)p)[2];
                        p += 2;
                    } else if (c == 0xC0) {
                        pt->pos[0] = pt->pos[1] = pt->pos[2] = 0;
                    }
                }
            }
            continue;
        skip:
            c = cmd & 0xC;
            if (c != 0) {
                if (c == 4) {
                    p += 3;
                } else if (c == 8) {
                    p += 2;
                }
            }
            c = cmd & 0x30;
            if (c != 0) {
                if (c == 0x10) {
                    p += 3;
                } else if (c == 0x20) {
                    p += 2;
                }
            }
            c = cmd & 0xC0;
            if (c != 0) {
                if (c == 0x40) {
                    p += 3;
                } else if (c == 0x80) {
                    p += 2;
                }
            }
        }
    }
    return p;
}
