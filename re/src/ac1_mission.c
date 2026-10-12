/* ac1_mission.c — behavioral reference model of the AC1 mission runtime.
 * Portable C99. See ac1_mission.h and docs/MISSION_STATE_MACHINE.md.
 *
 * Confidence labels follow the docs: CONFIRMED = disasm/byte-verified or
 * live-observed; HYPOTHESIS = inferred. Open points cite docs/LIVE_CHECKLIST.md. */
#include "ac1_mission.h"
#include <string.h>

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static int16_t  rds16(const uint8_t *p) { return (int16_t)rd16(p); }
static uint32_t rd32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void wr16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }

/* ========================================================================= */
/* DATA — chunk stream (FUN_8004F508 walk)  CONFIRMED                        */
/* ========================================================================= */
void ac1_chunk_begin(ac1_chunk_iter *it, const uint8_t *stream, size_t size) {
    it->buf = stream; it->size = size; it->off = 0; it->idx = 0;
}

int ac1_chunk_next(ac1_chunk_iter *it, unsigned *idx, const uint8_t **payload,
                   uint32_t *len) {
    if (!it->buf || it->off + 4 > it->size) return 0;
    uint32_t ln = rd32(it->buf + it->off);
    /* `len = u32@s1; handler(s1+4); s1 += len+4` — a zero or out-of-range
     * length is the terminator (tools/mission/mission_parse.walk_chunks). */
    if (ln == 0 || it->off + 4 + ln > it->size) return 0;
    if (idx) *idx = it->idx;
    if (payload) *payload = it->buf + it->off + 4;
    if (len) *len = ln;
    it->off += 4 + ln;
    it->idx++;
    return 1;
}

int ac1_chunk_find(const uint8_t *stream, size_t size, unsigned want,
                   const uint8_t **payload, uint32_t *len) {
    ac1_chunk_iter it; unsigned idx; const uint8_t *p; uint32_t ln;
    ac1_chunk_begin(&it, stream, size);
    while (ac1_chunk_next(&it, &idx, &p, &ln)) {
        if (idx == want) { if (payload) *payload = p; if (len) *len = ln; return 1; }
        if (idx > want) break;
    }
    return 0;
}

/* ========================================================================= */
/* DATA — spawn table + marshal                                              */
/* ========================================================================= */
int ac1_spawn_get(const uint8_t *chunk12, uint32_t len, unsigned i, ac1_spawn_rec *out) {
    if (i >= AC1_SPAWN_COUNT || (size_t)(i + 1) * AC1_SPAWN_REC_LEN > len) return 0;
    const uint8_t *p = chunk12 + (size_t)i * AC1_SPAWN_REC_LEN;
    for (int k = 0; k < 20; k++) out->hw[k] = rds16(p + 2 * k);
    return 1;
}

int ac1_spawn_is_empty(const ac1_spawn_rec *r) {
    /* mission_parse.dump_spawns: block == -1 and zero position */
    return r->hw[3] == -1 && r->hw[0] == 0 && r->hw[1] == 0 && r->hw[2] == 0;
}

/* FUN_80073B74 (0x80073B74), CONFIRMED structure:
 *   rec[+0x00] = 0 (status; runtime-set)
 *   rec[+0x04 + 2K] = hwK, K = 0..19            (flat reshape, stride 40 -> 44)
 *   rec[+0x02] = rec[+0x1A]  (= hw11, the HP seed; template +0x02 = "current HP")
 * docs/MISSION_SYSTEM.md §2 once recorded the derived copy as +0x00 = +0x18 —
 * the current decompile (DESTRUCTIBLE_OBJECTS.md "Record layout",
 * MISSION_STAGES.md Part 2) says +0x02 = +0x1A; LIVE_CHECKLIST S-5 seals it. */
void ac1_spawn_marshal(const ac1_spawn_rec *r, uint8_t out[AC1_TEMPLATE_LEN]) {
    memset(out, 0, AC1_TEMPLATE_LEN);
    for (int k = 0; k < 20; k++) wr16(out + 4 + 2 * k, (uint16_t)r->hw[k]);
    wr16(out + 2, rd16(out + 0x1A));
}

/* ========================================================================= */
/* DATA — secondary VM blob  (MISSION_SCRIPT_VM.md, byte-verified)           */
/* ========================================================================= */
static const ac1_vmop VMOPS[] = {
    { 0x0000, "copy_pos_lo",   6 },
    { 0x0002, "copy_pos_hi",   6 },
    { 0x0008, "spawn_at8",     6 },
    { 0x0009, "spawn_at9",     6 },
    { 0x000A, "init_unit",     12 },
    { 0x000B, "subcall_b320",  0 },
    { 0x000E, "mark_lerp",     14 },
    { 0x000F, "op_f",          -1 },   /* u16 cnt, then cnt*4 bytes */
    { 0x1000, "sel_actor",     2 },
    { 0x1001, "set_xyz",       6 },
    { 0x1002, "set_xyz2",      6 },
    { 0x1003, "spawn_group",   2 },
    { 0x1004, "play_sound",    2 },
    { 0x1005, "flag_all",      0 },
    { 0x1006, "clear_targets", 0 },
    { 0x1007, "call_db18",     2 },
    { 0x1008, "set_obj_flag",  6 },
    { 0x1009, "spawn_marker",  6 },
    { 0x100A, "set_result",    2 },
    { 0x100B, "set_obj_word",  4 },
    { 0x100C, "despawn_group", 2 },
    { 0xFFF0, "set_loop",      2 },
    { 0xFFFF, "END",           0 },
};
static const ac1_vmop VMOP_NOOP = { 0, "noop", 0 };

const ac1_vmop *ac1_vm_lookup(uint16_t op) {
    for (size_t i = 0; i < sizeof VMOPS / sizeof VMOPS[0]; i++)
        if (VMOPS[i].op == op) return &VMOPS[i];
    return &VMOP_NOOP;   /* reserved words are 0-operand silent no-ops in v1.1 */
}

size_t ac1_vm_oplen(const uint8_t *blob, size_t size, size_t pc) {
    if (pc + 2 > size) return 0;
    uint16_t op = rd16(blob + pc);
    size_t total;
    if (op == AC1_VMOP_OP_F) {
        if (pc + 4 > size) return 0;
        total = 4 + (size_t)rd16(blob + pc + 2) * 4;
    } else {
        total = 2 + (size_t)ac1_vm_lookup(op)->oplen;
    }
    return (pc + total <= size) ? total : 0;
}

unsigned ac1_vm_nsets(const uint8_t *blob, size_t size) {
    if (size < 2) return 0;
    return rd16(blob) / 2;
}

int ac1_vm_set_get(const uint8_t *blob, size_t size, unsigned s, ac1_vm_set *out) {
    unsigned n = ac1_vm_nsets(blob, size);
    if (s >= n || (size_t)(2 * s + 2) > size) return 0;
    memset(out, 0, sizeof *out);
    out->offset = rd16(blob + 2 * s);
    /* A set_offset inside the offset table itself (e.g. 0) or past the blob is
     * a placeholder slot (seen in 019.bin); the loader never derefs it because
     * no shipped mission requests that index. Mirror mission_script.parse. */
    if (out->offset < n * 2 || (size_t)out->offset + 6 > size) { out->sentinel = 1; return 1; }
    out->flags = rd16(blob + out->offset);
    out->count = rd16(blob + out->offset + 2);
    out->extra = rd16(blob + out->offset + 4);
    return 1;
}

uint16_t ac1_vm_thread_offset(const uint8_t *blob, size_t size, const ac1_vm_set *s, unsigned t) {
    if (s->sentinel || t >= s->count) return 0;
    size_t at = (size_t)s->offset + 6 + 2 * t;
    if (at + 2 > size) return 0;
    return rd16(blob + at);
}

int ac1_vm_thread_walk(const uint8_t *blob, size_t size, uint16_t toff,
                       unsigned *nops, size_t *nbytes) {
    size_t pc = (size_t)toff + 2;   /* thread_hdr, then PC starts at +2 */
    unsigned n = 0;
    if ((size_t)toff + 2 > size) return 0;
    for (unsigned guard = 0; guard < 4096; guard++) {
        size_t ln = ac1_vm_oplen(blob, size, pc);
        if (ln == 0) return 0;
        uint16_t op = rd16(blob + pc);
        n++; pc += ln;
        if (op == AC1_VMOP_END) {
            if (nops) *nops = n;
            if (nbytes) *nbytes = pc - toff;
            return 1;
        }
    }
    return 0;
}

/* ========================================================================= */
/* RUNTIME — shared lifecycle                                                */
/* ========================================================================= */
static void hook_sound(ac1_mission *m, int id) {
    if (m->hooks.sound) m->hooks.sound(m->hooks.user, id);
}

/* FUN_8004F508 prologue (CONFIRMED): init the control block, store the mission
 * number, zero the displayed timer (0x8004F9FC). Init values 128/0/64 for
 * +0x12/+0x14/+0x16 are recorded as "partial" in MISSION_SYSTEM.md §5. */
void ac1_mission_init(ac1_mission *m, uint8_t number, const ac1_objobj *obj,
                      const ac1_mission_hooks *hooks) {
    memset(m, 0, sizeof *m);
    m->mission_number = number;
    m->f52e = 128; m->f530 = 0; m->f532 = 64;
    m->obj = obj;
    if (hooks) m->hooks = *hooks;
}

/* FUN_8004C318(a0) (CONFIRMED by disasm + live):
 *   "Writes a0 directly into 0x8019F524, and if 0x8019F528 == 0 sets it to 100."
 * A direct store means a later set_result() would drop bits set in between
 * (e.g. the 0x400 event bit) — yet the live exit read 0x480 = 0x80 | 0x400.
 * Store-vs-OR is LIVE_CHECKLIST MIS-15; the model follows the disasm (store). */
void ac1_mission_set_result(ac1_mission *m, uint32_t flags) {
    m->flags = flags;
    if (m->end_counter == 0) m->end_counter = AC1_MIS_END_FADE_FRAMES;
}

/* FUN_8008A80C (CONFIRMED): progress counter 0x8009079C, range-checked < 37. */
void ac1_mission_objective_step(ac1_mission *m) {
    if (m->progress > AC1_MIS_PROGRESS_TERMINAL) return;      /* out of range: ignored */
    if (m->progress == AC1_MIS_PROGRESS_TERMINAL) {
        m->end_anim = 0;
        m->terminal = 1;                                     /* 0x801CC80C = 1 */
        m->flags &= ~AC1_MIS_FLAG_OBJ_PENDING;
        return;
    }
    m->end_anim = 0xFFFF;                                    /* start end sequence */
    m->flags |= AC1_MIS_FLAG_OBJ_PENDING;
    if (m->progress != 0) hook_sound(m, 101);                /* FUN_800529F0(0,0,101) */
    m->progress++;
}

/* FUN_8008A8F8 (CONFIRMED): run by the driver when end_anim < 61.
 *   state == 60: special end action (DAT_80039C5D callback), clears 0x1000;
 *   else: decrement end_anim toward 0.
 * It also re-writes the flags word every frame (`lw; and ~0x1000; sw` at
 * 0x8008A938) to clear the transient timer-warn bit — the "masker" that defeats
 * a naive watchpoint (MISSION_SYSTEM.md §6). */
void ac1_mission_eventflags_tick(ac1_mission *m) {
    if (m->end_anim == AC1_MIS_END_ANIM_SPECIAL) {
        if (m->hooks.end_special) m->hooks.end_special(m->hooks.user);
    } else if (m->end_anim > 0) {
        m->end_anim--;
    }
    m->flags &= ~AC1_MIS_FLAG_TIMER_WARN;
}

/* cmd 4 (0x8008A178) / FUN_8008A778: timer = 22 * param (×22 and the frame unit
 * are HYPOTHESIS — LIVE_CHECKLIST MIS-2); type 128/256/512 -> timer struct. */
void ac1_mission_set_timer(ac1_mission *m, uint32_t param, uint16_t type) {
    m->timer = AC1_MIS_TIMER_SCALE * param;
    m->countdown_type = type;
    m->countdown = (uint16_t)(m->timer > 0xFFFF ? 0xFFFF : m->timer);
}

/* Timer tick handler (~0x8004C4F0 / caller 0x8004C5F0): on expiry
 * FUN_8004C318(0x200) = FAIL. The per-frame decrement rate of the displayed
 * timer is modelled as 1/frame (HYPOTHESIS; MIS-2). */
static void timer_tick(ac1_mission *m) {
    if (m->timer == 0) return;
    m->timer--;
    if (m->timer == 0) ac1_mission_set_result(m, AC1_MIS_FLAG_FAIL);
}

/* FUN_8008AA90: end-sequence step + screen flash; the counter armed to 100 by
 * set_result counts down and the mission exits at 0 (CONFIRMED live: ~100-frame
 * fade to the debrief). */
static void end_step(ac1_mission *m) {
    if (m->end_counter == 0) return;
    m->end_counter--;
    if (m->end_counter == 0 && !m->exited) {
        m->exited = 1;
        m->result_code = ac1_mission_exit_result(m);
        if (m->hooks.exit) m->hooks.exit(m->hooks.user, m->result_code);
    }
}

/* FUN_8008A0B0 (CONFIRMED table; jump table 0x8004C164). */
int ac1_mission_cmd(ac1_mission *m, int cmd, int arg) {
    switch (cmd) {
    case 1:  /* clear a method-table flag on a range of objects */      return 0;
    case 2:  /* FUN_8008C2FC: load secondary-VM thread set `arg` */     return 0;
    case 3:  /* FUN_8008A048: toggle group enable */                    return 0;
    case 4:  ac1_mission_set_timer(m, (uint32_t)arg, 128);              return 0;
    case 5:  ac1_mission_set_result(m, AC1_MIS_FLAG_PROGRESS);          return 0;
    case 6:  /* enable group + per-object counter (FUN_8008A080) */     return 0;
    case 7:  /* group + counter */                                      return 0;
    case 8:  /* prompt-gated success: FUN_80052A2C(99) == 1 -> 0x100.
                The prompt is a host decision here: arg != 0 means "the
                prompt returned 1" (exact trigger: LIVE_CHECKLIST MIS-3). */
             if (arg) ac1_mission_set_result(m, AC1_MIS_FLAG_SUCCESS);
             return 0;
    case 9:  if (m->obj && m->obj->event) m->obj->event(m, arg);        return 0;
    case 10: /* broadcast 0x8200+sub to a range of objects */           return 0;
    default: return -1;
    }
}

/* One mission frame of the mode-2 handler body (~0x8004C3xx): the driver
 * FUN_8008AB68 is called at 0x8004C480 (CONFIRMED order inside it), and the
 * timer-tick handler (~0x8004C4F0, FAIL call at 0x8004C5F0) runs after it in
 * the same body — so a timer that expires this frame arms the fade for the
 * NEXT frame's end step. */
void ac1_mission_frame(ac1_mission *m) {
    if (m->exited) return;
    m->frame++;
    if (m->obj && m->obj->tick) m->obj->tick(m);                      /* [*0x8019F51C+0] */
    if (m->end_anim < AC1_MIS_END_ANIM_ACTIVE_LT) ac1_mission_eventflags_tick(m);
    if (m->end_counter != 0) end_step(m);
    timer_tick(m);
}

/* 0x8004C69C (CONFIRMED): 0x100 -> 1, 0x200 -> 2, neither -> 0. The order when
 * both bits are set is not documented; success is tested first here. Note that
 * the live border-exit path commits 0x80 (cmd 5), which this table maps to 0 —
 * how the debrief distinguishes success is LIVE_CHECKLIST MIS-16. */
uint32_t ac1_mission_exit_result(const ac1_mission *m) {
    if (m->flags & AC1_MIS_FLAG_SUCCESS) return AC1_MIS_RESULT_SUCCESS;
    if (m->flags & AC1_MIS_FLAG_FAIL)    return AC1_MIS_RESULT_FAIL;
    return 0;
}

const char *ac1_mission_phase(const ac1_mission *m) {
    if (m->exited)                                  return "EXITED";
    if (m->end_counter != 0)                        return "ENDING";
    if (m->objectives_state == 3)                   return "OBJECTIVES_COMPLETE";
    if (m->terminal)                                return "TERMINAL";
    if (m->frame == 0)                              return "LOADED";
    return "RUNNING";
}
