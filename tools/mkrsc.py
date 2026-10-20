#!/usr/bin/env python3
"""Build TYPESHLF.RSC, the resource file with the toolbar's icons.

    mkrsc.py OUT.RSC          write the file from res/icons/*.txt
    mkrsc.py --check FILE     read it back and check its structure

Each icon is a text file of size rows of size characters, '#' for a set pixel
and '.' for a clear one.  The file has two object trees, one per bitmap size:
tree 0 for 16x16 and tree 1 for 8x8, each a root box with the icons as its
children in the order of ICONS below (src/ui/icons.h has the same order).  An
icon is a G_ICON whose mask is solid, so it draws as black on white.

The file is the classic big-endian resource file (header, objects, icon
blocks, strings, image data, tree index), which pTOS's portable loader reads on
every architecture.  A colour icon (a G_CICON) can be added next to the
monochrome ones later.
"""
import os
import struct
import sys

ICONS = ["back", "icons", "samples", "list", "specimen"]
SIZES = [16, 8]                 # tree 0, tree 1

G_BOX, G_ICON = 20, 31
OF_LASTOB = 0x20
HEADER, OBJECT, ICONBLK = 36, 24, 34
NIL = -1


def read_icon(res_dir, name, size):
    path = os.path.join(res_dir, "icons", "%s%d.txt" % (name, size))
    rows = open(path).read().split()
    if len(rows) != size:
        raise SystemExit("%s: %d rows, want %d" % (path, len(rows), size))
    words = []
    for row in rows:
        if len(row) != size or set(row) - set("#."):
            raise SystemExit("%s: a row is not %d of '#' and '.'" % (path, size))
        words.append(sum(0x8000 >> x for x, c in enumerate(row) if c == "#"))
    if not any(words):
        raise SystemExit("%s: no pixel is set" % path)
    return words


def build(res_dir):
    n_icons = len(ICONS)
    per_tree = 1 + n_icons
    n_obj = per_tree * len(SIZES)
    n_ib = n_icons * len(SIZES)

    off_obj = HEADER
    off_ib = off_obj + n_obj * OBJECT
    off_str = off_ib + n_ib * ICONBLK
    strings = b"\0\0"                       # one empty string, padded to a word
    off_im = off_str + len(strings)

    # image data: a mask then the bitmap, for each icon
    images = []
    off = off_im
    ib_off = []
    for size in SIZES:
        for name in ICONS:
            words = read_icon(res_dir, name, size)
            mask = [0xFFFF if size == 16 else 0xFF00] * size
            blob = struct.pack(">%dH" % size, *mask) + struct.pack(">%dH" % size, *words)
            ib_off.append((off, off + 2 * size))
            images.append(blob)
            off += len(blob)
    off_tr = off
    total = off_tr + 4 * len(SIZES)

    objects = b""
    iconblks = b""
    tree_starts = []
    ib_index = 0
    for t, size in enumerate(SIZES):
        first = t * per_tree
        tree_starts.append(off_obj + first * OBJECT)
        spec = 0x00000000
        # links are indexes inside the tree: its root is 0, whatever its place in the file
        objects += struct.pack(">hhhHHHlhhhh", NIL, 1, n_icons, G_BOX, 0, 0,
                               spec, 0, 0, size << 8, size << 8)
        for i in range(n_icons):
            idx = first + 1 + i
            nxt = 0 if i == n_icons - 1 else i + 2
            flags = OF_LASTOB if i == n_icons - 1 else 0    # the last object of the tree
            spec = off_ib + ib_index * ICONBLK
            objects += struct.pack(">hhhHHHlhhhh", nxt, NIL, NIL, G_ICON, flags, 0,
                                   spec, 0, 0, size << 8, size << 8)
            # a bitmap is a whole number of 16 pixel words wide, so the 8x8 icons are 16
            # wide in their ICONBLK (the mask clears the right half) and 8 in the object
            mask_off, data_off = ib_off[ib_index]
            iconblks += struct.pack(">lllhhhhhhhhhhh", mask_off, data_off, off_str,
                                    0x1000, 0, 0, 0, 0, 16, size, 0, 0, 0, 0)    # black on white
            ib_index += 1

    header = struct.pack(">18H", 0, off_obj, off_ib, off_ib, off_ib + n_ib * ICONBLK, off_str,
                         off_str, off_im, off_tr, off_tr,
                         n_obj, len(SIZES), 0, n_ib, 0, 0, 0, total)
    body = header + objects + iconblks + strings + b"".join(images)
    body += b"".join(struct.pack(">l", s) for s in tree_starts)
    assert len(body) == total, (len(body), total)
    return body


def check(path):
    d = open(path, "rb").read()
    h = struct.unpack(">18H", d[:HEADER])
    n_obj, n_tree, n_ted, n_ib, n_bb = h[10], h[11], h[12], h[13], h[14]
    ok = (h[0] == 0 and h[17] == len(d) and n_tree == len(SIZES) and n_ted == 0 and n_bb == 0
          and n_obj == (1 + len(ICONS)) * len(SIZES) and n_ib == len(ICONS) * len(SIZES))
    per_tree = 1 + len(ICONS)
    for t in range(n_tree):
        for i in range(per_tree):
            o = struct.unpack(">hhhHHHlhhhh", d[h[1] + (t * per_tree + i) * OBJECT:
                                                h[1] + (t * per_tree + i + 1) * OBJECT])
            if i == 0:      # the root: icons 1 to n, itself unlinked
                ok = ok and o[3] == G_BOX and o[0] == NIL and o[1] == 1 and o[2] == len(ICONS)
            else:           # an icon: the next one, or back to the root; no children
                ok = ok and o[3] == G_ICON and o[1] == NIL and o[2] == NIL
                ok = ok and o[0] == (0 if i == len(ICONS) else i + 1)
    for t in range(n_tree):
        end = (t + 1) * (1 + len(ICONS)) - 1
        last = struct.unpack(">hhhHHHlhhhh", d[h[1] + end * OBJECT:h[1] + (end + 1) * OBJECT])
        ok = ok and last[4] & OF_LASTOB
    for t in range(n_tree):
        start = struct.unpack(">l", d[h[9] + 4 * t:h[9] + 4 * t + 4])[0]
        ok = ok and start == h[1] + t * (1 + len(ICONS)) * OBJECT
    if not ok:
        raise SystemExit("%s: not the file mkrsc.py writes" % path)
    print("%s: ok, %d bytes, %d trees, %d icons" % (path, len(d), n_tree, n_ib))


def main(argv):
    here = os.path.dirname(os.path.abspath(__file__))
    res = os.path.join(here, "..", "res")
    if len(argv) == 3 and argv[1] == "--check":
        check(argv[2])
    elif len(argv) == 2:
        os.makedirs(os.path.dirname(os.path.abspath(argv[1])), exist_ok=True)
        open(argv[1], "wb").write(build(res))
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main(sys.argv)
