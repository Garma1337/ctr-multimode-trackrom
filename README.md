# CTR Multi-Gamemode ROM

## Introduction

A track-testing ROM for CrashTeamEditor. Same idea as the `PS1_TrackROM`, but every single-player game mode is selectable from the main menu:

| Row | Mode              | Flags it sets                                             |
|-----|-------------------|-----------------------------------------------------------|
| 0   | ARCADE            | `ARCADE_MODE` + `arcadeDifficulty` (stock difficulty box) |
| 1   | RELIC RACE        | `RELIC_RACE`                                              |
| 2   | TIME TRIAL        | `TIME_TRIAL`                                              |
| 3   | CRYSTAL CHALLENGE | `CRYSTAL_CHALLENGE` + `originalEventTime`                 |
| 4   | CTR CHALLENGE     | `ADVENTURE_MODE` **and** `TOKEN_RACE`                     |
| 5   | SETTINGS          | opens the settings panel; sets no flags                   |

Pick a mode, pick a character, and you land on the level that mode runs on. On a fresh boot that is an original level, so the modes are testable on their own:

| Mode                                          | Level      |
|-----------------------------------------------|------------|
| Arcade, Relic Race, Time Trial, CTR Challenge | Crash Cove |
| Crystal Challenge                             | Skull Rock |

**Once CrashTeamEditor sends a track, every mode runs on that instead**, and stays there until the ROM is restarted.

Oxide is playable and selectable in the character select. The ROM requires an 8 MB RAM environment with shared memory. `SAVE GHOST` on the time trial results screen exports the ghost to the editor rather than to a memory card.

## Settings

`SETTINGS` on the main menu opens a panel for the things a track test needs but the game gives no way to set.

| Field              | Range         | Default |
|--------------------|---------------|---------|
| Relic - Sapphire   | 0.5s - 9:59.5 | 1:17.0  |
| Relic - Gold       | 0.5s - 9:59.5 | 1:05.0  |
| Relic - Platinum   | 0.5s - 9:59.5 | 0:52.0  |
| Crystal Time Limit | 0.5s - 9:59.5 | 2:00.0  |
| Intro Cutscene     | On / Off      | On      |
| Time Trial Ghosts  | On / Off      | On      |

The three relic times are the target times for Relic Race. The crystal limit is the clock for Crystal Challenge, which normally comes from the arena's own stored time. **Intro Cutscene** off skips the race intro camera fly-in, which saves a few seconds on every retry. **Time Trial Ghosts** off stops the N. Tropy / Oxide ghost replay from running.

Edits only take effect when you press Confirm, so a stray press cannot disturb a running setup. Back closes and discards. Holding an adjust direction repeats after a short delay.

The defaults are Crash Cove's real relic times, so a Relic Race on the default level behaves like the retail game until you change something.

CrashTeamEditor can push all six fields in, which is the primary way they are meant to be set — see [Interface to CrashTeamEditor](#interface-to-crashteameditor).

## Input mapping

Bindings are fixed; there is nothing to remap them with.

**Settings panel**

| Input        | Effect                      |
|--------------|-----------------------------|
| Up / Down    | pick a field                |
| Left / Right | -/+ 0.5s, or flip a toggle  |
| L1 / R1      | -/+ 5s                      |
| Cross        | confirm, saves and closes   |
| Triangle     | back, closes without saving |

Only Confirm and Back are hinted on the panel, back on the left and confirm on the right.

**In a race**

| Input    | Effect                                                 |
|----------|--------------------------------------------------------|
| L3 + R3  | toggle freecam                                         |
| SELECT   | toggle the debug HUD                                   |
| Triangle | swap the minimap and the speedometer (stock behaviour) |

**Freecam**

| Input            | Effect                    |
|------------------|---------------------------|
| D-pad            | move and strafe           |
| L1 / R1          | down / up                 |
| Triangle / Cross | pitch                     |
| Square / Circle  | yaw                       |
| L2               | cycle speed               |
| R2               | toggle the on-screen help |

Freecam and the debug HUD only become available once the race intro camera fly-in is over, so neither can be summoned during the intro cutscene. Either one hides the racing HUD while it is up.

## Debug HUD

Press `SELECT` during a race. Frame timings are measured against a two-frame budget, so **100% is 30fps** and anything above that is a dropped frame. Bars turn amber past 75% and red past 100%. The graph down the left edge holds the last 48 frames with a line marking the budget.

| Row   | Shows                                                    |
|-------|----------------------------------------------------------|
| FRAME | total frame time, and % of budget                        |
| LOGIC | game logic time, and % of the frame                      |
| DRAW  | CPU draw time, and % of the frame                        |
| GPU   | GPU time, and % of the frame                             |
| QUADS | visible quadblocks / total in the level                  |
| LEAFS | BSP leaves drawn / total nodes                           |
| PRIM  | primitive buffer used, and primitive count               |
| TRANS | % of primitives that are semi-transparent, and the count |
| TEX   | % of primitives that are textured, and the count         |
| WORST | worst frame in the last 60, in ms and whole vsyncs       |
| PEAK  | lap % and visible quad count at that worst frame         |
| TRACK | BSP node and instance counts                             |
| VERTS | vertex count                                             |
| MEM   | free MEMPACK bytes                                       |

`WORST` and `PEAK` are the useful pair: they tell you *where on the lap* the track is heaviest, which is hard to catch by eye. `QUADS` and `LEAFS` show how much the visibility tree is actually culling.

## Freecam

Press `L3` + `R3` during a race. Every kart is halted and all quadblocks are forced visible so you can fly outside the normal view and still see the level. `L2` cycles the fly speed, `R2` hides the control help.

Because the visibility tree is bypassed, what you see in freecam is not what the renderer would normally draw — use the debug HUD's `QUADS` row for that, not freecam.

Toggling it off restores the karts and the normal camera.

## Reserve bar

Sits directly under the powerslide meter and matches its size. It shows the reserves the player has accumulated: each filled bar is five seconds of fire, and the number to its left counts the whole levels banked on top of that. The colour steps through red, orange, yellow, green, cyan, blue and purple as levels climb; grey is empty and magenta means Saffi fire.

It is only drawn while the speedometer is hidden, so **triangle toggles it along with the minimap**.

## Interface to CrashTeamEditor

The hot reload protocol is unchanged from the `PS1_TrackROM`, so the editor needs no changes for track reloading:

| Address      | Purpose                  |
|--------------|--------------------------|
| `0x8000C000` | hot reload handshake     |
| `0x8000C004` | VRAM-only reload trigger |
| `0x8000C008` | ghost ready flag         |
| `0x80200000` | VRM staging buffer       |
| `0x80300000` | LEV staging buffer       |

### Pushing settings

Write `HostSettings` at `0x8000C080`, little-endian, 4 bytes per field:

| Offset | Field           |                                             |
|--------|-----------------|---------------------------------------------|
| `0x00` | `magic`         | `0x53544553` (`SETS`)                       |
| `0x04` | `sequence`      | change this on every push                   |
| `0x08` | `relicSapphire` | milliseconds                                |
| `0x0C` | `relicGold`     | milliseconds                                |
| `0x10` | `relicPlatinum` | milliseconds                                |
| `0x14` | `crystalTime`   | milliseconds                                |
| `0x18` | `introCutscene` | 1 plays the race intro camera, 0 skips it   |
| `0x1C` | `ghost`         | 1 leaves the ghost replay alone, 0 stops it |

Milliseconds, so 1 second is `1000` and 1:17.0 is `77000`. Valid range is 500 to 599500 and anything outside it is clamped. Values that are not a multiple of 500 work, they just will not line up with the panel's half-second steps.

**Write the payload first and `sequence` last.** The ROM latches on `sequence` changing, so writing it early lets a half-finished block be read.

Two things to get right:

- **`sequence` must differ from the previous push, including across editor restarts.** The ROM only compares for inequality, so wrap-around and resets are fine — but a counter starting at 0 on every launch will have its first push ignored if the ROM last saw 0. Seed it from a timestamp or persist it.
- **Send the fields as signed 32-bit.** A value with the top bit set reads as negative and clamps to the *minimum*, which looks like "the push never arrived" rather than an obvious error.

A push overrides whatever is in the panel. Panel edits made afterwards stick until the next push, and if the panel is open when a push arrives it updates to show the new values. Relic times are written into the level's slot as the race loads, which includes hot-reloaded tracks.

## Track requirements per mode

The default levels already satisfy all of this. It matters for tracks you author yourself.

- **Arcade / CTR Challenge** — checkpoints and paths, or the bots have nothing to follow. CTR Challenge additionally needs `STATIC_C`, `STATIC_T` and `STATIC_R` instances placed, and you must finish 1st *and* collect all three.
- **Relic Race** — optional time crates (`STATIC_TIME_CRATE_*`).
- **Crystal Challenge** — `STATIC_CRYSTAL` instances.

## Source layout

    src/rom.h              memory map, editor protocol, bigfile slots
    src/rom/               this ROM's own code, compiled into the EXE's rdata_free
      boot.{c,h}             one-shot power-on setup
      drivers.{c,h}          preloaded racer models, model lookup, MPK per mode
      game_mode.{c,h}        mode selection and the mode's effect on a level load
      hot_reload.{c,h}       handshake with CrashTeamEditor
    src/dll/               DLL.BIN, loaded into high RAM at boot
      race.{c,h}             per-frame driver for the in-race tools
      freecam.{c,h}          free camera
      debug.{c,h}            frame timing and scene counters
      reserves.{c,h}         reserve bar
      settings.{c,h}         settings panel and the editor push
      oxide.{c,h}            model scaling, 16-entry roster
      input.{c,h}            fixed button bindings
      prim.{c,h}             primitive allocation and boxes
      math.{c,h}             shared clamp / wrap / scale helpers
    src/hooks/exe/         in-place overwrites of EXE functions
    src/hooks/ovl230/      main menu, track select, character search, Oxide model
    src/hooks/ovl224/      time trial end-of-race menu data

## Notes

This repository is intended to be used in conjunction with the CTR-ModSDK. I did not fork that repository because it includes thousands of unrelated commits and large chunks of unrelated source code. On its own, this repository is useless as it needs the ModSDK to run.