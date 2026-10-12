/* ac1_mission.h — behavioral reference model of the Armored Core 1 (SLUS-01323)
 * mission runtime: the per-mission data (chunk stream, spawn table, secondary
 * script VM blob) and the shared mission lifecycle (control block, timer,
 * objective progress, result flags, end sequence). See docs/MISSION_STATE_MACHINE.md,
 * docs/MISSION_SYSTEM.md, docs/MISSION_SCRIPT_VM.md, docs/ENTITY_TYPES.md.
 *
 * This is NOT a byte-matching decompilation. It is portable C99 that reproduces
 * the documented *behaviour*, with the game's addresses kept as constants so the
 * model stays anchored to the binary. Every rule is labelled in the .c with the
 * doc's confidence (CONFIRMED / HYPOTHESIS) and, where it is open, the
 * docs/LIVE_CHECKLIST.md item that settles it.
 *
 * Two halves:
 *   DATA   — byte-exact decoders of what is on disc (FDAT entry 2N+1): the chunk
 *            stream walker, the 256x40 spawn table and its 44-byte marshal
 *            (FUN_80073B74), and the chunk-4 secondary-VM blob (sets / threads /
 *            opcodes). Validated against a real FDAT.T by examples/mission_sim.c.
 *   RUNTIME— the shared lifecycle primitives the per-mission objective object
 *            calls: set_result (FUN_8004C318), objective_step (FUN_8008A80C),
 *            eventflags_tick (FUN_8008A8F8), the per-frame driver (FUN_8008AB68),
 *            the timer (cmd 4 / FUN_8008A778) and the exit result code.
 *
 * Anchors (mission overlay FDAT entry 202 @ 0x8004ADA0 unless noted):
 *   FUN_8004F508  mission scene loader (walks the chunk stream)
 *   FUN_80073B74  spawn marshal: chunk-12 record -> 44-byte instance template
 *   FUN_8008C178  vm_load_threads(set)          FUN_8008B380 vm_tick
 *   FUN_8008B42C  vm dispatch loop
 *   FUN_8004C318  mission_set_result(flags)     (in the ovl202_lower block)
 *   FUN_8008A80C  objective_step
 *   FUN_8008A8F8  eventflags_tick (end-anim state machine + 0x1000 masker)
 *   FUN_8008AB68  per-frame mission driver
 *   FUN_8008A0B0  primary command VM (jump table 0x8004C164, cmds 1..10)
 *   FUN_8008A178 / FUN_8008A778  cmd 4: set mission timer
 */
#ifndef AC1_MISSION_H
#define AC1_MISSION_H

#include <stddef.h>
#include <stdint.h>

/* ------------------------------------------------------------------------- */
/* RAM anchors (NA v1.1)                                                      */
/* ------------------------------------------------------------------------- */
#define AC1_MIS_OBJOBJ_LOAD_ADDR   0x801C4B40u  /* FDAT 2N objective object      */
#define AC1_MIS_CTRL_OBJOBJ_PTR    0x8019F51Cu  /* -> objective object           */
#define AC1_MIS_CTRL_WORKPTR       0x8019F520u  /* chunk-stream / work dest ptr  */
#define AC1_MIS_CTRL_FLAGS         0x8019F524u  /* result / event flags word     */
#define AC1_MIS_CTRL_END_COUNTER   0x8019F528u  /* end fade counter (100 -> 0)   */
#define AC1_MIS_CTRL_END_ANIM      0x8019F52Au  /* timer-mode / end-anim counter */
#define AC1_MIS_CTRL_TIMER         0x8019F52Cu  /* displayed mission timer       */
#define AC1_MIS_PROGRESS_COUNTER   0x8009079Cu  /* objective step counter        */
#define AC1_MIS_TERMINAL_FLAG      0x801CC80Cu  /* objective-terminal flag       */
#define AC1_MIS_RESULT_CODE        0x80048610u  /* read by the base EXE on exit  */
#define AC1_MIS_NUMBER_BYTE        0x800411F8u  /* mission number / phase byte   */
#define AC1_MIS_TIMER_STRUCT       0x801D0B40u  /* countdown struct              */
#define AC1_MIS_OBJECTIVES_STATE   0x801D0B50u  /* struct +0x10: 3 = complete    */
#define AC1_MIS_INSTANCE_TABLE     0x8019FAB8u  /* 256 x 44 instance templates   */
#define AC1_MIS_BLOCK_RECORDS      0x8019F538u  /* 44-byte geometry records      */
#define AC1_MIS_ENTITY_ARRAY       0x801A26B8u  /* entities, stride 0x170        */
#define AC1_MIS_ENTITY_STRIDE      0x170u
#define AC1_MIS_VM_THREAD_TABLE    0x801D0B60u  /* 24-byte thread records        */
#define AC1_MIS_VM_BLOB_PTR        0x801D0B64u

/* Flag bits in the control-block flags word (0x8019F524). */
#define AC1_MIS_FLAG_PROGRESS   0x0080u  /* cmd 5 / objective sub-step commit   */
#define AC1_MIS_FLAG_SUCCESS    0x0100u  /* -> result code 1                    */
#define AC1_MIS_FLAG_FAIL       0x0200u  /* -> result code 2 (timer expiry)     */
#define AC1_MIS_FLAG_EVENT400   0x0400u  /* set by VM / destructible action bit */
#define AC1_MIS_FLAG_TIMER_WARN 0x1000u  /* transient; cleared every frame      */
#define AC1_MIS_FLAG_OBJ_PENDING 0x2400u /* set by objective_step (<36)         */

#define AC1_MIS_END_FADE_FRAMES     100u  /* FUN_8004C318 arms 0x8019F528 = 100 */
#define AC1_MIS_PROGRESS_TERMINAL   36u   /* FUN_8008A80C: progress == 36 ends  */
#define AC1_MIS_END_ANIM_SPECIAL    60u   /* FUN_8008A8F8 state 60              */
#define AC1_MIS_END_ANIM_ACTIVE_LT  61u   /* driver runs the state machine when < 61 */
#define AC1_MIS_TIMER_SHOW_LT       16u   /* HUD draws the timer when end_anim < 16 */
#define AC1_MIS_TIMER_SCALE         22u   /* cmd 4: timer = 22 * param (HYPOTHESIS) */
#define AC1_MIS_RESULT_SUCCESS      1u
#define AC1_MIS_RESULT_FAIL         2u

/* ------------------------------------------------------------------------- */
/* DATA — FDAT entry 2N+1 chunk stream                                        */
/* ------------------------------------------------------------------------- */
#define AC1_CHUNK_GEOMETRY      0   /* local-coord PA-format level blocks      */
#define AC1_CHUNK_SCRIPT        4   /* secondary-VM blob (sets / threads)      */
#define AC1_CHUNK_PLACEMENT     7   /* 52-byte section placement records       */
#define AC1_CHUNK_GEOM_RECORDS  11  /* per-mission geometry records            */
#define AC1_CHUNK_SPAWNS        12  /* 256 x 40-byte spawn / instance table    */

/* Iterator over `[u32 len][payload]...` chunks. The walk stops at a zero or
 * out-of-range length (the stream terminator), mirroring FUN_8004F508 and
 * tools/mission/mission_parse.walk_chunks. */
typedef struct {
    const uint8_t *buf;
    size_t         size;
    size_t         off;   /* next chunk header offset */
    unsigned       idx;   /* index of the next chunk  */
} ac1_chunk_iter;

void ac1_chunk_begin(ac1_chunk_iter *it, const uint8_t *stream, size_t size);
/* Returns 1 and fills idx/payload/len for the next chunk, 0 at the terminator. */
int  ac1_chunk_next(ac1_chunk_iter *it, unsigned *idx, const uint8_t **payload,
                    uint32_t *len);
/* Find chunk `want`; returns 1 and fills payload/len, 0 if absent. */
int  ac1_chunk_find(const uint8_t *stream, size_t size, unsigned want,
                    const uint8_t **payload, uint32_t *len);

/* ------------------------------------------------------------------------- */
/* DATA — chunk 12 spawn table + FUN_80073B74 marshal                         */
/* ------------------------------------------------------------------------- */
#define AC1_SPAWN_REC_LEN   40   /* 20 x int16                                */
#define AC1_SPAWN_COUNT     256
#define AC1_TEMPLATE_LEN    44   /* instance template at 0x8019FAB8 + i*44    */

typedef struct {
    int16_t hw[20];
} ac1_spawn_rec;

/* Field accessors (CONFIRMED meanings; see docs/MISSION_SYSTEM.md §2). */
#define AC1_SPAWN_X(r)      ((r)->hw[0])
#define AC1_SPAWN_Y(r)      ((r)->hw[1])
#define AC1_SPAWN_Z(r)      ((r)->hw[2])
#define AC1_SPAWN_BLOCK(r)  ((r)->hw[3])   /* geometry block index AND dispatch type; -1 = none */
#define AC1_SPAWN_ROT(r)    ((r)->hw[5])   /* PSX angle units, 2048 = half turn */
#define AC1_SPAWN_RESID(r)  ((r)->hw[7])   /* resource / class id (0..388); NOT the dispatch type */
#define AC1_SPAWN_HP(r)     ((r)->hw[11])  /* HP (path CONFIRMED, value HYP-strong) */

/* Read record i (0..255) of a chunk-12 payload. Returns 0 if out of range. */
int  ac1_spawn_get(const uint8_t *chunk12, uint32_t len, unsigned i, ac1_spawn_rec *out);
/* Empty slot rule used by the tools: block == -1 and position (0,0,0). */
int  ac1_spawn_is_empty(const ac1_spawn_rec *r);
/* FUN_80073B74: build the 44-byte instance template from a spawn record. */
void ac1_spawn_marshal(const ac1_spawn_rec *r, uint8_t out[AC1_TEMPLATE_LEN]);

/* ------------------------------------------------------------------------- */
/* DATA — chunk 4 secondary actor-thread VM blob                              */
/* ------------------------------------------------------------------------- */
#define AC1_VMOP_END        0xFFFFu
#define AC1_VMOP_SET_LOOP   0xFFF0u
#define AC1_VMOP_OP_F       0x000Fu   /* the only variable-length opcode */
#define AC1_VMOP_SET_RESULT 0x100Au
#define AC1_VMOP_SPAWN_GROUP 0x1003u

typedef struct {
    uint16_t    op;
    const char *name;
    int         oplen;    /* operand bytes after the opcode word; -1 = variable */
} ac1_vmop;

/* Opcode table lookup. Unknown words are 0-operand no-ops in v1.1 (returns a
 * static "noop" descriptor, never NULL). */
const ac1_vmop *ac1_vm_lookup(uint16_t op);
/* Total instruction length at blob offset pc (opcode word + operands), or 0 if
 * the instruction would run past `size`. */
size_t ac1_vm_oplen(const uint8_t *blob, size_t size, size_t pc);

typedef struct {
    uint16_t offset;      /* set header offset within the blob            */
    uint16_t flags;       /* +0 (0/1; HYP autostart/loop)                  */
    uint16_t count;       /* +2 thread count  -> 0x801D0B51                 */
    uint16_t extra;       /* +4 (HYP frame budget)                          */
    int      sentinel;    /* 1 if the set_offset is a placeholder (not walked) */
} ac1_vm_set;

/* Number of sets = set_offset[0] / 2. */
unsigned ac1_vm_nsets(const uint8_t *blob, size_t size);
/* Read set s. Returns 0 if out of range. Sentinel sets have count = 0. */
int  ac1_vm_set_get(const uint8_t *blob, size_t size, unsigned s, ac1_vm_set *out);
/* Thread t of set s: returns the thread's byte offset, or 0 if out of range. */
uint16_t ac1_vm_thread_offset(const uint8_t *blob, size_t size, const ac1_vm_set *s, unsigned t);

/* Walk one thread from its offset. Returns 1 if the walk reaches END (0xFFFF)
 * inside the blob, 0 if it runs off the end or exceeds the guard. Fills the
 * number of instructions (END included) and the byte length of the thread. */
int  ac1_vm_thread_walk(const uint8_t *blob, size_t size, uint16_t toff,
                        unsigned *nops, size_t *nbytes);

/* ------------------------------------------------------------------------- */
/* RUNTIME — shared mission lifecycle                                         */
/* ------------------------------------------------------------------------- */
struct ac1_mission;

/* The per-mission objective object (FDAT 2N, loaded at 0x801C4B40) is MIPS
 * code the runtime calls through a pointer block. The host supplies it as
 * callbacks; NULL entries are skipped (the game null-checks its vtables). */
typedef struct {
    void (*tick)(struct ac1_mission *m);               /* +0x00 per frame          */
    void (*event)(struct ac1_mission *m, int id);      /* +0x08 primary-VM cmd 9   */
    void (*query)(struct ac1_mission *m, void *buf);   /* +0x0C objective/HUD      */
    void (*free_entity)(struct ac1_mission *m, void *e);/* +0x10 despawn hook      */
    void (*init[16])(struct ac1_mission *m, void *e);  /* +0x20 per-type spawn ctor */
} ac1_objobj;

/* Host-side observers for side effects the game performs through other
 * subsystems (sound, the special end action, exit). All optional. */
typedef struct {
    void *user;
    void (*sound)(void *user, int id);                 /* FUN_800529F0(0,0,id)      */
    void (*end_special)(void *user);                   /* DAT_80039C5D callback     */
    void (*exit)(void *user, uint32_t result_code);    /* end counter reached 0     */
} ac1_mission_hooks;

typedef struct ac1_mission {
    /* control block 0x8019F51C.. */
    uint32_t flags;        /* 0x8019F524 */
    uint32_t end_counter;  /* 0x8019F528 */
    uint16_t end_anim;     /* 0x8019F52A */
    uint32_t timer;        /* 0x8019F52C displayed timer (frames) */
    uint16_t f52e, f530, f532; /* init 128 / 0 / 64 (partial)       */
    /* elsewhere */
    uint32_t progress;     /* 0x8009079C */
    uint8_t  terminal;     /* 0x801CC80C */
    uint32_t result_code;  /* 0x80048610 */
    uint16_t objectives_state; /* 0x801D0B50 (3 = objectives complete) */
    uint8_t  mission_number;   /* 0x800411F8 -> 0x8019F518 */
    /* timer struct 0x801D0B40 */
    uint16_t countdown;    /* +0x00 */
    uint16_t countdown_type; /* +0x04 (128/256/512 from cmd 4) */
    /* bookkeeping */
    uint32_t frame;
    int      exited;
    const ac1_objobj   *obj;
    ac1_mission_hooks   hooks;
} ac1_mission;

/* FUN_8004F508 prologue: reset the control block for mission `number`. */
void ac1_mission_init(ac1_mission *m, uint8_t number, const ac1_objobj *obj,
                      const ac1_mission_hooks *hooks);

/* FUN_8004C318(a0): store `flags` into the flags word and, if the end counter
 * is 0, arm it to 100 (the exit fade). */
void ac1_mission_set_result(ac1_mission *m, uint32_t flags);

/* FUN_8008A80C: advance the objective progress counter (terminal at 36). */
void ac1_mission_objective_step(ac1_mission *m);

/* FUN_8008A8F8: end-anim state machine + the per-frame 0x1000 masker. */
void ac1_mission_eventflags_tick(ac1_mission *m);

/* cmd 4 / FUN_8008A778: set the displayed timer. `type` = 128/256/512. */
void ac1_mission_set_timer(ac1_mission *m, uint32_t param, uint16_t type);

/* FUN_8008A0B0: primary command VM, cmd 1..10. Returns 0 if handled, -1 if the
 * command is outside the table. Commands that only touch other subsystems
 * (object flag ranges, group enables, broadcasts) are recorded as no-ops. */
int  ac1_mission_cmd(ac1_mission *m, int cmd, int arg);

/* FUN_8008AB68 + the timer tick: one mission frame. */
void ac1_mission_frame(ac1_mission *m);

/* 0x8004C69C: result code the base EXE reads (1 success, 2 fail, else 0). */
uint32_t ac1_mission_exit_result(const ac1_mission *m);

/* Human-readable lifecycle phase, derived from the state (for tools/tests). */
const char *ac1_mission_phase(const ac1_mission *m);

#endif /* AC1_MISSION_H */
