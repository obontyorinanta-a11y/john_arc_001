#!/usr/bin/env python3
# -----------------------------------------------------------------------
#  measure_art.py
#
#  Prints the three registration fractions that drawCharacter() in
#  Assets.hpp needs for a character sprite:
#
#     bodyFrac - height of the figure alone (sword excluded) / canvas height
#     feetFrac - where the soles sit, as a fraction down from the canvas top
#     cxFrac   - horizontal centre of the figure / canvas width
#
#  A raised sword or an outstretched blade is only a few pixels wide, so it
#  is separated from the body by counting opaque pixels per row and ignoring
#  rows that are thinner than 14% of the widest row.
#
#  Run it after re-exporting any character art, then paste the numbers into
#  the ART_* constants in Player.hpp / Units.hpp.
#
#      python tools/measure_art.py
#      python tools/measure_art.py demo/demo/Assets/player/joan_run_00.png
#
#  Requires: Pillow      ->   pip install pillow
# -----------------------------------------------------------------------

import os
import re
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow is not installed.  Run:   pip install pillow")

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
GAME_DIR = os.path.join(TOOLS_DIR, "..", "demo", "demo")
HEADER = os.path.join(GAME_DIR, "AssetList.hpp")

# the sprites drawCharacter() is actually used for
DEFAULT = [
    "Assets/player/joan_run_00.png",
    "Assets/player/joan_run_06.png",
    "Assets/player/joan_swordup.png",
    "Assets/player/joan_sworddown.png",
    "Assets/enemy/soldier_swordup.png",
    "Assets/enemy/soldier_sworddown.png",
    "Assets/boss/commander_idle.png",
]

OPAQUE = 60          # alpha above this counts as artwork
THIN = 0.14          # rows narrower than this share of the widest are sword


def artwork_size(path):
    """Real artwork size from AssetList.hpp, not the power-of-two canvas."""
    key = path.replace("\\", "/")
    if os.path.isfile(HEADER):
        with open(HEADER, "r", encoding="utf-8") as f:
            for line in f:
                m = re.match(r'\s*\{\s*"([^"]+)",\s*(\d+),\s*(\d+),', line)
                if m and m.group(1) == key:
                    return int(m.group(2)), int(m.group(3))
    im = Image.open(os.path.join(GAME_DIR, path))
    return im.size


def measure(path):
    w, h = artwork_size(path)
    im = Image.open(os.path.join(GAME_DIR, path)).convert("RGBA")
    px = im.load()

    rows = [sum(1 for x in range(w) if px[x, y][3] > OPAQUE) for y in range(h)]
    widest = max(rows)
    if widest == 0:
        return None

    limit = max(4, widest * THIN)
    dense = [y for y, c in enumerate(rows) if c >= limit]
    head, feet = dense[0], dense[-1]

    total = 0
    weighted = 0
    for y in range(head, feet + 1):
        for x in range(w):
            if px[x, y][3] > OPAQUE:
                total += 1
                weighted += x

    return {
        "w": w, "h": h,
        "bodyFrac": (feet - head) / float(h),
        "feetFrac": feet / float(h),
        "cxFrac": weighted / float(max(1, total) * w),
    }


def main():
    paths = sys.argv[1:] or DEFAULT
    print("%-38s %6s %6s %9s %9s %9s"
          % ("sprite", "w", "h", "bodyFrac", "feetFrac", "cxFrac"))
    print("-" * 82)
    for p in paths:
        p = p.replace("\\", "/")
        if p.startswith("demo/demo/"):
            p = p[len("demo/demo/"):]
        if not os.path.isfile(os.path.join(GAME_DIR, p)):
            print("%-38s  not found" % os.path.basename(p))
            continue
        m = measure(p)
        if not m:
            print("%-38s  fully transparent" % os.path.basename(p))
            continue
        print("%-38s %6d %6d %9.4f %9.4f %9.4f"
              % (os.path.basename(p), m["w"], m["h"],
                 m["bodyFrac"], m["feetFrac"], m["cxFrac"]))
    print("")
    print("Paste these into the ART_* constants in Player.hpp / Units.hpp.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
