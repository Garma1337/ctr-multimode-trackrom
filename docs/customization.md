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

## Editing the config

Everything lives in one block in `src/config_default.h`. It looks like this:

```c
#define CONFIG_DEFAULTS \
{ \
	.magic = CONFIG_MAGIC, \
	.version = CONFIG_VERSION, \
	.size = sizeof(Config), \
	.features = FEATURE_FREECAM | FEATURE_DEBUG_HUD | FEATURE_RESERVES | \
	            FEATURE_HOT_RELOAD | FEATURE_HOST_SETTINGS | \
	            FEATURE_MAX_STATS, \
	.modes = CONFIG_MODE_ALL, \
	.editable = OPTION_ALL, \
	.relicSapphire = 77000, \
	.relicGold = 65000, \
	.relicPlatinum = 52000, \
	.crystalTime = 120000, \
	.laps = 3, \
	.introCutscene = 1, \
	.ghosts = 1, \
	.bosses = CONFIG_BOSS_ALL, \
	.ctrToken = TOKEN_YELLOW, \
}
```

Four things to know before editing it:

- **Every line ends with a `\`.** The whole block is one long definition and the backslash joins the lines together. If you add or move a line, make sure it still ends with one — a missing backslash is the most common way to break this file. The closing `}` is the only line without one.
- **`.name = value,`** — keep the leading dot and the trailing comma.
- **`|` means "and also".** `FEATURE_FREECAM | FEATURE_DEBUG_HUD` is "freecam and the debug HUD". Anything you leave out is off.
- **Times are in milliseconds.** 1 second is `1000`, so `77000` is 1:17.0.

The names you can use are listed in `src/config_schema.h`:

| Setting     | Names to use    | Meaning                                 |
|-------------|-----------------|-----------------------------------------|
| `.features` | `FEATURE_*`     | which tools and behaviours are enabled  |
| `.modes`    | `CONFIG_MODE_*` | which rows appear on the main menu      |
| `.bosses`   | `CONFIG_BOSS_*` | which bosses the picker offers          |
| `.editable` | `OPTION_*`      | which rows appear in the settings panel |
| `.ctrToken` | `TOKEN_*`       | the CTR token colour                    |

`CONFIG_MODE_ALL`, `CONFIG_BOSS_ALL` and `OPTION_ALL` are shorthand for "every one of them".

## Examples

Each example shows only the lines you change; leave the rest of the block alone.

### A time trial track — only two modes on the menu

```c
	.modes = CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE, \
```

The main menu now has two rows plus SETTINGS. The modes you left out are gone entirely.

### Ship with no settings panel

```c
	.editable = 0, \
```

`0` means no rows are editable, so the SETTINGS row disappears from the main menu. This is what most released tracks want — the player gets the times and rules you chose.

### Let players change the relic times, nothing else

```c
	.editable = OPTION_RELIC_SAPPHIRE | OPTION_RELIC_GOLD | OPTION_RELIC_PLATINUM, \
```

SETTINGS appears with exactly three rows in it.

### Only race two of the bosses

```c
	.modes = CONFIG_MODE_BOSS_RACE, \
	.bosses = CONFIG_BOSS_RIPPER_ROO | CONFIG_BOSS_NITROS_OXIDE, \
```

The boss picker offers Ripper Roo and N. Oxide only.

### Strip the development tools out of a release

```c
	.features = FEATURE_MAX_STATS, \
	.editable = 0, \
```

No freecam, no debug HUD, no reserve bar, and the ROM stops listening for the editor. Keeping `FEATURE_MAX_STATS` leaves the updated engine stats on; drop it too for vanilla handling.

### Your own times and rules

```c
	.relicSapphire = 90000, \
	.relicGold = 80000, \
	.relicPlatinum = 70000, \
	.crystalTime = 150000, \
	.laps = 5, \
	.ctrToken = TOKEN_RED, \
	.introCutscene = 0, \
```

Relic targets of 1:30 / 1:20 / 1:10, a 2:30 crystal timer, five laps, red tokens, and the intro camera fly-in skipped.

### A finished release, all together

```c
	.features = FEATURE_MAX_STATS, \
	.modes = CONFIG_MODE_ARCADE | CONFIG_MODE_TIME_TRIAL | CONFIG_MODE_RELIC_RACE, \
	.editable = 0, \
	.relicSapphire = 90000, \
	.relicGold = 80000, \
	.relicPlatinum = 70000, \
	.laps = 3, \
	.ctrToken = TOKEN_BLUE, \
```

One caveat worth repeating: setting `.modes` or `.bosses` to `0` does **not** hide everything — an empty mask is treated as "all of them" so the ROM can never boot to a menu with no rows. If you want a single mode, name that one mode.

If you get it wrong, the build will fail with an error pointing at `config_default.h` rather than producing a broken ROM. A misspelled name or a missing backslash is caught at compile time.

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