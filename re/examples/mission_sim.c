/* mission_sim.c — exercise the mission reference model.
 *
 *   mission_sim                 self-test of the runtime lifecycle (no game data)
 *   mission_sim <FDAT.T>        + validate the data decoders against a real
 *                               extracted FDAT.T (every mission's chunk stream,
 *                               spawn table and secondary-VM blob)
 *   mission_sim <FDAT.T> N      dump mission N: chunks, spawns, script threads
 *
 * Expected on the retail disc (docs/MISSION_SCRIPT_VM.md): every thread walks
 * cleanly to END — 106 threads across the streams that carry a chunk 4. */
#include "ac1_mission.h"
#include "ac1_mxt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond, msg) do { if (!(cond)) { printf("  FAIL: %s\n", msg); fails++; } \
                              else printf("  ok:   %s\n", msg); } while (0)

/* ----------------------------------------------------------------------- */
/* Lifecycle self-test                                                     */
/* ----------------------------------------------------------------------- */
static int   sounds = 0, exits = 0; static uint32_t last_result = 99;
static void on_sound(void *u, int id) { (void)u; (void)id; sounds++; }
static void on_exit(void *u, uint32_t r) { (void)u; exits++; last_result = r; }

/* A fake objective object: an "intro-only" tick like M0/M3 (ENTITY_TYPES.md),
 * which just calls objective_step each frame until terminal. */
static void intro_tick(ac1_mission *m) { if (!m->terminal) ac1_mission_objective_step(m); }

static int selftest(void) {
    int fails = 0;
    ac1_mission m;
    ac1_objobj obj; memset(&obj, 0, sizeof obj);
    ac1_mission_hooks hooks = { NULL, on_sound, NULL, on_exit };

    printf("lifecycle self-test\n");

    /* 1. Timer expiry -> FAIL -> 100-frame fade -> exit with result 2. */
    ac1_mission_init(&m, 0, NULL, &hooks);
    ac1_mission_cmd(&m, 4, 10);                               /* cmd 4: timer = 22*10 */
    CHECK(m.timer == 220, "cmd 4 sets timer = 22 * param");
    CHECK(!strcmp(ac1_mission_phase(&m), "LOADED"), "phase LOADED before first frame");
    while (m.timer) ac1_mission_frame(&m);
    CHECK(m.flags == AC1_MIS_FLAG_FAIL, "timer expiry commits 0x200 (FAIL)");
    CHECK(m.end_counter == AC1_MIS_END_FADE_FRAMES, "set_result arms the 100-frame end fade");
    CHECK(!strcmp(ac1_mission_phase(&m), "ENDING"), "phase ENDING during the fade");
    for (int i = 0; i < 100; i++) ac1_mission_frame(&m);
    CHECK(exits == 1 && last_result == AC1_MIS_RESULT_FAIL, "exit after 100 frames with result code 2");
    CHECK(m.frame == 320, "exactly 220 + 100 frames elapsed");

    /* 2. Intro-only objective object: 36 steps to terminal; the 0x2400 pending
     *    bit is set while stepping and cleared at terminal. */
    exits = 0; sounds = 0;
    obj.tick = intro_tick;
    ac1_mission_init(&m, 3, &obj, &hooks);
    ac1_mission_frame(&m);
    CHECK((m.flags & AC1_MIS_FLAG_OBJ_PENDING) == AC1_MIS_FLAG_OBJ_PENDING, "first step sets 0x2400");
    CHECK(m.end_anim == 0xFFFF && m.progress == 1, "first step: end_anim = 0xFFFF, progress = 1");
    for (int i = 0; i < 40; i++) ac1_mission_frame(&m);
    CHECK(m.terminal == 1 && m.progress == AC1_MIS_PROGRESS_TERMINAL, "terminal at progress 36");
    CHECK((m.flags & AC1_MIS_FLAG_OBJ_PENDING) == 0, "terminal clears 0x2400");
    CHECK(sounds == 35, "jingle on every step except the first (35 of 36)");
    CHECK(exits == 0, "objective_step alone never ends the mission");

    /* 3. Border exit (cmd 5) commits 0x80 and the fade; the result-code table
     *    maps 0x80 to 0 (documented gap: LIVE_CHECKLIST MIS-16). */
    ac1_mission_cmd(&m, 5, 0);
    CHECK(m.flags == AC1_MIS_FLAG_PROGRESS, "cmd 5 stores 0x80");
    for (int i = 0; i < 100; i++) ac1_mission_frame(&m);
    CHECK(exits == 1 && last_result == 0, "exit via cmd 5 -> result code 0 (0x80 is not 0x100)");

    /* 4. Masker: the 0x1000 bit is cleared on the next frame. */
    ac1_mission_init(&m, 0, NULL, &hooks);
    m.flags |= AC1_MIS_FLAG_TIMER_WARN;
    ac1_mission_frame(&m);
    CHECK((m.flags & AC1_MIS_FLAG_TIMER_WARN) == 0, "eventflags_tick clears the transient 0x1000 each frame");

    /* 5. set_result is a store, so SUCCESS after a 0x400 event drops 0x400 —
     *    the behaviour that MIS-15 must confirm or refute. */
    m.flags = AC1_MIS_FLAG_EVENT400;
    ac1_mission_set_result(&m, AC1_MIS_FLAG_SUCCESS);
    CHECK(m.flags == AC1_MIS_FLAG_SUCCESS, "set_result stores (not ORs) the flags word [MIS-15]");
    CHECK(ac1_mission_exit_result(&m) == AC1_MIS_RESULT_SUCCESS, "0x100 -> result code 1");

    /* 6. Spawn marshal: hwK -> +0x04+2K, +0x02 = hw11, +0x00 = 0. */
    ac1_spawn_rec r; for (int k = 0; k < 20; k++) r.hw[k] = (int16_t)(0x1000 + k);
    r.hw[11] = 9800;
    uint8_t t[AC1_TEMPLATE_LEN]; ac1_spawn_marshal(&r, t);
    CHECK(t[0] == 0 && t[1] == 0, "template +0x00 = 0");
    CHECK((t[2] | (t[3] << 8)) == 9800, "template +0x02 = hw11 (HP seed)");
    CHECK((t[4 + 2*3] | (t[5 + 2*3] << 8)) == 0x1003, "template +0x0A = hw3 (block / dispatch type)");
    CHECK((t[4 + 2*11] | (t[5 + 2*11] << 8)) == 9800, "template +0x1A = hw11");

    /* 7. VM decoder: op_f is variable length; unknown words are 2-byte no-ops. */
    uint8_t blob[] = { 0x0F,0x00, 0x02,0x00, 1,0,2,0, 3,0,4,0,   /* op_f cnt=2 */
                       0x0C,0x00,                                 /* reserved noop */
                       0x0A,0x10, 0x00,0x01,                      /* set_result 0x100 */
                       0xFF,0xFF };
    CHECK(ac1_vm_oplen(blob, sizeof blob, 0) == 12, "op_f length = 4 + cnt*4");
    CHECK(ac1_vm_oplen(blob, sizeof blob, 12) == 2, "reserved word = 2-byte no-op");
    CHECK(!strcmp(ac1_vm_lookup(0x100A)->name, "set_result"), "opcode table lookup");
    unsigned nops; size_t nbytes;
    /* wrap it as a thread: thread_hdr then the ops */
    uint8_t th[2 + sizeof blob]; th[0] = 0; th[1] = 0; memcpy(th + 2, blob, sizeof blob);
    CHECK(ac1_vm_thread_walk(th, sizeof th, 0, &nops, &nbytes) && nops == 4 && nbytes == sizeof th,
          "thread walk reaches END (4 instructions)");
    CHECK(!ac1_vm_thread_walk(th, sizeof th - 2, 0, &nops, &nbytes), "truncated thread fails the walk");

    printf("self-test: %s (%d failures)\n", fails ? "FAILED" : "PASS", fails);
    return fails;
}

/* ----------------------------------------------------------------------- */
/* Validate against a real FDAT.T                                          */
/* ----------------------------------------------------------------------- */
static uint8_t *slurp(const char *path, size_t *n) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *b = malloc((size_t)sz);
    if (b && fread(b, 1, (size_t)sz, f) != (size_t)sz) { free(b); b = NULL; }
    fclose(f);
    if (n) *n = (size_t)sz;
    return b;
}

typedef struct { unsigned streams, with_script, sets, threads, bad_threads, spawns_total; } tally;

static void check_stream(const uint8_t *s, size_t n, unsigned mission, tally *t, int verbose) {
    ac1_chunk_iter it; unsigned idx; const uint8_t *p; uint32_t ln; unsigned nchunks = 0;
    ac1_chunk_begin(&it, s, n);
    if (verbose) printf("mission %u: chunk stream %zu B\n", mission, n);
    while (ac1_chunk_next(&it, &idx, &p, &ln)) {
        nchunks++;
        if (verbose) printf("  chunk %2u  %7u B%s\n", idx, ln,
            idx == AC1_CHUNK_GEOMETRY ? "  geometry" : idx == AC1_CHUNK_SCRIPT ? "  script (secondary VM)" :
            idx == AC1_CHUNK_PLACEMENT ? "  placement" : idx == AC1_CHUNK_GEOM_RECORDS ? "  geom records" :
            idx == AC1_CHUNK_SPAWNS ? "  spawns" : "");
    }
    if (nchunks == 0) return;
    t->streams++;

    if (ac1_chunk_find(s, n, AC1_CHUNK_SPAWNS, &p, &ln)) {
        unsigned live = 0; ac1_spawn_rec r;
        for (unsigned i = 0; i < AC1_SPAWN_COUNT && ac1_spawn_get(p, ln, i, &r); i++) {
            if (ac1_spawn_is_empty(&r)) continue;
            live++;
            if (verbose) printf("  spawn %3u: pos (%6d,%6d,%6d) block/type %3d rot %5d res %3d hp %5d\n",
                                i, r.hw[0], r.hw[1], r.hw[2], r.hw[3], r.hw[5], r.hw[7], r.hw[11]);
        }
        t->spawns_total += live;
        if (verbose) printf("  %u non-empty spawn records\n", live);
    } else if (verbose) printf("  (no chunk 12 — script-spawned / transition mission)\n");

    if (ac1_chunk_find(s, n, AC1_CHUNK_SCRIPT, &p, &ln)) {
        t->with_script++;
        unsigned ns = ac1_vm_nsets(p, ln);
        for (unsigned si = 0; si < ns; si++) {
            ac1_vm_set st;
            if (!ac1_vm_set_get(p, ln, si, &st)) break;
            t->sets++;
            if (verbose) printf("  set %u @%u: flags %u count %u extra %u%s\n", si, st.offset,
                                st.flags, st.count, st.extra, st.sentinel ? " (sentinel)" : "");
            for (unsigned ti = 0; ti < st.count; ti++) {
                uint16_t toff = ac1_vm_thread_offset(p, ln, &st, ti);
                unsigned nops = 0; size_t nb = 0;
                int ok = ac1_vm_thread_walk(p, ln, toff, &nops, &nb);
                t->threads++; if (!ok) t->bad_threads++;
                if (verbose) {
                    printf("    thread %u @%u: %s, %u ops, %zu B\n", ti, toff, ok ? "END ok" : "BAD", nops, nb);
                    size_t pc = (size_t)toff + 2;
                    for (unsigned k = 0; k < nops && ok; k++) {
                        uint16_t op = (uint16_t)(p[pc] | (p[pc + 1] << 8));
                        size_t l = ac1_vm_oplen(p, ln, pc);
                        printf("      %04x %-14s", op, ac1_vm_lookup(op)->name);
                        for (size_t b = 2; b + 1 < l; b += 2) printf(" %d", (int16_t)(p[pc+b] | (p[pc+b+1] << 8)));
                        printf("\n"); pc += l;
                    }
                }
            }
        }
    }
}

static int validate(const char *path, int only) {
    size_t n; uint8_t *data = slurp(path, &n);
    if (!data) return 1;
    ac1_mxt mx;
    if (ac1_mxt_open(&mx, data, n, AC1_TOC_AUTO) != 0) { printf("%s: not a .T image\n", path); free(data); return 1; }
    tally t; memset(&t, 0, sizeof t);
    unsigned first = only >= 0 ? (unsigned)only : 0, last = only >= 0 ? (unsigned)only : 99;
    for (unsigned m = first; m <= last; m++) {
        uint32_t e = 2 * m + 1; size_t off, len;
        if (e >= 200 || ac1_mxt_entry(&mx, e, &off, &len) != 0 || len == 0) continue;
        check_stream(data + off, len, m, &t, only >= 0);
    }
    printf("%s: %u mission streams, %u with a script chunk, %u sets, %u threads (%u bad), %u spawns\n",
           path, t.streams, t.with_script, t.sets, t.threads, t.bad_threads, t.spawns_total);
    printf("secondary-VM walk: %s\n", t.bad_threads ? "FAILURES" : "ALL THREADS REACH END");
    free(data);
    return t.bad_threads ? 1 : 0;
}

int main(int argc, char **argv) {
    int rc = selftest();
    if (argc >= 2) rc |= validate(argv[1], argc >= 3 ? atoi(argv[2]) : -1);
    return rc;
}
