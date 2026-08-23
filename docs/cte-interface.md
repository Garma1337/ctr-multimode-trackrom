# CrashTeamEditor interface

Everything the editor talks to lives in the low scratch page and in two staging buffers. The hot reload protocol is unchanged from the `PS1_TrackROM`, so the editor needs no changes for track reloading.

| Address      | Purpose                  |
|--------------|--------------------------|
| `0x8000C000` | hot reload handshake     |
| `0x8000C004` | VRAM-only reload trigger |
| `0x8000C008` | ghost ready flag         |
| `0x8000C080` | `HostSettings` push      |
| `0x80200000` | VRM staging buffer       |
| `0x802F0800` | LEV staging buffer       |

The LEV buffer is `CUSTOM_LEV_MAP_LOCATION` in `src/rom.h`, and it moves when the memory map changes — read it from the header rather than hardcoding it. It begins with the 4-byte pointer-map offset, with the level data immediately after.

Both channels can be switched off in a shipped ROM: the **Hot Reload** and **Host Settings** feature flags stop the ROM polling for them. See [Customization](customization.md).

## Pushing a track

Once a track has been pushed, every mode runs on it instead of the built-in fallback level, until the ROM is restarted. A pushed track also takes priority over one baked into the ROM, so a shipped ROM is still iterable.

## Pushing settings

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

`HostSettings` covers six of the ROM's settings. The rest — modes, bosses, features, lap count, token color — are not pushable and are set in the config; see [Customization](./customization.md).

## Ghost export

`SAVE GHOST` on the time trial results screen writes the ghost to the staging buffer and raises the ready flag at `0x8000C008` instead of going to a memory card.
