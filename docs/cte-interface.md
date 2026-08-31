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

Write `HostSettings` at `0x8000C080`, little-endian:

| Offset | Field      |                                                    |
|--------|------------|----------------------------------------------------|
| `0x00` | `magic`    | `0x53544553` (`SETS`)                              |
| `0x04` | `sequence` | change this on every push                          |
| `0x08` | `config`   | a complete `Config`, laid out by `config_schema.h` |

The payload is the ROM's whole config, so anything that can be built in can be pushed: relic and crystal times, lap count, token color, every feature flag, which modes appear, which bosses appear, and the `editable` mask. There is no second list of pushable fields to keep in step — include `src/config/config_schema.h` on the editor side and fill it in.

Times are milliseconds, so 1 second is `1000` and 1:17.0 is `77000`. Valid range is 500 to 599500 and anything outside it is clamped. Values that are not a multiple of 500 work, they just will not line up with the panel's half-second steps.

Fill in the config's own `magic`, `version` and `size` as well as the outer ones. The ROM checks `magic`, `version` and `size` before applying a push, so a stale or half-written block cannot be applied.

**Write the payload first and `sequence` last.** The ROM latches on `sequence` changing, so writing it early lets a half-finished block be read.

Three things to get right:

- **A push replaces the whole config**, not just the fields you care about. Sending a default-constructed struct silently resets modes, features and the editable mask. Seed it from the values the ROM reports, or from the mod's `src/user_config.h` and `src/config/config_default.h`, and change what you need.
- **`sequence` must differ from the previous push, including across editor restarts.** The ROM only compares for inequality, so wrap-around and resets are fine — but a counter starting at 0 on every launch will have its first push ignored if the ROM last saw 0. Seed it from a timestamp or persist it.
- **Send the time fields as signed 32-bit.** A value with the top bit set reads as negative and clamps to the *minimum*, which looks like "the push never arrived" rather than an obvious error.

A push overrides whatever is in the panel and takes effect immediately — toggling a mode off rebuilds the main menu, changing the ghost setting re-patches the code. Panel edits made afterwards stick until the next push, and if the panel is open when a push arrives it updates to show the new values. Relic times are written into the level's slot as the race loads, which includes hot-reloaded tracks.

Nothing is written back to disc, so a push only lasts the session.

## Ghost export

`SAVE GHOST` on the time trial results screen writes the ghost to the staging buffer and raises the ready flag at `0x8000C008` instead of going to a memory card.
