# Customization

Everything this ROM lets you change lives in one struct. There are three places it can come from:

| Layer            | Where                                         | Survives a reboot |
|------------------|-----------------------------------------------|-------------------|
| Built-in default | `src/config/config_default.h`                 | yes               |
| Your override    | `src/user_config.h`, which git ignores        | yes               |
| Live edit        | the in-game settings panel, or an editor push | no                |

Your overrides feed `CONFIG_DEFAULTS`, which is compiled into the ROM. There is no settings file on the disc — what you build with is what ships.

Live edits apply immediately — changing a mode toggle rebuilds the menu, changing the ghost setting re-patches the code — but they are not written back to disc. To ship settings you write them to `src/user_config.h` and rebuild.

An editor push carries the same complete `Config`, so everything on this page can be set live as well as baked — see [CrashTeamEditor interface](./cte-interface.md).

## What can be configured

The settings panel shows one row per field. It scrolls; twelve rows are visible at a time. The rows are grouped: the mode toggles first, then the settings belonging to each mode in the same order, then presentation and the authoring tools.

### Modes

One toggle per main-menu mode.

| Panel row           | Name to use                     |
|---------------------|---------------------------------|
| Mode - Arcade       | `CONFIG_MODE_ARCADE`            |
| Mode - Relic Race   | `CONFIG_MODE_RELIC_RACE`        |
| Mode - Time Trial   | `CONFIG_MODE_TIME_TRIAL`        |
| Mode - Crystal Race | `CONFIG_MODE_CRYSTAL_CHALLENGE` |
| Mode - CTR Token    | `CONFIG_MODE_CTR_TOKEN`         |
| Mode - Boss Race    | `CONFIG_MODE_BOSS_RACE`         |

Turning a mode off removes its row; the remaining rows renumber and the menu re-links itself. Turning *all* modes off is normalised back to "everything" rather than leaving an empty menu, so you cannot lock yourself out.

Turning a mode off does **not** hide the rows below that configure it — a track can ship with Relic Race off and still carry sensible relic times for whoever turns it back on.

### Every race

| Field     | Range      | Default |
|-----------|------------|---------|
| Lap Count | 1, 3, 5, 7 | 3       |

**Lap Count** only offers 1, 3, 5 and 7 because those are the only counts vanilla CTR handles — the lap-time table holds seven entries per driver and the end-of-race box only has heights for those four.

### Relic Race

| Field                 | Range         | Default |
|-----------------------|---------------|---------|
| Relic Time - Sapphire | 0.5s – 9:59.5 | 1:17.0  |
| Relic Time - Gold     | 0.5s – 9:59.5 | 1:05.0  |
| Relic Time - Platinum | 0.5s – 9:59.5 | 0:52.0  |

The three are the targets for each relic tier. The defaults are Crash Cove's real times, so a Relic Race on the default level behaves like retail until you change something.

### Crystal Race

| Field              | Range         | Default |
|--------------------|---------------|---------|
| Crystal Time Limit | 0.5s – 9:59.5 | 2:00.0  |

The timer for Crystal Race, which normally comes from the arena's own stored time.

### CTR Token

| Field           | Range                           | Default |
|-----------------|---------------------------------|---------|
| CTR Token Color | [5 colours](#ctr-token-colours) | Yellow  |

**CTR Token Color** exists because a custom track takes over a vanilla level slot — Dingo Canyon by default — and inherits its metadata, which is where the token takes its color from. Without this the token is always yellow.

### Time Trial

| Field             | Range                  | Default  |
|-------------------|------------------------|----------|
| Time Trial Ghosts | On / Off               | On       |
| Ghost 1 Time      | 0.5s – 9:59.5          | 1:55.0   |
| Ghost 1 Character | any [driver](#drivers) | N. Tropy |
| Ghost 2 Time      | 0.5s – 9:59.5          | 1:45.0   |
| Ghost 2 Character | any [driver](#drivers) | N. Oxide |

* **Time Trial Ghosts** off stops the ghost replay from running. It is applied by patching an instruction, so a change takes effect on the next race.
* **Ghost 1 Time** and **Ghost 2 Time** are the targets: beat the first and the first ghost unlocks, beat the second and so does the second. Vanilla calls these the N. Tropy and Oxide ghosts, but nothing about them is fixed to those two characters, hence the neutral names. The defaults are Dingo Canyon's real N. Tropy time and a plausible Oxide time, since that is the level slot a custom track takes over.
* **Ghost 1 Character** and **Ghost 2 Character** pick who is driving. Any of the sixteen [drivers](#drivers) works in either slot, including the same one twice — the ROM keeps every driver model resident and loads the shared texture set at boot, so the lookup never misses. The defaults reproduce vanilla.
* The times and the characters only do anything if your LEV actually carries the matching ghost recording. A track with no ghosts in it keeps them locked no matter what you set here — see [Time trial ghosts](./features.md#time-trial-ghosts).

### Boss Race

Each boss slot gets its own block of three rows:

| Field                | Range                                    | Default          |
|----------------------|------------------------------------------|------------------|
| Boss `<n>` Enabled      | On / Off                                 | On               |
| Boss `<n>` Character    | any [driver](#drivers)                   | the vanilla boss |
| Boss `<n>` Item Preset  | any [preset](#boss-items)                | Vanilla          |

Turning a boss off removes it from the boss picker. Turning *all* bosses off is normalised back to "everything", the same as the modes.

### Boss slots

The five bosses are slots, `BOSS_1` to `BOSS_5`. Nothing about a slot is tied to a particular character: the settings panel groups its rows per slot - `Boss <n> Enabled`, `Boss <n> Character`, `Boss <n> Item Preset` - and who actually drives each one is the `Boss <n> Character` row, picking any of the 16 drivers. The boss's model comes from that character's time trial pack, so there is nothing special about the vanilla five.

Set the starting driver for a slot with `CFG_BOSS_CHARACTER_<n>`:

```c
#define CFG_BOSS_CHARACTER_1 TINY_TIGER
#define CFG_BOSS_CHARACTER_5 N_GIN
```

The same sixteen names work here and for the ghost characters — see [Drivers](#drivers). Leave `CFG_BOSS_CHARACTER_<n>` alone and the slot starts on its vanilla boss: Ripper Roo, Papu Papu, Komodo Joe, Pinstripe and N. Oxide, in slot order.

The name shown in the boss picker is separate, and lives in `src/config/bosses.h`. Define `BOSS_LIST` in your `src/user_config.h` and yours replaces it wholesale:

```c
#define BOSS_LIST(X) \
	X(1, "Jak",      BossWeaponRoo)       \
	X(2, "Sonic",    BossWeaponPapu)      \
	X(3, "Link",     BossWeaponJoe)       \
	X(4, "Greymon",  BossWeaponPinstripe) \
	X(5, "Kong",     BossWeaponOxide)
```

Each entry is `X(slot, name, vanilla item table)`. The last column is whose vanilla throwing table the boss inherits, which sets his item pacing before any preset is applied. Keep it as it is unless you also want his rhythm changed; the preset is the intended way to change what he throws.

You need exactly `CONFIG_BOSS_COUNT` entries — a shorter or longer list is a compile error, not a silent truncation.

### Boss items

One row per boss, `Boss <n> Item Preset`, picking a named preset. A preset is a set of items plus a juice setting. The shipped ones, from `src/config/boss_item_presets.h`:

| Name to use | Shown in panel | Items | Juice |
|-------------|----------------|-------------------------------------------|---------|
| `BOSS_ITEM_PRESET_VANILLA`   | Vanilla   | whatever the game gives that boss      | vanilla |
| `BOSS_ITEM_PRESET_TNT`       | TNT       | TNT                                    | vanilla |
| `BOSS_ITEM_PRESET_BOMBS`     | Bombs     | bombs                                  | vanilla |
| `BOSS_ITEM_PRESET_BEAKERS`   | Beakers   | beakers                                | vanilla |
| `BOSS_ITEM_PRESET_MISSILES`  | Missiles  | missiles                               | never   |
| `BOSS_ITEM_PRESET_CLASSIC`   | Classic   | TNT, beakers, bombs                    | random  |
| `BOSS_ITEM_PRESET_RUTHLESS`  | Ruthless  | missiles, warp orbs, clocks, invisibility | always  |
| `BOSS_ITEM_PRESET_DEFENSIVE` | Defensive | TNT, beakers, masks, shields           | always  |
| `BOSS_ITEM_PRESET_SPEEDY`    | Speedy    | turbos, super engines, masks           | always  |

A boss with several items in his preset cycles through them across the race.

#### Defining your own presets

The list lives in `src/config/boss_item_presets.h`, but define `BOSS_ITEM_PRESET_LIST` in your `src/user_config.h` and yours replaces it wholesale:

```c
#define BOSS_ITEM_PRESET_LIST(X) 	X(VANILLA, "Vanilla",  BOSS_ITEMS_VANILLA,                     BOSS_JUICE_VANILLA) 	X(NASTY,   "Nasty",    BOSS_ITEM_MISSILE | BOSS_ITEM_WARP_ORB, BOSS_JUICE_NONE)

#define CFG_BOSS_ITEM_PRESET_1 BOSS_ITEM_PRESET_NASTY
```

Each entry is `X(id, name, items, juice)`. The id becomes `BOSS_ITEM_PRESET_<id>` so you can name it in a `CFG_BOSS_ITEM_PRESET_*` line; the name is what the settings panel shows. Keep a sensible entry first — it is what an unconfigured boss falls back to.

Items are OR-ed together. The full set, from `src/config/config_schema.h`:

| Name                     | Item        |
|--------------------------|-------------|
| `BOSS_ITEM_TURBO`        | turbo       |
| `BOSS_ITEM_BOMB`         | bomb        |
| `BOSS_ITEM_MISSILE`      | missile     |
| `BOSS_ITEM_TNT`          | TNT         |
| `BOSS_ITEM_BEAKER`       | beaker      |
| `BOSS_ITEM_SHIELD`       | shield      |
| `BOSS_ITEM_MASK`         | Aku Aku     |
| `BOSS_ITEM_CLOCK`        | clock       |
| `BOSS_ITEM_WARP_ORB`     | warp orb    |
| `BOSS_ITEM_INVISIBILITY` | invisibility |
| `BOSS_ITEM_SUPER_ENGINE` | super engine |
| `BOSS_ITEMS_VANILLA`     | none of the above — leave the boss's vanilla loadout alone |

Juice is one of:

| Name                 | Effect                                    |
|----------------------|-------------------------------------------|
| `BOSS_JUICE_VANILLA` | leave the vanilla table's juicing as it is |
| `BOSS_JUICE_NONE`    | never juiced                              |
| `BOSS_JUICE_ALWAYS`  | always juiced                             |
| `BOSS_JUICE_RANDOM`  | juiced at random                          |

Where along the track the boss throws is not configurable and does not need to be. The ROM rescales the vanilla checkpoint pattern onto your track's restart points, and sections the vanilla table leaves unarmed stay unarmed, so changing his items keeps his pacing.

### Presentation

| Field              | Range    | Default |
|--------------------|----------|---------|
| Intro Cutscene     | On / Off | On      |
| High Detail Tracks | On / Off | On      |

* **Intro Cutscene** off skips the race intro camera fly-in, which saves a few seconds on every retry. Like the ghost toggle it is applied by patching an instruction, so it takes effect on the next race.
* **High Detail Tracks** pins every quadblock to its highest detail level instead of letting the renderer drop detail with distance. On is what you want while authoring — it is how the track was designed to look. Turn it off to see what the engine would normally draw and what that costs; the `QUADS` and `FRAME` rows of the [debug HUD](./features.md#debug-hud) are the ones to watch. It re-applies at the start of each race, so a change takes effect on the next load.

### Features

| Panel row            | Name to use             | Effect when off                                      |
|----------------------|-------------------------|------------------------------------------------------|
| Freecam              | `FEATURE_FREECAM`       | L3+R3 does nothing                                   |
| Debug HUD            | `FEATURE_DEBUG_HUD`     | SELECT does nothing                                  |
| Reserves Display     | `FEATURE_RESERVES`      | the reserve bar is not drawn                         |
| Hot Reload           | `FEATURE_HOT_RELOAD`    | the ROM stops listening for editor track pushes      |
| Host Settings        | `FEATURE_HOST_SETTINGS` | the ROM stops listening for editor settings pushes   |
| Updated Engine Stats | `FEATURE_MAX_STATS`     | characters keep their vanilla speed and acceleration |

These sit last because they are authoring tools rather than track rules. Hot Reload and Host Settings are the only two that are editor-side; a shipped ROM has no reason to keep polling for them. See [Features](./features.md) for what Updated Engine Stats actually changes.

## Editing the config

Everything lands in `src/user_config.h`. That file is gitignored, so it is yours alone and never conflicts when you pull. **Do not edit `src/config/config_default.h`** — it is tracked, it gains fields over time, and editing it means a conflict on every update.

Copy `src/user_config.example.h` to `src/user_config.h` and edit that. Define only what you want to change; everything you leave out — including fields added in later versions — keeps following the shipped defaults.

The settings panel is for trying values out, not for saving them: its edits last until the ROM restarts. Once you like a value, write it here and rebuild.

```c
#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#define BAKED_TRACK  1
#define CFG_MODES    (CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE)
#define CFG_LAPS     5
#define CFG_EDITABLE 0

#endif
```

That is a complete, valid override file. Three things to know:

- **The names are `CFG_<field>`**, matching the fields listed above: `CFG_LAPS`, `CFG_MODES`, `CFG_CTR_TOKEN`, `CFG_RELIC_GOLD`, and so on. `src/config/config_default.h` lists all of them, one `#ifndef` block each — read it, don't edit it.
- **`|` means "and also".** `CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE` is "time trial and relic race". Anything you leave out is off.
- **Times are in milliseconds.** 1 second is `1000`, so `77000` is 1:17.0.

`BAKED_TRACK` and `CUSTOM_LEVEL_ID` go in the same file even though they live in `rom.h` rather than the config struct.

The value names you can use, and where each set is defined:

| Setting                       | Names to use         | Defined in                        |
|-------------------------------|----------------------|-----------------------------------|
| `CFG_FEATURES`                | `FEATURE_*`          | `src/config/config_schema.h`      |
| `CFG_MODES`                   | `CONFIG_MODE_*`      | `src/config/config_schema.h`      |
| `CFG_BOSSES`                  | `CONFIG_BOSS_*`      | `src/config/config_schema.h`      |
| `CFG_EDITABLE`                | `OPTION_*`           | `src/config/config_schema.h`      |
| `CFG_CTR_TOKEN`               | `TOKEN_*`            | `src/config/config_schema.h`      |
| `CFG_GHOST_CHARACTER_1/2`     | a driver name or `0`–`15` | `enum Characters`, `namespace_Vehicle.h` |
| `CFG_BOSS_CHARACTER_<slot>`   | a driver name or `0`–`15` | `enum Characters`, `namespace_Vehicle.h` |
| `CFG_BOSS_ITEM_PRESET_<boss>` | `BOSS_ITEM_PRESET_*` | `src/config/boss_item_presets.h`  |

`CONFIG_MODE_ALL`, `CONFIG_BOSS_ALL` and `OPTION_ALL` are shorthand for "every one of them". `CFG_EDITABLE` is one bit per settings row and there are more than 32 rows, so it is 64 bits wide — that changes nothing about how you write it.

Every `CFG_*` name and its shipped default is in `src/config/config_default.h`, one `#ifndef` block each. Read it to see what exists; don't edit it.

## Value reference

### Drivers

Any of the sixteen works in any driver slot, including the same one twice. The names come from `enum Characters` in the ModSDK's `namespace_Vehicle.h`; the panel column is what the settings screen shows, from `characterNames[]` in `src/dll/settings.c`.

| #  | Name to use       | Shown in panel | #  | Name to use     | Shown in panel |
|----|-------------------|----------------|----|-----------------|----------------|
| 0  | `CRASH_BANDICOOT` | Crash          | 8  | `PINSTRIPE`     | Pinstripe      |
| 1  | `NEO_CORTEX`      | Cortex         | 9  | `PAPU_PAPU`     | Papu Papu      |
| 2  | `TINY_TIGER`      | Tiny           | 10 | `RIPPER_ROO`    | Ripper Roo     |
| 3  | `COCO_BANDICOOT`  | Coco           | 11 | `KOMODO_JOE`    | Komodo Joe     |
| 4  | `N_GIN`           | N. Gin         | 12 | `N_TROPY`       | N. Tropy       |
| 5  | `DINGODILE`       | Dingodile      | 13 | `PENTA_PENGUIN` | Penta          |
| 6  | `POLAR`           | Polar          | 14 | `FAKE_CRASH`    | Fake Crash     |
| 7  | `PURA`            | Pura           | 15 | `NITROS_OXIDE`  | N. Oxide       |

### CTR token colours

| Name           | Value |
|----------------|-------|
| `TOKEN_RED`    | 0     |
| `TOKEN_GREEN`  | 1     |
| `TOKEN_BLUE`   | 2     |
| `TOKEN_YELLOW` | 3     |
| `TOKEN_PURPLE` | 4     |

The value is written straight into `ctrTokenGroupID` in the level's `data.metaDataLEV[]` entry, which is the field vanilla uses to pick the token set.

### Mode and boss flags

| Name                            | Menu row          |
|---------------------------------|-------------------|
| `CONFIG_MODE_ARCADE`            | Arcade            |
| `CONFIG_MODE_RELIC_RACE`        | Relic Race        |
| `CONFIG_MODE_TIME_TRIAL`        | Time Trial        |
| `CONFIG_MODE_CRYSTAL_CHALLENGE` | Crystal Race      |
| `CONFIG_MODE_CTR_TOKEN`         | CTR Token         |
| `CONFIG_MODE_BOSS_RACE`         | Boss Race         |

Bosses are `CONFIG_BOSS_1` through `CONFIG_BOSS_5`, one per slot.

### Settings panel rows

`CFG_EDITABLE` takes `OPTION_*` names, one per row. The non-boss ones match the field they control — `OPTION_RELIC_SAPPHIRE`, `OPTION_LAPS`, `OPTION_TOKEN_COLOR`, `OPTION_FREECAM`, `OPTION_MODE_ARCADE`, and so on. Each boss slot has three:

```c
OPTION_BOSS_<n>_ENABLED
OPTION_BOSS_<n>_CHARACTER
OPTION_BOSS_<n>_ITEM_PRESET
```

The full list is in `src/config/config_schema.h`, in panel order. The bit positions matter to the ROM but not to you, and they shift whenever the panel is reordered — always write the names, never a raw number.

## Examples

Each example is the body of your `src/user_config.h`. Leave out anything you do not want to change.

### A time trial track — only two modes on the menu

```c
#define CFG_MODES (CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE)
```

The main menu now has two rows plus SETTINGS. The modes you left out are gone entirely.

### Ship with no settings panel

```c
#define CFG_EDITABLE 0
```

No rows are editable, so the SETTINGS row disappears from the main menu. This is what most released tracks want — the player gets the times and rules you chose.

### Let players change the relic times, nothing else

```c
#define CFG_EDITABLE (OPTION_RELIC_SAPPHIRE | OPTION_RELIC_GOLD | OPTION_RELIC_PLATINUM)
```

SETTINGS appears with exactly three rows in it.

### Only race two of the bosses

```c
#define CFG_MODES  CONFIG_MODE_BOSS_RACE
#define CFG_BOSSES (CONFIG_BOSS_1 | CONFIG_BOSS_5)
```

The boss picker offers Ripper Roo and N. Oxide only.

### Make one boss harder

```c
#define CFG_BOSS_ITEM_PRESET_1 BOSS_ITEM_PRESET_RUTHLESS
#define CFG_BOSS_CHARACTER_1   TINY_TIGER
```

Slot 1 is now driven by Tiny, cycling missiles, warp orbs, clocks and invisibility, all juiced. The other four keep their vanilla drivers and loadouts.

### Strip the development tools out of a release

```c
#define CFG_FEATURES FEATURE_MAX_STATS
#define CFG_EDITABLE 0
```

No freecam, no debug HUD, no reserve bar, and the ROM stops listening for the editor. Keeping `FEATURE_MAX_STATS` leaves the updated engine stats on; drop it too for vanilla handling.

### Your own times and rules

```c
#define CFG_RELIC_SAPPHIRE 90000
#define CFG_RELIC_GOLD     80000
#define CFG_RELIC_PLATINUM 70000
#define CFG_CRYSTAL_TIME   150000
#define CFG_LAPS           5
#define CFG_CTR_TOKEN      TOKEN_RED
#define CFG_INTRO_CUTSCENE 0
```

Relic targets of 1:30 / 1:20 / 1:10, a 2:30 crystal timer, five laps, red tokens, and the intro camera fly-in skipped.

### A finished release, all together

```c
#define BAKED_TRACK        1
#define CFG_FEATURES       FEATURE_MAX_STATS
#define CFG_MODES          (CONFIG_MODE_ARCADE | CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE)
#define CFG_EDITABLE       0
#define CFG_RELIC_SAPPHIRE 90000
#define CFG_RELIC_GOLD     80000
#define CFG_RELIC_PLATINUM 70000
#define CFG_LAPS           3
#define CFG_CTR_TOKEN      TOKEN_BLUE
```

One caveat worth repeating: setting `CFG_MODES` or `CFG_BOSSES` to `0` does **not** hide everything — an empty mask is treated as "all of them" so the ROM can never boot to a menu with no rows. If you want a single mode, name that one mode.

If you get a name wrong the build fails pointing at your file, rather than producing a broken ROM.

## Shipping a ROM with your own track

The ModSDK is required, since you rebuild the ROM to bake anything in.

**1. Drop your files in `src/assets/`** as `customtrack.lev` and `customtrack.vrm`.

**2. Uncomment the two lines at the top of `buildList.txt`:**

```
common, bigfilelevelstracksproto81Pdatavrm, 0x0, 0x0, src/assets/customtrack.vrm
common, bigfilelevelstracksproto81Pdatalev, 0x0, 0x0, src/assets/customtrack.lev
```

These write your files over Dingo Canyon's bigfile entries. The bigfile is repacked afterwards, so your files may be larger than the originals. To take over a different level instead, see [Overriding a different level](#overriding-a-different-level).

**3. Set `BAKED_TRACK` to `1` in `src/rom.h`.** Without it the ROM keeps falling back to Crash Cove no matter what is on the disc.

**4. Put the modes, times and features you want to ship in `src/user_config.h`** and rebuild.

**5. Rebuild, then xdelta the resulting ISO** against a clean one.

An editor push still overrides a baked track while the editor is attached, so you can keep iterating on a ROM that already ships one.

### Size limits

| File | Limit                                                 |
|------|-------------------------------------------------------|
| lev  | `CUSTOM_LEV_MAX_SIZE`, currently **3,145,728 bytes** (`0x300000`) |
| vrm  | exactly `VRM_FILESIZE`, **458,808 bytes** (`0x70038`) |

### Overriding a different level

The custom track takes over a vanilla level slot, `CUSTOM_LEVEL_ID` in `src/rom.h`, which defaults to `0` — Dingo Canyon. Everything else derives from it, so changing that one define moves the whole ROM to another slot.

You would do this because a custom track inherits the slot's metadata, and some of it is not configurable:

| Inherited from the slot   | Effect                          |
|---------------------------|---------------------------------|
| `data.levBank_FX[]`       | which music and FX bank loads   |
| `data.reverbMode[]`       | the reverb preset               |
| `data.ArcadeDifficulty[]` | bot difficulty tuning           |
| `data.metaDataLEV[]`      | token color, N. Tropy time, hub |

The ROM already overrides the two that matter most — the token color comes from your config, and the N. Tropy ghost time can be disabled. The music, ambience and bot tuning are currently the reason to move the slot.

**1. Change the define** in `src/rom.h`:

```c
#define CUSTOM_LEVEL_ID 12   // POLAR_PASS
```

Names are in `namespace_Level.h`; you can write the name instead of the number.

**2. Retarget the bake lines** in `buildList.txt` to that level's bigfile folder. The folder name is not the track name — Polar Pass is `ice1`:

```
common, bigfilelevelstracksice11Pdatavrm, 0x0, 0x0, src/assets/customtrack.vrm
common, bigfilelevelstracksice11Pdatalev, 0x0, 0x0, src/assets/customtrack.lev
```

| ID | Level          | Folder    | ID | Level           | Folder    |
|----|----------------|-----------|----|-----------------|-----------|
| 0  | Dingo Canyon   | `proto8`  | 9  | Mystery Caves   | `cave1`   |
| 1  | Dragon Mines   | `proto9`  | 10 | Cortex Castle   | `castle1` |
| 2  | Blizzard Bluff | `desert2` | 11 | N. Gin Labs     | `labs1`   |
| 3  | Crash Cove     | `island1` | 12 | Polar Pass      | `ice1`    |
| 4  | Tiger Temple   | `temple1` | 13 | Oxide Station   | `space`   |
| 5  | Papu Pyramid   | `temple2` | 14 | Coco Park       | `coco1`   |
| 6  | Roo's Tubes    | `tube1`   | 15 | Tiny Arena      | `arena2`  |
| 7  | Hot Air Skyway | `blimp1`  | 16 | Slide Coliseum  | `secret1` |
| 8  | Sewer Speedway | `sewer1`  | 17 | Turbo Track     | `secret2` |

Two rules constrain the choice:

- **Stay in 0-17.** Above that are the battle arenas and the adventure hubs, which load a different set of threads, have no checkpoints and no relic support. The engine keys several things on the ID being a race track, including a per-level array with exactly eighteen entries.
- **Not Crash Cove (3) or Skull Rock (21).** Those are the fallback levels the ROM loads when no custom track is present. The custom level is intercepted before the normal disc read, so a collision would swallow the fallback.
