# AC1 link-cable versus — how two consoles talk over the serial port

Target **SLUS-01323 (v1.1)**. Static RE, 2026-10-21: offline `objdump` of the carved exe
(base `0x80010800`) and the on-disc FDAT 203 program overlay
(`disc_map/overlays/ovl203_mode1.bin`, base `0x8004ADA0`, main entry `0x8004BF94`, see
[`OVERLAY_MAP.md`](../overlays/OVERLAY_MAP.md)). `CONFIRMED` = read in disassembly.
`HYPOTHESIS` = inferred and not yet checked on a running pair of consoles.

**Short version.** The exe installs PsyQ's `libcomb` serial driver as device `sio:` and
sets it to its fastest rate. ovl203 is the only overlay that reads or writes it. Every
packet is **8 bytes, framed `'R' … 'C'`**. One handshake decides who is master and who is
slave. Then 79 packets swap the two AC builds, one packet acts as a start barrier, and
the battle runs as a **deterministic input lockstep**. Each frame both consoles send
their pad, a frame checksum and a random-number byte. Each applies both pads one frame
late. Nothing is ever resent. A desync or a 5-second silence ends the match with an
error banner.

---

## 1. Serial driver setup (exe)  CONFIRMED

Done once at boot in the exe init `0x800167D0`, at `0x80016984..0x800169DC`:

| step | call | meaning |
|---|---|---|
| 1 | `0x80030368` | `AddCOMB()`: `EnterCriticalSection` → `AddDevice(0x80039C0C)` (BIOS B(47h)) → `ExitCriticalSection`. |
| 2 | `open("sio:" @0x80039C98, 2)` → `0x8004A288` | **write fd**, synchronous (`O_WRONLY`). |
| 3 | `open("sio:", 0x8001)` → `0x8004A284` | **read fd**, asynchronous (`O_RDONLY \| FASYNC`). |
| 4 | `_comb_control(1, 3, 2073600)` | `CombSetBPS`: the driver writes `2073600 / bps` = **reload 1** to `SIO_BAUD` (`0x1F80105E`). |
| 5 | `_comb_control(1, 4, 8)` | `CombSetPacketSize(8)`: a table at `0x80039BF0` maps 8 to `SIO_CTRL` bits 8–9 = 3, so the **RX interrupt fires every 8 bytes**. |

- **Device control block** `0x80039C0C`: name `"sio"`, flags 3, block size 1,
  description `"SIO console"`. Entry points: init `0x800312D0`, open `0x800313F0`,
  in/out `0x800314A0`, close `0x800313F8`.
- **Register base** `*(0x80039BD4) = 0x1F801050` (SIO port 1). The driver keeps shadow
  copies of MODE, CTRL and BAUD at `0x80040800`, `0x80040802` and `0x80040806`.
- **Driver init defaults** `0x800312D0`: `MODE = 0xCE` (×16 clock factor, 8 data bits,
  no parity, 2 stop bits), `BAUD = 0xD8` (216, which is 9600 by libcomb's formula),
  `CTRL = 5` (TX and RX enabled). AC1 then overrides BAUD to 1.
- **Line rate.** Nominally 2,073,600 bit/s. HYPOTHESIS: on hardware this is
  33.8688 MHz / (16 × 1) ≈ 2.12 Mbit/s. At 11 bits per byte, one 8-byte packet takes
  about 42 µs on the wire.
- **`_comb_control` = `0x800309A8`** (`cmd`, `arg`, `param`). Jump tables are at
  `0x80011E18` (cmd), `0x80011E30` (cmd 0) and `0x80011E4C` (cmd 1). The subcommands AC1
  uses:
  - `(2,3,0)` `CombCancelRead`: clears `CTRL` bits `0x800 | 0x20` (RX IRQ and RTS) and the
    pending-read state. The exe wrapper is `0x800167A8`.
  - `(3,1,0)` `CombCTS`: returns `SIO_STAT` bit 8 (the CTS input).
  - `(4,0,fn)` `CombWaitCallback`: swaps the callback at `0x80040808` and returns the old
    one. The driver's blocking read and write loops call it while they spin
    (`jalr` at `0x800305CC` and `0x8003083C`).
- **Async read arms RTS.** The async-read setup `0x800303E8` ORs `0x800 | 0x20` into
  `CTRL`. So a console with a read pending raises RTS, and the peer sees that as CTS. The
  handshake below relies on this.
- **SIO events.** The exe also opens two events of class `0xF000000B`, the class the
  driver passes to `DeliverEvent`. One is spec `0x400`, polled, at `0x8004A278`. The
  other is spec `0x8000` (error) with handler `0x80015864`, at `0x8004A27C`. ovl203 never
  tests either. It polls the receive buffer directly (§3).

## 2. Shared state  CONFIRMED

| address | what |
|---|---|
| `0x8004A284` / `0x8004A288` | read fd (async) / write fd (sync) |
| `0x8004A28C..93` | **rx** packet (8 bytes) |
| `0x8004A298..9F` | **tx** packet (8 bytes) |
| `0x800895C4` | timestamp of the last send/wait start (VBlank count) |
| `0x800895C8` | **fatal link error** byte. Once set, all exchange stops. |
| `0x80049F94` | VBlank tick counter. Incremented by the `RCntCNT3` (VBlank) callback `0x80016220`. Read with `0x8001628C` (now) and `0x80016278(t)` (now − t). |
| `0x80039D30` / `0x80039D34` | pointers to the **local** / **remote** player record (§4) |
| `0x80040FA0`, `0x8004133C` | the two player records, slot 1 and slot 2 (stride `0x39C`). Record `+0x0` is the raw pad, `+0x4` the previous raw pad, `+0x8` the gameplay pad, `+0xC` the previous gameplay pad (cf. [PLAYER_CONTROL.md](PLAYER_CONTROL.md)). |
| `0x8008A21C` | link frame counter (u16, +1 per exchange) |
| `0x801AB48C` | battle status flags. Link bits: `0x100`, `0x200`, `0x400` (§6). |
| `0x801AB490` | battle end countdown (frames) |
| `0x801AB481` | 1 on the master, 0 on the slave |

All timeouts below are counted in VBlanks, so 300 is 5 s and 1800 is 30 s on NTSC.

## 3. Packet format  CONFIRMED

| byte | handshake `0x8004F4E0` | AC-data transfer `0x8004F3C4` | per-frame `0x80077E10` |
|---|---|---|---|
| 0 | `'R'` (0x52) | `'R'` | `'R'` |
| 1 | role: `'M'` (0x4D) or `'S'` (0x53) | (stale role byte) | **frame checksum** |
| 2–3 | — | payload halfword *n* | **pad** (u16, port 1) |
| 4–5 | — | payload halfword *n*+1 | 0 |
| 6 | — | — | **rand byte** |
| 7 | `'C'` (0x43) | `'C'` | `'C'` |

- **Every exchange is symmetric.** The sender clears rx[0] and rx[7] and arms an async
  `read(rfd, rx, 8)`. It then does a blocking `write(wfd, tx, 8)` and calls the wait
  helper.
- **Wait helper** `0x8004BEB0`. It spins until rx[0] == `'R'` or rx[7] == `'C'`, then
  returns 1. If 300 VBlanks pass first, it sets `0x800895C8 = 1` and returns 0. Bytes 1–6
  carry **no integrity check**: framing is only the first and last byte.
- **Write timeout** `0x8004BE10`. It is installed as the `CombWaitCallback` by
  `0x8004BE54` at battle init and removed by `0x8004BE88` at exit. It applies the same
  300-VBlank rule to the blocking write. HYPOTHESIS: returning 0 makes the driver abandon
  the write.

## 4. Handshake and role selection  CONFIRMED

`0x8004F4E0` is called from the scene setup `0x8004FE84` (at `0x8004FEA8`), which is
called from the ovl203 main entry `0x8004BF94`.

1. **Build the packet.** Clear rx[0], rx[1] and rx[7]. Set tx = `'R' 'S' … 'C'` and arm
   the async read.
2. **Pick a role.** Poll `CombCTS` for up to 1800 VBlanks. Each poll that finds CTS low
   rewrites tx[1] = `'M'`. On timeout, set `0x800895C8 = 1` and return.
   - So a console that finds its partner **not yet listening** becomes the **master**.
   - A console whose partner is already listening (CTS high on the first poll) stays the
     **slave**.
3. **Exchange.** Write tx and wait for rx.
4. **Resolve.**
   - **Collision:** rx[1] is `'M'` or `'S'` and equals tx[1]. **Garbage:** rx[1] is
     neither. Either way, back off `VSync(3 + (rand() & 15))` (3–18 frames) and go to
     step 1. HYPOTHESIS: the random delay means one console re-arms its read first and
     so sees no CTS, which breaks the tie.
   - **Master** (tx[1] == `'M'`): local = `0x80040FA0` (slot 1), remote = `0x8004133C`
     (slot 2). `0x801AB481 = 1`.
   - **Slave:** copy its own record `0x80040FA0` → `0x8004133C` (`0x39C` bytes). Then
     local = `0x8004133C`, remote = `0x80040FA0`. `0x801AB481 = 0`.
   - **Result: both consoles end up with the same memory layout.** Slot 1 holds the
     master's AC and slot 2 holds the slave's. This is what lets the same simulation run
     on both.
5. **AC data transfer** `0x8004F3C4(local+0x254, remote+0x254, 79)`.
   - 79 packets, each carrying 2 halfwords (tx[2..5]). Each one is read back from rx[2..5]
     into the remote record.
   - Total: **316 bytes (`+0x254..+0x38F`)** of the player record each way. That block
     includes the AP at `+0x272` and the leg-class byte at `+0x27A` that `player_init`
     reads (PLAYER_CONTROL.md). HYPOTHESIS: it is the whole AC build and stats block.
   - Each packet is lockstep: write, then wait. It stops on the first error.
6. **Reset.** Zero tx, rx and `0x8004A2A0`, then prime rx[0] = `'R'` and rx[7] = `'C'`.

**Record backup.** The entry `0x8004BF94` saves record `0x80040FA0` (`0x39C` bytes) to
`0x8008C490` before the handshake. It copies it back at exit (`0x8004C86C`), so the
slave gets its own AC back in slot 1 after the match.

## 5. Start barrier and RNG seed  CONFIRMED

Later in `0x8004BF94`:

1. `srand(1)` (BIOS A(30h)) at `0x8004C0D0`. **Both consoles seed the BIOS RNG with the
   same constant.** So the RNG stays in sync as long as both make the same `rand()` calls.
2. **Start barrier** `0x8004C0F4..0x8004C1E0`. This is one more `'R' … 'C'` exchange
   behind a 1800-VBlank `CombCTS` wait. Its payload is ignored.
3. Zero tx and rx again and prime rx = `'R' … 'C'`. The first per-frame call then
   "receives" an all-zero packet at once. That packet matches the zeroed tx, so frame 0
   starts with no input and no errors.

## 6. Per-frame lockstep `0x80077E10`  CONFIRMED

This function is called from `0x800780C0`, the per-frame player-input step that then runs
both records (`0x80040FA0` and `+0x39C`). It is also called from `0x80077574`,
`0x8007764C` and `0x800776B0`. Each call does the following:

```c
if (link_error /*0x800895C8*/) goto zero_pads;      // no flag change
r   = rand();                       // BIOS A(2Fh), one draw per frame on both sides
pad = pad_read_0x80016A84(1);       // port1 | port2<<16; only the low half is sent
if (!wait_rx_0x8004BEB0()) goto fail;
// Compare what the peer sent last frame with what WE sent last frame (tx still holds it)
if (rx[1] != tx[1]) flags |= 0x100;          // frame-checksum mismatch
if (rx[6] != tx[6]) flags |= 0x200;          // rand mismatch
// (each of these sets the end countdown 0x801AB490 = 80 if it was 0)
local->pad_prev  = local->pad;   local->pad  = tx[2..3];   // OUR pad from last frame
remote->pad_prev = remote->pad;  remote->pad = rx[2..3];   // PEER pad from last frame
tx[2..3] = pad; tx[0]='R'; tx[7]='C'; tx[6] = (u8)r;
tx[1] = (u8)(frame_lo + local->game_pad + (local->game_pad>>8)
                      + remote->game_pad + (remote->game_pad>>8));  // +8 = last applied
rx[0] = rx[7] = 0;  read(rfd, rx, 8);  write(wfd, tx, 8);
if (link_error) { fail: flags |= 0x300;  zero_pads: local->game_pad = remote->game_pad = 0; }
local->game_pad_prev = local->game_pad;  remote->game_pad_prev = remote->game_pad;
if (!(flags & 0x400)) { local->game_pad = local->pad; remote->game_pad = remote->pad; }
else                  { local->game_pad = remote->game_pad = 0; }
frame /*0x8008A21C*/ ++;
```

- **One frame of input delay, applied the same way on both sides.**
  - The pad sampled on frame N is sent on frame N.
  - On frame N+1, each console applies its own pad from tx (the one it just sent) and the
    peer's pad from rx.
  - Both consoles therefore feed the same two inputs into the same slots on the same
    frame.
- **The wait is at the top of the call.** The reply to frame N's packet is awaited at the
  start of frame N+1. The serial transfer overlaps the rest of the frame.
- **Bandwidth:** 8 bytes each way per frame.
- **Desync detectors.**
  - The checksum (byte 1) covers the frame counter and the last applied gameplay pads of
    both records. It catches lost or misaligned frames and pad disagreement.
  - The rand byte (byte 6) catches RNG divergence: one side making a different number of
    `rand()` calls.
- **`flags & 0x400` zeroes both gameplay pads.** HYPOTHESIS: this is the same input lock
  used for scripted or ending sequences (cf. `0x8019F524 & 0x400` in the mission
  overlay).
- **After a fatal error** the `zero_pads` path still falls through to the copy below
  it. So the gameplay pad is not left at 0: it is reloaded from the frozen raw pad, while
  the previous gameplay pad is 0. HYPOTHESIS: held buttons then look newly pressed on
  every frame until the 80-frame countdown ends the match.
- **Analog pads.** `pad_read` only accepts a controller whose type nibble is 4 (digital).
  Any other type reads as no input (`0x80016A3C`).

## 7. Errors, timeouts and the on-screen banner  CONFIRMED

| condition | sets | effect |
|---|---|---|
| CTS never rises (handshake or barrier), 1800 VBlanks | `0x800895C8 = 1` | the handshake or barrier aborts. The main loop is skipped (`0x8004C0EC`, `0x8004C1CC`). |
| no `'R'`/`'C'` packet, or a blocking write stalls, 300 VBlanks | `0x800895C8 = 1`, `flags \|= 0x300` | all further exchange is skipped. The raw pads at `+0x0` stop updating, so each frame the gameplay pad is set back to the last raw pad (input freezes) and the previous gameplay pad reads 0 (§6). |
| rx[1] ≠ tx[1] | `flags \|= 0x100` | banner **`REMOTE:ERROR:FRAME`** |
| rx[6] ≠ tx[6] | `flags \|= 0x200` | banner **`REMOTE:ERROR:RAND`** |
| both bits (timeout path) | `flags \|= 0x300` | banner **`LINK CABLE ERROR`** |

- **Banner selection.** The HUD routine `0x80051B84` switches on `*(0x801AB48C) & 0x300`
  and draws the string through `0x80051E1C`. The strings are at `0x8004ADCC`,
  `0x8004ADE0` and `0x8004ADF4`.
- **Ending the match.** Each error path arms the end countdown `0x801AB490 = 80` if it was
  idle. The main loop (`0x8004C2FC..0x8004C474`) counts it down and leaves the battle at 1.
  At 30 it would normally decide the winner, but it skips that when `0x800895C8` is set.
- **On exit with a fatal error** (`0x8004C8C4`): `CombCancelRead` (`_comb_control(2,3,0)`)
  and `*(u8*)0x80039CAC = 1`. HYPOTHESIS: this flag tells the front end that the link
  failed.
- **No retransmission or recovery.** Neither exchange path ever resends a packet, rewinds,
  or resynchronises. A desync only flags the error and ends the match. A timeout ends the
  link for good.

## 8. Open questions

- **CTS tie-break.** Does the async read drop RTS when its 8 bytes complete? This decides
  whether a collision really settles in one backoff. To check, trace `CTRL` bit `0x20` in
  the driver's IRQ path, or watch two DuckStation instances linked together.
- **Uncopied part of the record.** What is in `+0x000..+0x253` and `+0x390..+0x39B`? Those
  bytes are not transferred, so they must be rebuilt the same way on both consoles (or
  never matter) for the lockstep to hold.
- **The 0x400 input lock.** Which code sets `flags 0x400`, and is the moment it is set
  itself in lockstep?
- **Other callers.** What the three other callers of `0x80077E10` (`0x80077574`,
  `0x8007764C`, `0x800776B0`) are for, such as pre-battle or result screens.
- **`0x80039CAC`.** How the front end consumes it after a failed link.
- **Physical baud.** The real rate with reload 1, and whether hardware tolerates it on
  long cables.
- **Port 2.** Whether a second pad is ever read in link mode. Only the low halfword
  (port 1) is sent.
