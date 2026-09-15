#!/usr/bin/env python3
# -----------------------------------------------------------------------
#  prepare_assets.py
#  Asset pipeline for "Joan of Arc : Banner of Orleans"
#
#  Standalone tool. It is NOT part of the C++ build and the game never
#  runs it. You run it by hand whenever the art changes.
#
#  What it does for every job listed in assets.json:
#     1. open the source image from tools/source_art/
#     2. optionally crop a rectangle out of it
#     3. turn the grey/white background into real transparency
#     4. trim away the empty border
#     5. fit the result into a power-of-two canvas (OpenGL 1.1 needs that)
#     6. write the PNG into demo/demo/Assets/<category>/
#     7. write a checkerboard preview into tools/preview/ so you can see
#        whether the background removal actually worked
#
#  Then it regenerates demo/demo/AssetList.hpp, which gives the C++ side
#  an enum of sprite ids, the pixel sizes and the UV sub-rectangle of
#  every asset, plus an animation table built from _00 _01 _02 suffixes.
#
#  Requires: Pillow      ->   pip install pillow
# -----------------------------------------------------------------------

import argparse
import json
import os
import sys
from collections import deque

try:
    from PIL import Image, ImageDraw, ImageFilter, ImageChops
except ImportError:
    sys.exit("Pillow is not installed.  Run:   pip install pillow")

TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
MANIFEST = os.path.join(TOOLS_DIR, "assets.json")
PREVIEW_DIR = os.path.join(TOOLS_DIR, "preview")

# categories the game understands; each becomes a sub folder of Assets/
CATEGORIES = ["player", "enemy", "boss", "background", "items", "effects", "ui"]

GENERATED_HEADER_NAME = "AssetList.hpp"


# ----------------------------------------------------------------------
#  helpers
# ----------------------------------------------------------------------

def next_pot(n):
    """smallest power of two >= n, minimum 2"""
    p = 2
    while p < n:
        p *= 2
    return p


def load_manifest(path):
    if not os.path.isfile(path):
        sys.exit("manifest not found: %s" % path)
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def guess_background_colour(img):
    """most common colour along the four borders"""
    w, h = img.size
    px = img.load()
    counts = {}
    step = max(1, min(w, h) // 200)
    for x in range(0, w, step):
        for y in (0, h - 1):
            counts[px[x, y][:3]] = counts.get(px[x, y][:3], 0) + 1
    for y in range(0, h, step):
        for x in (0, w - 1):
            counts[px[x, y][:3]] = counts.get(px[x, y][:3], 0) + 1
    return max(counts.items(), key=lambda kv: kv[1])[0]


def colour_distance_image(img_rgb, colour):
    """greyscale image: 0 = identical to colour, 255 = far away from it"""
    solid = Image.new("RGB", img_rgb.size, colour)
    return colour_distance_image_pair(img_rgb, solid)


def colour_distance_image_pair(a_rgb, b_rgb):
    """greyscale image: per pixel distance between two images"""
    diff = ImageChops.difference(a_rgb, b_rgb)
    r, g, b = diff.split()
    # max of the three channels keeps a strong signal on coloured edges
    return ImageChops.lighter(ImageChops.lighter(r, g), b)


# ----------------------------------------------------------------------
#  background removal
# ----------------------------------------------------------------------

def remove_background(img, bg_colour=None, tolerance=42, feather=26,
                      local_tolerance=14, edge_block=10):
    """
    Flood fills from the image border inwards, so only background that is
    actually connected to the edge is erased.  Grey armour or white cloth
    inside the character keeps its pixels, which a plain colour key would
    destroy.

    Two tests must both pass before a pixel counts as background:

      local  - it must look like the neighbour it spread from.  A studio
               backdrop is a smooth gradient, so neighbouring background
               pixels barely differ, while the silhouette edge is a big
               jump.  This is what lets grey-blue armour survive on a grey
               backdrop, where a single global tolerance fails.
      global - it must still be in the neighbourhood of the backdrop
               colour, which stops the fill leaking through a dark outline
               and eating the figure from inside.

    tolerance       : global limit, distance from the backdrop colour
    local_tolerance : per step limit, 0 disables the local test
    feather         : width of the soft alpha ramp at the silhouette edge
    """
    img = img.convert("RGBA")
    w, h = img.size
    rgb = img.convert("RGB")

    if bg_colour is None:
        bg_colour = guess_background_colour(rgb)

    # ---- 1. flood fill from every border pixel -----------------------
    # mask: 255 = background, 0 = keep
    mask = Image.new("L", (w, h), 0)
    mpx = mask.load()
    px = rgb.load()

    br, bgc, bb = bg_colour
    tol_sq = tolerance * tolerance * 3
    loc_sq = local_tolerance * local_tolerance * 3

    # Edge map: how much each pixel differs from its own blurred neighbourhood.
    # A smooth studio backdrop scores near zero, the outline of the figure
    # scores high, so the fill can be forbidden from crossing it.
    edge_px = None
    if edge_block > 0:
        blurred = rgb.filter(ImageFilter.GaussianBlur(2))
        edge_px = colour_distance_image_pair(rgb, blurred).load()

    def near_bg(x, y):
        r, g, b = px[x, y]
        dr = r - br
        dg = g - bgc
        db = b - bb
        return (dr * dr + dg * dg + db * db) <= tol_sq

    def near_each_other(ax, ay, bx, by):
        if loc_sq <= 0:
            return True
        r1, g1, b1 = px[ax, ay]
        r2, g2, b2 = px[bx, by]
        dr = r1 - r2
        dg = g1 - g2
        db = b1 - b2
        return (dr * dr + dg * dg + db * db) <= loc_sq

    queue = deque()
    seeds = []
    for x in range(w):
        seeds.append((x, 0))
        seeds.append((x, h - 1))
    for y in range(h):
        seeds.append((0, y))
        seeds.append((w - 1, y))

    for s in seeds:
        x, y = s
        if mpx[x, y] == 0 and near_bg(x, y):
            mpx[x, y] = 255
            queue.append(s)

    while queue:
        x, y = queue.popleft()
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if 0 <= nx < w and 0 <= ny < h:
                if mpx[nx, ny] != 0:
                    continue
                if edge_px is not None and edge_px[nx, ny] > edge_block:
                    continue                      # an outline, do not cross it
                if near_bg(nx, ny) and near_each_other(x, y, nx, ny):
                    mpx[nx, ny] = 255
                    queue.append((nx, ny))

    # ---- 2. soft edge --------------------------------------------------
    # inside the ring around the erased area, fade alpha by how far the
    # pixel colour sits from the background colour
    dist = colour_distance_image(rgb, bg_colour)
    ramp = dist.point(lambda v: 0 if v <= 4 else min(255, int(v * 255.0 / max(1, feather))))

    grown = mask.filter(ImageFilter.MaxFilter(5))          # background, widened
    ring = ImageChops.subtract(grown, mask)                # the edge band only

    alpha = Image.new("L", (w, h), 255)
    alpha.paste(ramp, (0, 0), ring)                        # soft in the band
    alpha.paste(0, (0, 0), mask)                           # hard zero on background

    out = img.copy()
    out.putalpha(alpha)
    return out


def punch_colour_key(img, colour=(255, 255, 255), threshold=215, feather=4):
    """
    Erase every pixel close to `colour`, whether or not it is connected to the
    image border.

    remove_background() deliberately refuses to do this: it floods inward from
    the edge precisely so that white cloth or pale armour INSIDE a figure keeps
    its pixels.  But some source art arrives with a hole in the cutout that the
    figure itself seals off - a gap between a sword arm and a cloak, say - and
    no amount of flooding from outside can ever reach it.  This is the tool for
    that case.

    It is only safe when the artwork genuinely contains nothing that bright.
    Check first:  the run frames here peak at ~197 in the darkest channel for
    real pixels, and everything above 215 is hole, so a 215 threshold clears it
    without touching a single highlight.

    threshold : channel value above which a pixel counts as the key colour;
                255 disables the step
    feather   : blur radius on the cut, to keep the new edge from aliasing
    """
    if threshold >= 255:
        return img

    img = img.convert("RGBA")
    rgb = img.convert("RGB")
    alpha = img.split()[3]

    # distance from the key colour, 0 = identical
    dist = colour_distance_image(rgb, tuple(colour))

    # keep = far enough from the key colour
    tol = 255 - threshold
    keep = dist.point(lambda v: 255 if v > tol else 0)

    if feather > 0:
        keep = keep.filter(ImageFilter.GaussianBlur(feather))

    img.putalpha(ImageChops.multiply(alpha, keep))
    return img


def punch_grey_key(img, sat=30, val=120, feather=2):
    """
    Erase every pixel that is both NEUTRAL and DARK, wherever it sits.

    This exists for source art that ships with a transparency checkerboard
    already flattened into the pixels.  remove_background() cannot deal with
    that: it floods inward and needs neighbouring background pixels to look
    alike, but a checkerboard alternates between two greys, so every checker
    edge is a hard jump that stops the fill dead.  punch_colour_key() cannot
    deal with it either, because there are two different背景 greys, not one.

    What the checker and the artwork DO differ in is colour.  The checker is
    perfectly neutral (r == g == b); a flame is strongly saturated.  So the
    test is on saturation, not on any particular grey:

        keep the pixel if  (max - min) >= sat   - it has colour in it
        or if              max         >= val   - it is bright enough to matter

    sat     : channel spread below which a pixel counts as neutral
    val     : brightness above which a neutral pixel is kept anyway, so white
              highlights inside a flame survive
    feather : blur radius on the cut
    """
    img = img.convert("RGBA")
    w, h = img.size
    rgb = img.convert("RGB")
    src = rgb.load()

    keep = Image.new("L", (w, h), 0)
    kp = keep.load()

    for y in range(h):
        for x in range(w):
            r, g, b = src[x, y]
            hi = r if r > g else g
            if b > hi:
                hi = b
            lo = r if r < g else g
            if b < lo:
                lo = b
            if (hi - lo) >= sat or hi >= val:
                kp[x, y] = 255

    if feather > 0:
        keep = keep.filter(ImageFilter.GaussianBlur(feather))

    img.putalpha(ImageChops.multiply(img.split()[3], keep))
    return img


def trim_to_content(img, padding=0, alpha_threshold=8):
    """crop away fully transparent border rows and columns"""
    alpha = img.split()[3]
    box = alpha.point(lambda v: 255 if v > alpha_threshold else 0).getbbox()
    if box is None:
        return img
    if padding:
        x0, y0, x1, y1 = box
        box = (max(0, x0 - padding), max(0, y0 - padding),
               min(img.size[0], x1 + padding), min(img.size[1], y1 + padding))
    return img.crop(box)


def fit_into_pot(img, max_size=0):
    """
    Scale down if larger than max_size, then paste into a transparent
    power-of-two canvas anchored at the TOP-LEFT.  Aspect ratio is never
    distorted; the unused part of the canvas is reported back as the UV
    sub-rectangle so the game only samples the real pixels.
    """
    w, h = img.size
    if max_size and (w > max_size or h > max_size):
        scale = min(float(max_size) / w, float(max_size) / h)
        w = max(1, int(round(w * scale)))
        h = max(1, int(round(h * scale)))
        img = img.resize((w, h), Image.LANCZOS)

    tw, th = next_pot(w), next_pot(h)
    canvas = Image.new("RGBA", (tw, th), (0, 0, 0, 0))
    canvas.paste(img, (0, 0))
    return canvas, w, h, tw, th


def write_preview(img, name):
    """checkerboard composite so transparency is visible to the eye"""
    if not os.path.isdir(PREVIEW_DIR):
        os.makedirs(PREVIEW_DIR)
    w, h = img.size
    board = Image.new("RGBA", (w, h), (255, 255, 255, 255))
    d = ImageDraw.Draw(board)
    tile = 16
    for y in range(0, h, tile):
        for x in range(0, w, tile):
            if ((x // tile) + (y // tile)) % 2 == 0:
                d.rectangle([x, y, x + tile - 1, y + tile - 1], fill=(202, 202, 210, 255))
    board.alpha_composite(img)
    board.convert("RGB").save(os.path.join(PREVIEW_DIR, name + ".png"))


# ----------------------------------------------------------------------
#  one job
# ----------------------------------------------------------------------

def process_job(job, out_root, quiet=False):
    """returns a record dict for the generated header, or None if skipped"""
    name = job["out"]
    category = job.get("category", "ui")
    if category not in CATEGORIES:
        print("  ! unknown category '%s' in job '%s'" % (category, name))
        return None

    if not job.get("enabled", True):
        return None

    src = job["src"]
    if not os.path.isabs(src):
        src = os.path.join(TOOLS_DIR, src)

    if not os.path.isfile(src):
        print("  - %-28s SKIPPED (source not found: %s)" % (name, job["src"]))
        return None

    img = Image.open(src).convert("RGBA")

    crop = job.get("crop")
    if crop:
        x, y, w, h = crop
        img = img.crop((x, y, x + w, y + h))

    if job.get("remove_bg", True):
        bg = job.get("bg_color")
        if isinstance(bg, list):
            bg = tuple(bg)
        img = remove_background(
            img,
            bg_colour=bg,
            tolerance=job.get("tolerance", 42),
            feather=job.get("feather", 26),
            local_tolerance=job.get("local_tolerance", 14),
            edge_block=job.get("edge_block", 10),
        )

    # Runs after the flood fill and before the trim, so a hole the flood could
    # not reach is gone before the bounding box is measured.
    if job.get("key_grey_sat", 0) > 0:
        img = punch_grey_key(
            img,
            sat=job.get("key_grey_sat", 30),
            val=job.get("key_grey_val", 120),
            feather=job.get("key_feather", 2),
        )

    if job.get("key_white", 255) < 255:
        img = punch_colour_key(
            img,
            colour=tuple(job.get("key_colour", [255, 255, 255])),
            threshold=job.get("key_white", 255),
            feather=job.get("key_feather", 4),
        )

    if job.get("trim", True):
        img = trim_to_content(img, padding=job.get("padding", 0))

    canvas, w, h, tw, th = fit_into_pot(img, job.get("max_size", 0))

    out_dir = os.path.join(out_root, category)
    if not os.path.isdir(out_dir):
        os.makedirs(out_dir)
    out_path = os.path.join(out_dir, os.path.basename(name) + ".png")
    canvas.save(out_path)

    if job.get("preview", True):
        write_preview(canvas, category + "_" + os.path.basename(name))

    if not quiet:
        print("  + %-28s %4dx%-4d in %4dx%-4d canvas   -> Assets/%s/%s.png"
              % (name, w, h, tw, th, category, os.path.basename(name)))

    return {
        "id": sprite_id_for(category, os.path.basename(name)),
        "path": "Assets/%s/%s.png" % (category, os.path.basename(name)),
        "w": w, "h": h, "tw": tw, "th": th,
        "category": category,
        "base": os.path.basename(name),
    }


def sprite_id_for(category, base):
    return ("SPR_" + category + "_" + base).upper().replace("-", "_").replace(" ", "_")


# ----------------------------------------------------------------------
#  generated header
# ----------------------------------------------------------------------

def animation_groups(records):
    """
    Group sprites whose names end in _00 _01 _02 ... into animations.
    joan_run_00, joan_run_01  ->  ANIM_PLAYER_JOAN_RUN with 2 frames.
    A lone sprite still gets a one frame animation, which keeps the
    drawing code uniform.
    """
    groups = []
    seen = {}
    for index, rec in enumerate(records):
        base = rec["base"]
        stem = base
        if "_" in base and base.rsplit("_", 1)[1].isdigit():
            stem = base.rsplit("_", 1)[0]
        key = rec["category"] + "/" + stem
        if key in seen:
            groups[seen[key]]["count"] += 1
        else:
            seen[key] = len(groups)
            groups.append({
                "id": ("ANIM_" + rec["category"] + "_" + stem).upper().replace("-", "_"),
                "first": index,
                "count": 1,
            })
    return groups


def write_header(records, path):
    groups = animation_groups(records)
    lines = []
    a = lines.append

    a("// ---------------------------------------------------------------")
    a("//  AssetList.hpp   ***  AUTO-GENERATED FILE - DO NOT EDIT BY HAND ***")
    a("//")
    a("//  Produced by tools/prepare_assets.py")
    a("//  Re-run that script after adding or changing any artwork.")
    a("// ---------------------------------------------------------------")
    a("")
    a("#ifndef ASSETLIST_HPP")
    a("#define ASSETLIST_HPP")
    a("")
    a("#define ASSET_SPRITE_COUNT %d" % len(records))
    a("#define ASSET_ANIM_COUNT   %d" % len(groups))
    a("")

    if records:
        a("// every sprite the pipeline exported, in table order")
        a("enum SpriteId {")
        for rec in records:
            a("\t%s," % rec["id"])
        a("\tSPR_COUNT")
        a("};")
    else:
        a("// no artwork exported yet - drop files in tools/source_art and re-run")
        a("enum SpriteId { SPR_COUNT };")
    a("")

    a("struct AssetEntry {")
    a("\tconst char* path;   // relative to the working directory (the project folder)")
    a("\tint w, h;           // real pixel size of the artwork")
    a("\tint texW, texH;     // power-of-two canvas it sits in")
    a("};")
    a("")

    a("static const AssetEntry ASSET_TABLE[%d] = {" % max(1, len(records)))
    if records:
        for rec in records:
            a("\t{ \"%s\", %d, %d, %d, %d }," % (rec["path"], rec["w"], rec["h"], rec["tw"], rec["th"]))
    else:
        a("\t{ \"\", 0, 0, 1, 1 }   // placeholder, table is empty")
    a("};")
    a("")

    if groups:
        a("// frame sequences, grouped from _00 _01 _02 style file names")
        a("enum AnimId {")
        for g in groups:
            a("\t%s," % g["id"])
        a("\tANIM_COUNT")
        a("};")
    else:
        a("enum AnimId { ANIM_COUNT };")
    a("")

    a("struct AnimEntry {")
    a("\tint first;   // index of frame 0 inside ASSET_TABLE")
    a("\tint count;   // number of frames")
    a("};")
    a("")
    a("static const AnimEntry ANIM_TABLE[%d] = {" % max(1, len(groups)))
    if groups:
        for g in groups:
            a("\t{ %d, %d }," % (g["first"], g["count"]))
    else:
        a("\t{ 0, 0 }   // placeholder, table is empty")
    a("};")
    a("")
    a("#endif")
    a("")

    with open(path, "w", encoding="utf-8", newline="\r\n") as f:
        f.write("\n".join(lines))

    return len(groups)


# ----------------------------------------------------------------------
#  extra commands
# ----------------------------------------------------------------------

def cmd_probe(paths):
    """print size and border colour so you can work out crop boxes"""
    for p in paths:
        full = p if os.path.isabs(p) else os.path.join(TOOLS_DIR, p)
        if not os.path.isfile(full):
            print("%s : not found" % p)
            continue
        img = Image.open(full).convert("RGB")
        print("%s : %d x %d   border colour %s"
              % (p, img.size[0], img.size[1], guess_background_colour(img)))


def cmd_selftest():
    """
    Builds a throwaway image with a grey background, a white shape and a
    grey shape inside the figure, runs it through the whole pipeline and
    checks the result.  Proves the tool works without needing real art.
    """
    print("self test")
    img = Image.new("RGB", (300, 220), (63, 63, 63))
    d = ImageDraw.Draw(img)
    d.ellipse([90, 40, 210, 180], fill=(40, 90, 180))
    d.rectangle([130, 80, 170, 140], fill=(63, 63, 63))   # grey INSIDE the figure
    d.ellipse([120, 60, 150, 90], fill=(255, 255, 255))   # white highlight

    cut = remove_background(img, tolerance=42, feather=26)
    cut = trim_to_content(cut)
    canvas, w, h, tw, th = fit_into_pot(cut, 0)
    write_preview(canvas, "selftest")

    alpha = canvas.split()[3]
    corner = alpha.getpixel((0, 0))
    inner_grey = cut.getpixel((cut.size[0] // 2, cut.size[1] // 2))

    print("  trimmed size      : %d x %d" % (w, h))
    print("  canvas size       : %d x %d  (power of two: %s)"
          % (tw, th, "yes" if (tw & (tw - 1)) == 0 and (th & (th - 1)) == 0 else "NO"))
    print("  outside alpha     : %d   (must be 0)" % corner)
    print("  grey inside figure: alpha %d   (must stay 255, a colour key would kill it)"
          % inner_grey[3])
    print("  preview written   : tools/preview/selftest.png")

    ok = (corner == 0 and inner_grey[3] == 255 and (tw & (tw - 1)) == 0)
    print("  RESULT            : %s" % ("PASS" if ok else "FAIL"))
    return 0 if ok else 1


# ----------------------------------------------------------------------
#  main
# ----------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(
        description="Prepare game art for the iGraphics project.")
    ap.add_argument("--manifest", default=MANIFEST, help="path to assets.json")
    ap.add_argument("--only", metavar="CATEGORY",
                    help="process one category only (player, enemy, boss, background, items, effects, ui)")
    ap.add_argument("--job", metavar="NAME", help="process a single job by its 'out' name")
    ap.add_argument("--probe", nargs="+", metavar="IMAGE",
                    help="print size and border colour of images, then exit")
    ap.add_argument("--selftest", action="store_true",
                    help="run the pipeline on a generated test image and exit")
    args = ap.parse_args()

    if args.probe:
        cmd_probe(args.probe)
        return 0

    if args.selftest:
        return cmd_selftest()

    man = load_manifest(args.manifest)
    out_root = man.get("output_dir", "../demo/demo/Assets")
    if not os.path.isabs(out_root):
        out_root = os.path.normpath(os.path.join(TOOLS_DIR, out_root))

    header_dir = man.get("header_dir", "../demo/demo")
    if not os.path.isabs(header_dir):
        header_dir = os.path.normpath(os.path.join(TOOLS_DIR, header_dir))

    jobs = man.get("jobs", [])
    if args.only:
        jobs = [j for j in jobs if j.get("category") == args.only]
    if args.job:
        jobs = [j for j in jobs if j.get("out") == args.job]

    print("asset pipeline")
    print("  manifest : %s" % args.manifest)
    print("  output   : %s" % out_root)
    print("  jobs     : %d" % len(jobs))
    print("")

    records = []
    skipped = 0
    for job in jobs:
        rec = process_job(job, out_root)
        if rec:
            records.append(rec)
        else:
            skipped += 1

    header_path = os.path.join(header_dir, GENERATED_HEADER_NAME)
    anim_count = write_header(records, header_path)

    print("")
    print("  exported %d sprite(s), %d animation group(s), skipped %d job(s)"
          % (len(records), anim_count, skipped))
    print("  wrote    %s" % header_path)
    print("  previews %s" % PREVIEW_DIR)
    print("")
    print("Now rebuild the Visual Studio project so the new AssetList.hpp is picked up.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
