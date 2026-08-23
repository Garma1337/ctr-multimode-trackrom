# Keymap

Bindings are compile-time constants in `src/dll/input.h`. There is no remapping screen; change the defines and rebuild.

## Settings panel

| Input        | Effect                                      |
|--------------|---------------------------------------------|
| Up / Down    | pick a field, scrolls when the list is long |
| Left / Right | -/+ 0.5s, step a value, or flip a toggle    |
| L1 / R1      | -/+ 5s                                      |
| Cross        | confirm, saves and closes                   |
| Triangle     | back, closes without saving                 |

Only Confirm and Back are hinted on the panel, back on the left and confirm on the right.

Edits only take effect when you press Confirm, so a stray press cannot disturb a running setup. Holding an adjust direction repeats after a short delay on time fields; toggles, lap count and the token color step once per press.

## In a race

| Input    | Effect                                                   |
|----------|----------------------------------------------------------|
| L3 + R3  | toggle freecam                                           |
| SELECT   | toggle the debug HUD                                     |
| Triangle | swap the minimap and the speedometer (vanilla behaviour) |

Freecam and the debug HUD each have a feature flag and do nothing when it is off — see [Customization](./customization.md).

Both only become available once the race intro camera fly-in is over, so neither can be summoned during the intro cutscene. Either one hides the racing HUD while it is up.

## Freecam

| Input            | Effect                    |
|------------------|---------------------------|
| D-pad            | move and strafe           |
| L1 / R1          | down / up                 |
| Triangle / Cross | pitch                     |
| Square / Circle  | yaw                       |
| L2               | cycle speed               |
| R2               | toggle the on-screen help |
