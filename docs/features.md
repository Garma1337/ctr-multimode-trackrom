# Features

Each of these has a toggle in the config and in the settings panel, so a shipped ROM can include or drop them individually. See [Customization](./customization.md).

## Freecam

Press `L3` + `R3` during a race. Every kart is halted and all quadblocks are forced visible so you can fly outside the normal view and still see the level. `L2` cycles the fly speed, `R2` hides the control help.

Because the visibility tree is bypassed, what you see in freecam is not what the renderer would normally draw — use the debug HUD's `QUADS` row for that, not freecam.

Note that **High Detail Tracks** also changes what is drawn: with it on, every quadblock renders at full detail regardless of distance. Turn it off in the settings when you want realistic numbers.

Toggling it off restores the karts and the normal camera. Freecam only becomes available once the race intro fly-in is over.

## Debug HUD

Press `SELECT` during a race. It has a feature flag and does nothing when that is off — see [Customization](./customization.md). Like freecam it only becomes available once the race intro fly-in is over, and it hides the racing HUD while it is up.

Frame timings are measured against a two-frame budget, so **100% is 30fps** and anything above that is a dropped frame. Bars turn amber past 75% and red past 100%. The graph down the left edge holds the last 48 frames with a line marking the budget.

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

## Reserve bar

Sits directly under the powerslide meter and matches its size. It shows the reserves the player has accumulated: each filled bar is five seconds of fire, and the number to its left counts the whole levels banked on top of that. The color steps through red, orange, yellow, green, cyan, blue and purple as levels climb; grey is empty and magenta means Saffi fire.

It is only drawn while the speedometer is hidden, so **triangle toggles it along with the minimap**.

## Updated engine stats

Vanilla CTR has a poor stat balanced and the updated (OnlineCTR) stats have more or less become the default. With the toggle off the original stats are used.
