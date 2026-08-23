# Customization

Everything this ROM lets you change lives in one struct. There are three places it can come from:

| Layer            | Where                                         | Survives a reboot |
|------------------|-----------------------------------------------|-------------------|
| Built-in default | `CONFIG_DEFAULTS` in `src/config_default.h`   | yes               |
| Baked config     | `CONFIG.BIN` on the disc                      | yes               |
| Live edit        | the in-game settings panel, or an editor push | no                |

`CONFIG_DEFAULTS` feeds both `CONFIG.BIN` and the in-ROM fallback, so the two can never disagree. If `CONFIG.BIN` is missing or unreadable the ROM falls back to those defaults and says so in red on the main menu.

Live edits apply immediately — changing a mode toggle rebuilds the menu, changing the ghost setting re-patches the code — but they are not written back to disc. To ship settings you edit `src/config_default.h` and rebuild.

An editor push carries the same complete `Config`, so everything on this page can be set live as well as baked — see [CrashTeamEditor interface](./cte-interface.md).

## What can be configured

The settings panel shows one row per field. It scrolls; twelve rows are visible at a time.

### Times and race setup

| Field                 | Range                        | Default |
|-----------------------|------------------------------|---------|
| Relic Time - Sapphire | 0.5s – 9:59.5                | 1:17.0  |
| Relic Time - Gold     | 0.5s – 9:59.5                | 1:05.0  |
| Relic Time - Platinum | 0.5s – 9:59.5                | 0:52.0  |
| Crystal Time Limit    | 0.5s – 9:59.5                | 2:00.0  |
| Lap Count             | 1, 3, 5, 7                   | 3       |
| Intro Cutscene        | On / Off                     | On      |
| Time Trial Ghosts     | On / Off                     | On      |
| CTR Token Color       | Red/Green/Blue/Yellow/Purple | Yellow  |

The three relic times are the targets for Relic Race; the defaults are Crash Cove's real times, so a Relic Race on the default level behaves like retail until you change something. The crystal limit is the timer for Crystal Race, which normally comes from the arena's own stored time.

**Lap Count** only offers 1, 3, 5 and 7 because those are the only counts vanilla CTR handles — the lap-time table holds seven entries per driver and the end-of-race box only has heights for those four.

**CTR Token Color** exists because a custom track uses Dingo Canyon's level slot and inherits its metadata, which is where the token takes its color from. Without this the token is always yellow.

**Intro Cutscene** off skips the race intro camera fly-in, which saves a few seconds on every retry.

**Time Trial Ghosts** off stops the N. Tropy / Oxide ghost replay from running. Both are applied by patching instructions, so they take effect on the next race.

### Features

| Field                | Effect when off                                      |
|----------------------|------------------------------------------------------|
| Freecam              | L3+R3 does nothing                                   |
| Debug HUD            | SELECT does nothing                                  |
| Reserves Display     | the reserve bar is not drawn                         |
| Hot Reload           | the ROM stops listening for editor track pushes      |
| Host Settings        | the ROM stops listening for editor settings pushes   |
| Updated Engine Stats | characters keep their vanilla speed and acceleration |

Hot Reload and Host Settings are the only two that are editor-side; a shipped ROM has no reason to keep polling for them. See [Features](./features.md) for what Updated Engine Stats actually changes.

### Modes and bosses

One toggle per main-menu mode (Arcade, Relic Race, Time Trial, Crystal Race, CTR Token, Boss Race) and one per boss (Ripper Roo, Papu Papu, Komodo Joe, Pinstripe, N. Oxide).

Turning a mode off removes its row; the remaining rows renumber and the menu re-links itself. Turning a boss off removes it from the boss picker.

Turning *all* modes or *all* bosses off is normalised back to "everything" rather than leaving an empty menu, so you cannot lock yourself out.

## Hiding the settings panel

The `editable` mask has one bit per field. A field is shown only if its bit is set. Set the mask to `0` and the SETTINGS row disappears from the main menu entirely, which is what a shipped ROM usually wants.

```c
.editable = 0,                                   // no settings panel at all
.editable = OPTION_RELIC_SAPPHIRE
          | OPTION_RELIC_GOLD
          | OPTION_RELIC_PLATINUM,             // relic times only
.editable = OPTION_ALL,                        // everything (the default)
```

The bit names are in `src/config_schema.h` and are asserted against the field order at compile time, so they cannot drift apart silently.

## Shipping a ROM with your own track

The ModSDK is required, since you rebuild the ROM to bake anything in.

**1. Drop your files in `src/assets/`** as `customtrack.lev` and `customtrack.vrm`.

**2. Uncomment the two lines at the top of `buildList.txt`:**

```
common, bigfilelevelstracksproto81Pdatavrm, 0x0, 0x0, src/assets/customtrack.vrm
common, bigfilelevelstracksproto81Pdatalev, 0x0, 0x0, src/assets/customtrack.lev
```

These write your files over Dingo Canyon's bigfile entries. The bigfile is repacked afterwards, so your files may be larger than the originals.

**3. Set `BAKED_TRACK` to `1` in `src/rom.h`.** Without it the ROM keeps falling back to Crash Cove no matter what is on the disc.

**4. Edit `CONFIG_DEFAULTS` in `src/config_default.h`** for the modes, times and features you want to ship.

**5. Rebuild, then xdelta the resulting ISO** against a clean one.

An editor push still overrides a baked track while the editor is attached, so you can keep iterating on a ROM that already ships one.

### Size limits

| File | Limit                                                 |
|------|-------------------------------------------------------|
| lev  | `CUSTOM_LEV_MAX_SIZE`, currently **3,209,216 bytes**  |
| vrm  | exactly `VRM_FILESIZE`, **458,808 bytes** (`0x70038`) |

## CONFIG.BIN for tooling

`CONFIG.BIN` is its own file on the disc, fixed at 256 bytes, so a tool can rewrite it inside a finished ISO without touching the EXE, the DLL or the bigfile. `src/config_schema.h` is plain C with no engine dependencies and can be included directly from C or C++.