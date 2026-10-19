#include "ac1_types.h"
/* exe 0x800164A0  size 172  score 5.821  2026-10-18
 * Build "\GG\<name>;1", CdSearchFile it, read (size + 2047) / 2048 sectors.
 * The "\GG\" prefix sits in retail .sdata (gp+48, gp = 0x80039C5C) and is
 * copied gp-relative; the pinned flags are -G0, so gp is a register variable. */

typedef struct {
    u32 pos;
    u32 size;
    char name[24];
} Func800164A0File;

register u32 *gp_base asm("$28");

extern char *func_8002A440(char *dst, char *src);
extern Func800164A0File *func_800191C0(Func800164A0File *fp, char *name);
extern void func_80015DC0(s32 code);
extern void func_80016328(Func800164A0File *fp, void *dst, u32 sectors);

s32 func_800164A0(void *dst, char *name, u32 size)
{
    u32 path[16];
    Func800164A0File file;
    char *p;
    Func800164A0File *f;

    __builtin_memcpy(path, gp_base + 12, 5);
    p = (char *)path;
    func_8002A440(p, name);
    func_8002A440(p, ";1");
    f = &file;
    if (func_800191C0(f, p) == 0) {
        func_80015DC0(0);
    }
    if (size == 0) {
        size = file.size;
    }
    func_80016328(f, dst, (size + 2047) >> 11);
    return 0;
}
