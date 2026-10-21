/* include/ac1/mission.h - overlay 202 mission control block and world
 * structures.
 *
 * MISSION_SYSTEM.md calls 0x801D0B40 the "timer struct".  The retail code
 * shows a 0x28-byte block that mixes the alarm timer with the mission
 * script VM state, followed directly by the world-structure array:
 *   - mission init 0x8008A348: +0x00 = func_80073904(), +0x02 = 1200,
 *     +0x10 = 0xFF, +0x18 = heap start (word-aligned);
 *   - timer helper 0x8008A778: +0x06 (a1), +0x04 (a2, the alarm argument
 *     that 0x8004C54C passes to FUN_800529F0);
 *   - vm_load_threads 0x8008C178: +0x10 = set index, +0x11 = thread count,
 *     +0x12 = set header word, +0x1C/+0x20 = heap start, +0x24 = blob;
 *   - bump allocator 0x8008C150: +0x1C += (n + 3) & ~3.
 * The "objectives complete = 3" byte at 0x801D0B50 is the VM set index.
 */
#ifndef AC1_MISSION_H
#define AC1_MISSION_H

#include "types.h"

typedef struct MissionCtl {
    s16 unk00;                  /* +0x00 func_80073904() at mission init */
    u16 unk02;                  /* +0x02 1200 at mission init */
    u8 alarm_arg;               /* +0x04 */
    u8 pad05;
    u16 alarm_param;            /* +0x06 */
    u8 unk08[8];
    u8 vm_set;                  /* +0x10 loaded thread set, 0xFF = none */
    u8 vm_threads;              /* +0x11 thread count of that set */
    u16 vm_extra;               /* +0x12 set header +4 (frame budget?) */
    u32 unk14;                  /* +0x14 */
    u8 *heap;                   /* +0x18 VM heap start */
    u8 *heap_cur;               /* +0x1C bump-allocator cursor */
    void *vm_thread_tab;        /* +0x20 thread records (24 bytes each) */
    u8 *vm_blob;                /* +0x24 chunk-4 script blob */
} MissionCtl;
AC1_ASSERT_SIZE(MissionCtl, 0x28);

/* g_world_structs (0x801D0B68): 0x40 each, g_world_struct_count entries.
 * Doors, generators and fuel tanks (DESTRUCTIBLE_OBJECTS.md,
 * INTERACTIONS.md, LEVEL_LOADING.md). */
typedef struct WorldStruct {
    u8 unk00[0x10];             /* +0x00/+0x08 read by the visibility walk */
    s16 org[3];                 /* +0x10 origin the collision mesh is relative to (func_800705D8) */
    s16 unk16;                  /* +0x16 passed to func_8006FA30 with the hit point */
    s16 bbox_min[3];            /* +0x18 */
    s16 mesh;                   /* +0x1E index into the collision-mesh table (CollMesh at 0x80091228) */
    s16 bbox_max[3];            /* +0x20 */
    s16 n_parts;                /* +0x26 count of parts[] (func_8006D45C) */
    u16 *parts;                 /* +0x28 g_world_structs indices of the parts */
    void *on_hit;               /* +0x2C handler record (fn at +4); 0 = destroyed */
    u8 unk30[8];                /* +0x30 flags (LEVEL_LOADING.md) */
    void *stats;                /* +0x38 u16 hp, effect, model, sound */
    u8 unk3c[4];
} WorldStruct;
AC1_ASSERT_SIZE(WorldStruct, 0x40);

#endif
