# Asset pipeline

Turns raw artwork into game-ready PNGs plus the C++ header that lists them.
It runs on your machine only. The game never calls it, and Visual Studio never builds it.

## One time setup

```
pip install pillow
```

That is the only package needed. Nothing else, no numpy.

## Folders

```
tools/
  prepare_assets.py     the tool
  assets.json           the job list - this is the file you edit
  source_art/           raw artwork you drop in
  preview/              checkerboard previews so you can check the cutouts
demo/demo/
  Assets/               generated game art (player, enemy, boss, background, items, effects, ui)
  AssetList.hpp         GENERATED - never edit by hand
  Assets.hpp            hand written loader and draw helpers
```

## Everyday use

1. Put the raw image in `tools/source_art/`.
2. Find the crop rectangle you want:
   ```
   python prepare_assets.py --probe source_art/joan_turnaround.png
   ```
   It prints the image size and the border colour it detected.
3. Add or edit an entry in `assets.json`.
4. Run the tool:
   ```
   python prepare_assets.py
   ```
5. Open `tools/preview/` and check the cutout looks right on the checkerboard.
6. Rebuild in Visual Studio. The new sprite IDs are available in C++.

## Useful commands

| Command | What it does |
|---|---|
| `python prepare_assets.py` | process every job in assets.json |
| `python prepare_assets.py --only player` | process one category |
| `python prepare_assets.py --job joan_idle_00` | process a single job |
| `python prepare_assets.py --probe img.png` | print size and border colour |
| `python prepare_assets.py --selftest` | check the tool itself still works |

## Tuning the cutout

| Problem | Fix in assets.json |
|---|---|
| Bits of background survive around the figure | raise `tolerance` (try 55, 65) |
| The figure is being eaten away | lower `tolerance` (try 30, 22) |
| The edge looks jagged or has a hard fringe | raise `feather` (try 34) |
| A pale halo rings the sprite | lower `feather` (try 14) and raise `tolerance` slightly |
| The wrong colour is being erased | set `bg_color` explicitly, e.g. `[63, 63, 63]` |
| Sprite is far too large in memory | lower `max_size`, e.g. 256 |

The background is erased by flooding inwards from the image border, so grey
armour or white cloth **inside** the figure is never touched. A plain colour
key would destroy those.

## Animations

Name frames with a two digit suffix and the tool groups them automatically:

```
joan_run_00.png
joan_run_01.png
joan_run_02.png
```

becomes `ANIM_PLAYER_JOAN_RUN` with 3 frames, drawn in C++ with:

```cpp
drawAnimH(ANIM_PLAYER_JOAN_RUN, animFrame / 4, x, y, height, facingLeft);
```

## Why power of two

The OpenGL 1.1 driver behind GLUT refuses textures whose sides are not powers
of two - they simply do not draw. Every export is therefore pasted into a
power-of-two canvas at the top-left, without stretching. The unused part of the
canvas is recorded in `AssetList.hpp` as the real width and height, and the
draw helpers sample only that region, so nothing looks squashed.
