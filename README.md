# CTR Multi-Gamemode ROM

## Introduction

A track-testing ROM for CrashTeamEditor. Same idea as the `PS1_TrackROM`, but every single-player game mode is selectable from the main menu:

| Mode          | Flags it sets                                                 |
|---------------|---------------------------------------------------------------|
| ARCADE        | `ARCADE_MODE` + `arcadeDifficulty` (vanilla difficulty box)   |
| RELIC RACE    | `RELIC_RACE`                                                  |
| TIME TRIAL    | `TIME_TRIAL`                                                  |
| CRYSTAL RACE  | `CRYSTAL_CHALLENGE` + `originalEventTime`                     |
| CTR CHALLENGE | `ADVENTURE_MODE` **and** `TOKEN_RACE`                         |
| BOSS RACE     | `ADVENTURE_MODE` **and** `ADVENTURE_BOSS`, then a boss picker |
| SETTINGS      | opens the settings panel; sets no flags                       |

Any mode can be hidden, and so can the settings row — see [Customization](./docs/customization.md).

Pick a mode, pick a character, and you land on the level that mode runs on. On a fresh boot that is an original level, so the modes are testable on their own:

| Mode                                                      | Level      |
|-----------------------------------------------------------|------------|
| Arcade, Relic Race, Time Trial, CTR Challenge, Boss Race  | Crash Cove |
| Crystal Race                                              | Skull Rock |

**Once CrashTeamEditor sends a track, every mode runs on that instead**, and stays there until the ROM is restarted. A track can also be baked into the ROM so it is used with no editor attached.

Oxide is playable and selectable in the character select. The ROM requires an 8 MB RAM environment with shared memory. `SAVE GHOST` on the time trial results screen exports the ghost to the editor rather than to a memory card.

## Documentation

| Document                                           | Covers                                                          |
|----------------------------------------------------|-----------------------------------------------------------------|
| [Keymap](./docs/keymap.md)                         | every binding                                                   |
| [Features](./docs/features.md)                     | freecam, reserve bar, updated engine stats                      |
| [Customization](./docs/customization.md)           | every setting, how to ship a ROM with your own track and config |
| [CrashTeamEditor interface](docs/cte-interface.md) | hot reload, the settings push, ghost export                     |

## Track requirements per mode

The default levels already satisfy all of this. It matters for tracks you author yourself.

- **Arcade / CTR Challenge / Boss Race** — checkpoints and paths, or the bots have nothing to follow. CTR Challenge additionally needs `STATIC_C`, `STATIC_T` and `STATIC_R` instances placed, and you must finish 1st *and* collect all three.
- **Relic Race** — optional time crates (`STATIC_TIME_CRATE_*`).
- **Crystal Race** — `STATIC_CRYSTAL` instances.

## Source layout

    src/assets/        customtrack.lev / customtrack.vrm when baking a track
    src/config/        the default config, compiled into CONFIG.BIN
    src/rom/           appended to the EXE's rdata_free
    src/dll/           DLL.BIN, loaded into high RAM at boot
    src/hooks/exe/     in-place overwrites of EXE functions
    src/hooks/ovl230/  main menu, track select, character search, Oxide model
    src/hooks/ovl224/  time trial end-of-race menu data
    src/patches/       raw instruction patches
    docs/              this documentation

## Notes

This repository is intended to be used in conjunction with the CTR-ModSDK. I did not fork that repository because it includes thousands of unrelated commits and large chunks of unrelated source code. On its own, this repository is useless as it needs the ModSDK to run.
