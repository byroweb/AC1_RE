#include "ac1/ac1.h"

typedef struct {
    u32 pos;
    u32 size;
    char name[16];
} CdFile;

register u32 *gp_base asm("$28");

extern char D_80039C94[];  /* ";1" */
extern char *func_8002A440(char *dst, char *src);
extern CdFile *func_800191C0(CdFile *fp, char *name);
extern void func_80015DC0(s32 code);
extern void *func_80016190(u32 size);
extern void func_80016328(CdFile *fp, void *dst, u32 sectors);

void *func_800163F8(char *name)
{
    u32 path[16];
    CdFile file;
    u32 size;
    void *buf;

    __builtin_memcpy(path, gp_base + 12, 5);
    func_8002A440((char *)path, name);
    func_8002A440((char *)path, D_80039C94);
    if (func_800191C0(&file, (char *)path) == 0) {
        func_80015DC0(0);
    }
    size = (file.size + 0x7FF) & ~0x7FF;
    buf = func_80016190(size);
    func_80016328(&file, buf, size >> 11);
    return buf;
}
