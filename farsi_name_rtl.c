/*
 * farsi_name_rtl.c — RTL layout patches for the AC1 pilot-name screen.
 *
 * Three independent, verified-in-RAM changes that together make the name-entry
 * screen a right-to-left Farsi layout. All addresses confirmed live via the
 * DuckStation MCP (see project_ac1_name_rtl_layout memory). Screen is 320x240,
 * glyph advance 16px, field = 8 columns (X=8,24,..,120), MAX name = 8.
 *
 *   (1) RIGHT-JUSTIFY NAME + REVERSE CURSOR  — rewrite cursor.update @0x80083A28
 *   (2) MOVE WHOLE BOX TO THE RIGHT          — box descriptor X 24 -> 152
 *   (3) LOCALIZE LABEL "PILOT NAME"->نام خلبان — type 7->6 + shaped bytes
 *
 * Children (label / name field / cursor) are positioned RELATIVE to the box, so
 * (2) moves the frame + label + cursor + name field as one unit, and (1)'s
 * box-relative anchors keep working unchanged in the new position.
 *
 * Status: proven by live RAM poke (save slots 3=box-right, 5=full+Farsi label).
 * To bake into the FDAT overlay: apply the byte patches below, then recompute
 * the overlay trailing checksum (see project_ac1_overlay_checksum) or the game
 * hangs at NOW LOADING.
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
    /* TODO clamp: when n>=8 (full) cursor_X = -8 (off-screen left). Original
       clamped to the last slot; pick a sane full-field cursor before baking. */
}
/* Exact patch bytes @0x80083A28 (mipsel, validated live):
 *   7c00828c 00000000 96004590 1a80013c 00290500 78000224
 *   23104500 607322a4 88000224 23104500 0800e003 007522a4
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
 *     Mirror to the right half: X = 320 - 24 - 144 = 152. Keyboard panel
 *     (also via box_init: X=24,Y=40,W=288,H=136) must stay at X=24, so patch
 *     must be PILOT-NAME-specific — change this box's descriptor X only, not a
 *     shared constant.
 * ======================================================================== */
/* Align the box's RIGHT edge with the keyboard panel's right edge (the stock
 * screen aligned their LEFT edges at X=24). Keyboard panel right = 24+288 = 312;
 * name box W=144 -> X = 312 - 144 = 168. */
#define KBD_RIGHT      (24 + 288)        /* keyboard panel right edge = 312 */
#define BOX_W          144
#define BOX_X_RIGHTALIGN (KBD_RIGHT - BOX_W)   /* = 168 */
/* Live demo: poke the box element X (0x801A72C4) at init, right at
 *   0x8005B7B8 'lh a1,0x7c(s3)' (before prim-build), value = 168.
 * Permanent: set the PILOT NAME box descriptor's X field (s2+0xe) = 168 at its
 * source. Descriptor origin still TODO (built at runtime into 0x801BCFxx). */

/* ======================================================================== *
 * (4) TRANSLATE KEYBOARD KEYS  SPC -> فاصله,  END -> تمام
 *     The "SPC END" label is one type-6 row element (0x801A4C18, strptr +0xc =
 *     0x800823D4 -> "SPC END>" = 8 bytes, immediately followed by the 0x3e that
 *     the unused rows share — so it can't be overwritten longer in place).
 *     Relocate the row string to free RAM and repoint +0xc. Lay فاصله at the
 *     SPC slot (left) and تمام at the END slot (right): each word in its own
 *     draw order, separated by spaces (tune the gap to the selection cells).
 *     Shaped (farsi-translate skill): فاصله = ED DE B5 81 CD ; تمام = E0 81 E2 8B.
 * ======================================================================== */
#define KEYROW_SPCEND_STRPTR ((u32 *)0x801a4c24)   /* element 0x801A4C18 +0x0c */
/* "فاصله تمام>" : EDDEB581CD 20 E081E28B 3E  (relocate to a free slot, e.g. one
 * found via find_free_ram; live demo used 0x800820E8). */
static const u8 KEY_FASELE_TAMAM[] = {
    0xED,0xDE,0xB5,0x81,0xCD, 0x20, 0xE0,0x81,0xE2,0x8B, 0x3e
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
#define LABEL_X      ((s16 *)0x801a7530) /* +0x48 (right-align: still tuning)   */

/* "نام خلبان" draw-order glyph bytes from farsi_runtime_shape (atlas-matched,
 * validated == build-time shaper). See the farsi-translate skill to regenerate. */
static const u8 LABEL_NAM_E_KHALBAN[] = {
    0xE4,0x81,0x84, 0xDE,0x9F, 0x20, 0xE0,0x81,0xE5, 0x3e
};

void apply_label_farsi(void)
{
    for (unsigned i = 0; i < sizeof LABEL_NAM_E_KHALBAN; i++)
        LABEL_STR[i] = LABEL_NAM_E_KHALBAN[i];
    *LABEL_TYPE = 6;                    /* route through the Farsi renderer    */
    /* *LABEL_X = ...  right-align TODO: type-6 path position handling differs;
       0x801A7530 nudge had little effect — find the type-6 X source.          */
}
