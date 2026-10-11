# farsi_name_xpos.s — proportional right-justify for the Farsi pilot/AC name.
#
# Hand-assembled (GCC's -Os emitted ~192 B; this is ~100 B to fit the proven-free
# tail of the 200 B padding block at 0x80081FD0..0x80082038).
#
# Sums the actual fmet glyph advances over the '>'-terminated name buffer and
# anchors the right edge at x=136:  name_X = 136 - Sum(adv), cursor_X = name_X-16.
# Tail-jumps to cursext (0x80082130) for the per-frame label fix (type=6, X=72);
# cursext expects at = 0x801a0000 (set here) and ends with jr ra.
#
# Installed by repointing cursor.update @0x80083A28 -> `j name_xpos`. Runs only on
# the name screen (cursor element's update method), where the buffer is always
# '>'-terminated, so the loop needs no explicit cap.
#
#   NAME = 0x80031be6   FMET = 0x80065da0 (6-byte entries, adv at +5)

    .set    noreorder
    .set    noat
    .globl  name_xpos
name_xpos:
    lui     $v0, 0x8003
    addiu   $v0, $v0, 0x1be6        # v0 = NAME
    lui     $a0, 0x8006
    addiu   $a0, $a0, 0x5da0        # a0 = FMET
    move    $v1, $zero              # v1 = sum
    li      $t1, 0x3e               # terminator
1:
    lbu     $t0, 0($v0)
    addiu   $v0, $v0, 1
    beq     $t0, $t1, 2f
    andi    $t2, $t0, 0x7f          # (delay) glyph index = byte & 0x7f
    sll     $t3, $t2, 1
    addu    $t3, $t3, $t2           # index*3
    sll     $t3, $t3, 1             # index*6
    addu    $t3, $t3, $a0           # &fmet[index*6]
    lbu     $t3, 5($t3)             # adv
    b       1b
    addu    $v1, $v1, $t3           # (delay) sum += adv
2:
    lui     $at, 0x801a
    li      $t0, 136
    subu    $t0, $t0, $v1           # name_X = 136 - sum
    sh      $t0, 0x7500($at)        # 0x801a7500
    addiu   $t0, $t0, -16           # cursor_X = name_X - 16
    j       0x80082130             # cursext: label fix (type=6, X=72) + jr ra
    sh      $t0, 0x7360($at)        # (delay slot) 0x801a7360
