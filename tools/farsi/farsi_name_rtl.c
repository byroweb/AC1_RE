/*
 * farsi_name_rtl.c — RTL layout patches for the AC1 pilot-name screen.
 *
 * The complete RTL Farsi name-entry screen. All addresses confirmed live via the
 * DuckStation MCP (see project_ac1_name_rtl_layout memory). Screen is 320x240,
 * glyph advance 16px, field = 8 columns (X=8,24,..,120), MAX name = 8.
 *
 *   (1) RIGHT-JUSTIFY NAME + REVERSE CURSOR  — rewrite cursor.update @0x80083A28
 *       (its tail also drives the label flip below — see cursext / part (3)).
 *   (2) MOVE WHOLE BOX TO THE RIGHT          — boxhook @0x80082108 (X 24 -> 168)
 *   (3) LOCALIZE LABEL "PILOT NAME"->نام خلبان — type 7->6 + right-justify via cursext
 *   (4) TRANSLATE KEYBOARD SPC/END -> فاصله / تمام (relocated row string)
 *       (+ surgical ا/آ alef X-spacer nudges)
 *
 * Children (label / name field / cursor) are positioned RELATIVE to the box, so
 * (2) moves the frame + label + cursor + name field as one unit, and (1)'s
 * box-relative anchors keep working unchanged in the new position.
 *
 * Status: ALL of the above are BAKED into "Armored Core (v1.1) [RTL].bin" via
 * build_rtl_patch.py (the executable source of truth) and confirmed BOOTING from
 * a fresh disc (no save state). Baking the FDAT overlay requires recomputing the
 * trailing checksum (see project_ac1_overlay_checksum) or the game hangs at NOW
 * LOADING. The runtime per-keystroke name SHAPER is a separate, later task.
 */
typedef unsigned char  u8;
typedef unsigned short u16;
typedef short          s16;
typedef unsigned int   u32;

/* ---- shared address map (name-entry screen) ------------------------------ */
#define NAME_BUF   ((u8 *)0x80031be6)   /* '>'(0x3e)-terminated display bytes  */
#define NAME_X     ((s16 *)0x801a7500)  /* name element  +0x48 (box-relative)  */
#define CURSOR_X   ((s16 *)0x801a7360)  /* cursor element +0x48                 */
#define NAME_N     ((u8 *)0x801a28ee)   /* glyph count n (state 0x801a2858+0x96)*/
#define GLYPH_W    16
#define FIELD_RIGHT 136                 /* right edge of the 8-col field        */

/* ======================================================================== *
 * (1) cursor.update()  @ 0x80083A28  (the cursor element's +0x74 method)
 *     Runs every frame. Original computed only cursor_X = n*16 + 8 (LTR).
 *     Rewrite below recomputes BOTH coords from n and stores them to their
 *     fixed addresses (absolute stores -> safe even if another element ever
 *     shares this method). 12 words / 48 bytes — fits the original exactly.
 * ======================================================================== */
void cursor_update_rtl(void)            /* a0 = cursor element (unused here) */
{
    int n = *NAME_N;                    /* lw v0,0x7c(a0); lbu a1,0x96(v0)   */
    *CURSOR_X = (s16)(120 - n * GLYPH_W);   /* one slot left of leftmost glyph */
    *NAME_X   = (s16)(FIELD_RIGHT - n * GLYPH_W); /* right-justified run       */
    cursext();                          /* tail call (see part 3): label fixups */
    /* TODO clamp: when n>=8 (full) cursor_X = -8 (off-screen left). Original
       clamped to the last slot; pick a sane full-field cursor before baking. */
}
/* BAKED patch bytes @0x80083A28 (mipsel, 48B). Identical to the validated cursor
 * rewrite EXCEPT the final `jr ra` (word 11) is now `j 0x80082130` (cursext); the
 * delay slot `sh v0,0x7500(at)` (name_X) is unchanged, so cursext returns via its
 * own jr ra (see part 3). at stays = 0x801A0000 across the tail call.
 *   7c00828c 00000000 96004590 1a80013c 00290500 78000224
 *   23104500 607322a4 88000224 23104500 4c080208 007522a4
 *                                        ^^^^^^^^ j 0x80082130 (was 0800e003 jr ra)
 * Restore (original 48 bytes):
 *   7c00828c 00000000 96004590 08000224 ff00a330 03006214
 *   00110500 07000524 00110500 08004224 0800e003 480082ac
 */

/* ======================================================================== *
 * (2) MOVE BOX TO THE RIGHT
 *     box_init @0x8005B648 (X=a0,Y=a1,W=a2,H=a3) builds the window border as
 *     CACHED GPU prims at screen-init (NOT redrawn per-frame). Caller @0x8005C2A8
 *     reads X/Y/W/H from a descriptor struct s2: X=lh(s2+0xe), Y=+0x10, W=+0x12,
 *     H=+0x14. PILOT NAME box descriptor (runtime RAM ~0x801BCF42): X=24,Y=178,
 *     W=144,H=48 -> element base 0x801A7248 (X @0x801A72C4).
 *
 *     Right-edge align with the keyboard panel: X = 312 - 144 = 168 (see below).
 *     Keyboard panel (also via box_init: X=24,Y=40,W=288,H=136) must stay at
 *     X=24, so the patch must be PILOT-NAME-specific — keyed on Y==178, not a
 *     shared constant (the box caller is shared with the memory-card slot boxes).
 * ======================================================================== */
/* Align the box's RIGHT edge with the keyboard panel's right edge (the stock
 * screen aligned their LEFT edges at X=24). Keyboard panel right = 24+288 = 312;
 * name box W=144 -> X = 312 - 144 = 168. */
#define KBD_RIGHT      (24 + 288)        /* keyboard panel right edge = 312 */
#define BOX_W          144
#define BOX_X_RIGHTALIGN (KBD_RIGHT - BOX_W)   /* = 168 */
/* Live demo: poke the box element X (0x801A72C4) at init, right at
 *   0x8005B7B8 'lh a1,0x7c(s3)' (before prim-build), value = 168.
 * BAKED (2026-06-07) — name-screen-SPECIFIC trampoline at the shared box caller:
 *   The caller @0x8005C284.. loads X=lh(0xe(s2)),Y=lh(0x10(s2)),W,H then jal
 *   box_init. Replace `lh a3,0x14(s2)` @0x8005C290 with `j boxhook`; boxhook
 *   redoes the load and, ONLY if a1(Y)==178 (the PILOT NAME box's Y), forces
 *   a0(X)=168, then `j 0x8005C298`. Keyboard panel (Y=40) and memory-card slot
 *   boxes pass through untouched. Children (label/name/cursor) follow because
 *   they are box-relative. Confirmed booting from a fresh disc. */

/* ======================================================================== *
 * (4) TRANSLATE KEYBOARD KEYS  SPC -> فاصله,  END -> تمام
 *     The "SPC END" label is one type-6 row element (0x801A4C18, strptr +0xc =
 *     0x800823D4 -> "SPC END>" = 8 bytes, immediately followed by the 0x3e that
 *     the unused rows share — so it can't be overwritten longer in place).
 *     Relocate the row string to free RAM and repoint +0xc. Lay فاصله at the
 *     SPC slot (left) and تمام at the END slot (right): each word in its own
 *     draw order, separated by spaces (tune the gap to the selection cells).
 *     Shaped (farsi-translate skill): فاصله = ED DE B5 81 CD ; تمام = E0 81 E2 8B.
 *     BAKED gap = 5 spaces so the two keys read as distinct phrases, not one.
 *
 *   KEYBOARD ROW GLYPH FORMAT (RE'd 2026-06-07, rows draw LTR; RTL look comes
 *   from pre-reversed glyph order):
 *     A row string is a run of 1-byte glyph codes terminated by 0x3e. A VISIBLE
 *     letter (code >= 0x80) is drawn then followed by spacer/positioning bytes;
 *     in the stock letter/digit rows each visible glyph is trailed by TWO blank
 *     "spacer" glyphs (a Y-advance byte then an X-advance byte) that set the gap
 *     to the next letter — e.g. row2 digits are f6 7d 3c | f7 7d 3c | ... (the
 *     0xf6..0xff are ۰..۹; 0x7d,0x3c are width-spacers). Shrinking a letter's
 *     X-spacer nudges it RIGHT toward cell centre; growing it nudges LEFT.
 *     Rows: table @0x800B8608, entries -> row0 0x80082340 (top, 17 letters),
 *     row1 0x80082374 (16), row2 0x800823A7 (10 digits), row3 = relocated SPC/END.
 *     A BLANKET right-shift of every non-flagged letter was tried and REVERTED:
 *     it threw the whole alphabet (esp. ا alef) out of alignment. The stock
 *     spacing is good; only do surgical single-glyph nudges when one is off.
 * ======================================================================== */
#define KEYROW_SPCEND_STRPTR ((u32 *)0x801a4c24)   /* element 0x801A4C18 +0x0c */
/* BAKED (clean, per-element): the 8 keyboard rows draw from a strptr TABLE at
 * 0x800B8608 (setup loop @0x80081340: lw a0,table+i*4). SPC/END is row i=3, so
 * table[3] @0x800B8614 = 0x800823D4 ("SPC END>"). Bake = write "فاصله تمام>"
 * into free overlay space 0x800820E8 and repoint table[3] -> 0x800820E8. The row
 * is already type 6 (Farsi renderer), so no type change and no other-screen risk.
 * (Visual re-confirm pending an empty memory-card slot to reach New Game name
 * entry; the name screen itself renders these rows fine.) */
#define KEYROW_TABLE      0x800b8608
#define KEYROW_SPCEND_TAB 0x800b8614            /* table[3] -> relocate target   */
#define SPCEND_RELOC      0x800820e8            /* free entry-201 space          */
static const u8 KEY_FASELE_TAMAM[] = {          /* فاصله + 5 spaces + تمام + '>' */
    0xED,0xDE,0xB5,0x81,0xCD, 0x20,0x20,0x20,0x20,0x20, 0xE0,0x81,0xE2,0x8B, 0x3e
};

/* ======================================================================== *
 * (3) LOCALIZE LABEL  "PILOT NAME" -> "نام خلبان"
 *     Label = element 0x801A73E8. As shipped it is TYPE 7 (+0x0a) -> dispatcher
 *     routes type 7 to plain draw_string (ASCII font, '>' printed literally).
 *     The Farsi-capable path is TYPE 6 -> 0x80059b50 (same path the name field
 *     uses; renders bytes>=0x80 as Farsi glyphs and honours the 0x3e terminator).
 *     Flipping the type is sufficient — the shaped bytes then render correctly.
 *
 *     Label string slot @0x8004C6F4 ("PILOT NAME>" = 11 bytes). Shaped
 *     "نام خلبان" = 10 bytes incl terminator, so it overwrites in place; the
 *     element's strptr (+0x0c) already points there.
 * ======================================================================== */
#define LABEL_ELEM   0x801a73e8
#define LABEL_TYPE   ((u16 *)0x801a73f2) /* +0x0a : set 7 -> 6                  */
#define LABEL_STR    ((u8  *)0x8004c6f4) /* shaped bytes go here                */
#define LABEL_X      ((s16 *)0x801a7430) /* +0x48 (NOT 0x7530 — earlier typo). */
/* RIGHT-JUSTIFY label: type-6 draws LTR from X=left edge; set X=72 (0x48, box-
 * relative) so نام خلبان's right edge sits just inside the box's right border.
 * BAKED via cursext (below) writing LABEL_X each frame. */
/* BUILDER NOTES (how the type flip was settled):
 *  - The label is made by a SHARED type-7 text constructor that HARDCODES type 7
 *    at 0x8005D868 'addiu v1,zero,7' (then 'sh v1,0xa(v0)'), strptr at 0x8005D88C.
 *    Patching that immediate 7->6 is REJECTED (garbles memory-card SLOT/"Load
 *    saved data?" labels — type-6 runs past their \0).
 *  - A name-screen-specific trampoline that flips type 7->6 DURING construction
 *    (keyed on strptr==0x8004C6F4) also FAILED: it crashed the render (element
 *    half-built, fault to 0x805EF6CC).
 *  - SOLUTION (see BAKE STATUS at bottom): poke the STORED type=6 every frame,
 *    post-build, from cursext (tail-called by cursor.update). */

/* "نام خلبان" draw-order glyph bytes from farsi_runtime_shape (atlas-matched,
 * validated == build-time shaper). See the farsi-translate skill to regenerate. */
static const u8 LABEL_NAM_E_KHALBAN[] = {
    0xE4,0x81,0x84, 0xDE,0x9F, 0x20, 0xE0,0x81,0xE5, 0x3e
};

/* The shaped bytes are baked into LABEL_STR statically (build_rtl_patch.py). The
 * type flip + right-justify are applied at runtime by cursext(), tail-called from
 * cursor.update every frame (post-build → no crash; only runs on the name screen).
 * cursext lives at 0x80082130 in free overlay space; `at` is still 0x801A0000 from
 * cursor.update, so both stores are at+offset. Baked bytes:
 *   addiu v0,zero,6  ; sh v0,0x73F2(at)   // LABEL_TYPE = 6 (Farsi renderer)
 *   addiu v0,zero,72 ; sh v0,0x7430(at)   // LABEL_X   = 72 (right-justify)
 *   jr ra ; nop                            // return into the UI update loop      */
void cursext(void)
{
    *LABEL_TYPE = 6;                    /* route through the Farsi renderer    */
    *LABEL_X    = 72;                   /* right-justify نام خلبان in the box   */
}
/* BAKE STATUS (2026-06-07) — SOLVED + shipped booting.
 * Flipping the type 7->6 *during* construction crashed the render (element half
 * built; CPU faulted to 0x805EF6CC). A POST-build poke of *LABEL_TYPE=6 always
 * renders نام خلبان fine. So do the poke every frame from a point that only runs
 * on the name screen and only after setup: the cursor.update method @0x80083A28
 * (the same one that right-justifies the name). Its final `jr ra` is replaced
 * with `j cursext`; the delay slot (sh name_X) is unchanged, and cursext (24B in
 * free overlay space @0x80082130) is, with at still = 0x801A0000:
 *      addiu v0,6 ; sh v0,0x73F2(at)   // *(0x801A73F2) label type = 6
 *      addiu v0,72; sh v0,0x7430(at)   // *(0x801A7430) label X = 72 (right-justify)
 *      jr ra ; nop
 * Net: label type latched to 6 and right-justified each frame. Confirmed booting
 * from a fresh disc with the box at 168 and no crash. (Earlier dispatcher-hook
 * @0x8009C13C attempt did not latch — the cursor.update tail-call is the fix.) */
