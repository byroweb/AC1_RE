# Scene viewer — AC1 levels in the browser, PS1-style, with knobs (`browser-tools/sceneview/`)

A local WebGL2 page that draws a mission's walkable level the way the PS1 draws it, using
the level's real texels from a save state's VRAM. Walk a pilot through it and turn the
knobs that decide how an on-foot mode would look: camera distance, the AC1-style camera that
drops under low ceilings, texture density, texture LOD, floor subdivision, draw distance and
the PS1 rendering quirks.

```bash
python browser-tools/sceneview/server.py --open   # http://127.0.0.1:8771/
python browser-tools/sceneview/scene_test.py      # synthetic tests, no disc needed
```

On Windows, `browser-tools\sceneview\sceneview.cmd` is a double-click launcher (point a desktop
shortcut at it). A second launch just opens the page of the server that is already running.

The disc image comes from `--bin` or `$AC1_BIN`, and save states from the library
(`$AC1_STATES_DIR`, default `~/dev/ac1_states`, see `tools/duckstation/states.py`). Geometry
and texels are game data: the server streams them to 127.0.0.1 only and writes nothing.

```
scene.py        level decode with UVs, TPAGE/CLUT, LOD range and class per face; VRAM from a state
server.py       stdlib HTTP server: /api/missions, /api/level?m=, /api/states, /api/state, /api/vram
index.html      the page (WebGL2, no build step, no CDN)
scene_test.py   synthetic PA block + VRAM-marker tests
```

## What is real and what is a stand-in

- **Geometry** is the mission's chunk-0 blocks, placed by chunk 7 (`docs/LEVEL_LOADING.md`),
  the same decode as AC1mod's `core/level.py`, with one difference: same-wound duplicate faces
  at section seams are dropped.
- **Texels:** the page samples a 1 MB VRAM image the way the GPU does: 4/8/15 bpp, through the
  CLUT, with texel 0x0000 transparent, modulated by the face colour / 0x80. Three sources:
  - **Disc textures (the default; every level).** `GG/COM/RTIM.T` entry 100, then entry *N*,
    uploaded the way mission *N*'s load does (`docs/LEVEL_LOADING.md` §7). This matches a real
    in-mission VRAM byte for byte on the level pages.
  - **A save state's VRAM.** This adds what the game uploads later (AC and object art). A state
    of another mission may not hold this level's pages.
  - **Stand-in noise at the real texel grid.** The grain size is honest, but the colours are made up.
- **Start position** is the player slot 0 position from a save state of this mission, if one
  exists (`docs/PLAYER_CONTROL.md`). Otherwise it is a floor face near the middle of the level.
- **Scale:** the AC's collision box is 212 units tall (`+0x2A`). The "Pilot on foot" preset
  is 1.8 m when the AC is as tall as the *AC height* slider says (10 m by default; that is an
  assumption, not a decoded value).
- **Camera:** a follow camera with distance, height and pitch. *Dynamic ceiling* caps the
  camera under the ceiling above the pilot and pulls it in when a wall or ceiling is between
  them, which is what AC1's camera does in corridors. AC1's own boom constants are not
  surveyed yet (`docs/PLAYER_CONTROL.md` §4), so the defaults are eyeballed. Mission 4 is all
  corridors (every floor has a ceiling under 500 units), which makes it a good test.
- **LOD:** level faces in the detail-only range hide past *Detail LOD dist*. Mission 30's level
  has none; the two-LOD records are mostly in object/AC blocks (`docs/PA_FORMAT.md`).
  *Section draw dist* hides whole placed sections, PS1 pop-in style.

## Why the floor swims on foot

The PS1 maps textures affinely: it interpolates UVs in screen space, without perspective.
The error grows with a polygon's depth range on screen, so it is worst on big polygons
close to the camera. The camera follows an AC from metres away, and level floors are built
from mech-scale tiles. At pilot scale the camera sits close above the same big tiles, so
the floor under the pilot warps every time the camera moves. The viewer reproduces it
(*affine UVs* on) and shows the two period fixes: split near polygons smaller
(*Tessellate*; 32–64 units holds up at pilot scale on missions 4 and 30), or drop them
(*PS1 near drop*). An emulator's PGXP / perspective-correct texturing is the modern third
fix (*affine UVs* off).

## The knobs

| knob | does |
|---|---|
| Pilot height / walk speed | 20–260 units; presets *AC* (212) and *Pilot on foot* (1.8 m) |
| Distance / height above / pitch / FOV | follow-camera boom; *camera scales with pilot* shrinks it with the pilot |
| Ceiling clearance | how far the dynamic camera stays off walls and ceilings |
| Density × | tiles each face's own texel rectangle N times: same texture, smaller texels |
| Texture LOD | merges 2ⁿ×2ⁿ texels (a coarser mip) |
| Tessellate ≤ / walls too | halves the longest edge of floor (and wall) triangles until every edge is ≤ N units: takes the affine warp out of near polygons (capped at 2M triangles) |
| PS1 near drop | does not draw any triangle with a corner closer than N units, the way PS1 games without clipping did: holes instead of warp |
| Texel-density map | red = one texel covers several screen pixels (blocky), green ≈ 1:1, blue = shimmer |
| Internal res, affine UVs, vertex snap, 15-bit + dither | the PS1 look; *Modern look* turns them off |
| Ceilings, wireframe, backface cull, lighting, fog | display helpers |
| Screenshot / AC vs on-foot pair | PNG of the view, or the two scales side by side |

Keys: click the view, then W/S move, A/D turn, Q/E strafe, drag to look, wheel = camera
distance, Shift = fast, F = free fly.
