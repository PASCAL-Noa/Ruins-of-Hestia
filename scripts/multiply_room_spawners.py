#!/usr/bin/env python3
"""
multiply_room_spawners.py

Scale enemy and resource spawner counts in .room files.
Player spawner is preserved untouched.

Format binaire .room :
    [0..7]    Header        : u32 enemy_count | u32 resource_count   (LE)
    [8..264]  RawRoom       : TileMatrix (16x16 uint8) + Mask (uint8) = 257 B
    [265..272] PlayerSpawner : 2x float32 (vec2)
    [...]     N enemy spawners   (8 B each, vec2)
    [...]     M resource spawners (8 B each, vec2)

Tile convention (decoded from data) :
    Spawner pos (x, y) maps to tile at byte index (int(x) * 16 + int(y)).
    TileID 1 = walkable, TileID 0 = wall / empty terrain.

Placement strategy :
    New spawners are sampled UNIFORMLY at random from walkable tiles.
    Each spawner gets a random sub-tile offset (default 0.1..0.9) so identical
    tiles still yield distinct positions; some natural stacking will occur.
    Originals are kept untouched.

Usage :
    python multiply_room_spawners.py <input_dir> [--out <out_dir>]
                                     [--enemy-mult 1.75] [--resource-mult 2.5]
                                     [--walkable-tile 1] [--seed 42]
                                     [--in-place] [--recursive] [--dry-run]

Defaults : enemy x1.75, resource x2.5, walkable_tile=1.
out dir = <input_dir>_scaled (originals untouched).
--in-place writes back into <input_dir>, with .bak backups.
"""

import argparse
import os
import random
import shutil
import struct
import sys
from pathlib import Path

HEADER_FMT  = "<II"          # ennemi_count, resource_count
HEADER_SIZE = struct.calcsize(HEADER_FMT)         # 8
RAW_ROOM_SIZE  = 16 * 16 + 1                      # 257 (tiles + mask)
SPAWNER_FMT  = "<ff"                              # vec2
SPAWNER_SIZE = struct.calcsize(SPAWNER_FMT)       # 8
PLAYER_SIZE  = SPAWNER_SIZE                       # 8

FIXED_PREFIX_SIZE = HEADER_SIZE + RAW_ROOM_SIZE + PLAYER_SIZE  # 273


ROOM_DIM = 16
TILE_BYTES_OFFSET = HEADER_SIZE                    # tiles start right after header
TILE_BYTES_SIZE   = ROOM_DIM * ROOM_DIM            # 256 (mask is the byte after)


def parse_room(data: bytes):
    if len(data) < FIXED_PREFIX_SIZE:
        raise ValueError(f"file too short ({len(data)} B < {FIXED_PREFIX_SIZE})")

    enemy_count, resource_count = struct.unpack_from(HEADER_FMT, data, 0)

    raw_room = data[HEADER_SIZE:HEADER_SIZE + RAW_ROOM_SIZE]
    player   = data[HEADER_SIZE + RAW_ROOM_SIZE:FIXED_PREFIX_SIZE]

    expected = FIXED_PREFIX_SIZE + (enemy_count + resource_count) * SPAWNER_SIZE
    if len(data) != expected:
        raise ValueError(
            f"size mismatch: got {len(data)} B, expected {expected} B "
            f"(enemy={enemy_count}, resource={resource_count})"
        )

    enemies = []
    off = FIXED_PREFIX_SIZE
    for _ in range(enemy_count):
        enemies.append(struct.unpack_from(SPAWNER_FMT, data, off))
        off += SPAWNER_SIZE

    resources = []
    for _ in range(resource_count):
        resources.append(struct.unpack_from(SPAWNER_FMT, data, off))
        off += SPAWNER_SIZE

    return raw_room, player, enemies, resources


def encode_room(raw_room: bytes, player: bytes, enemies, resources) -> bytes:
    out = bytearray()
    out += struct.pack(HEADER_FMT, len(enemies), len(resources))
    out += raw_room
    out += player
    for x, y in enemies:
        out += struct.pack(SPAWNER_FMT, x, y)
    for x, y in resources:
        out += struct.pack(SPAWNER_FMT, x, y)
    return bytes(out)


def collect_walkable_cells(raw_room: bytes, walkable_value: int):
    """Return a list of (x, y) integer tile coords where tile == walkable_value.
    Convention: tile byte index = x * ROOM_DIM + y (verified empirically)."""
    cells = []
    tiles = raw_room[:TILE_BYTES_SIZE]
    for x in range(ROOM_DIM):
        base = x * ROOM_DIM
        for y in range(ROOM_DIM):
            if tiles[base + y] == walkable_value:
                cells.append((x, y))
    return cells


def scale_spawners(spawners, multiplier: float, walkable_cells, rng: random.Random,
                   sub_lo: float = 0.1, sub_hi: float = 0.9):
    """Scale count by multiplier; new spawners placed at uniformly-random
    walkable cells with random sub-tile offset. Existing spawners untouched.
    Stacking is allowed (cells sampled with replacement)."""
    if multiplier <= 1.0:
        return list(spawners)

    target = round(len(spawners) * multiplier)
    to_add = target - len(spawners)
    out = list(spawners)

    if to_add <= 0 or not walkable_cells:
        return out

    for _ in range(to_add):
        cx, cy = rng.choice(walkable_cells)
        fx = cx + rng.uniform(sub_lo, sub_hi)
        fy = cy + rng.uniform(sub_lo, sub_hi)
        out.append((fx, fy))

    return out


def process_file(in_path: Path, out_path: Path, args, rng: random.Random):
    data = in_path.read_bytes()
    raw_room, player, enemies, resources = parse_room(data)
    walkable = collect_walkable_cells(raw_room, args.walkable_tile)

    new_enemies   = scale_spawners(enemies,   args.enemy_mult,    walkable, rng)
    new_resources = scale_spawners(resources, args.resource_mult, walkable, rng)

    new_data = encode_room(raw_room, player, new_enemies, new_resources)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_bytes(new_data)

    return {
        "file":       in_path.name,
        "enemies":    (len(enemies),   len(new_enemies)),
        "resources":  (len(resources), len(new_resources)),
        "size":       (len(data),      len(new_data)),
        "walkable":   len(walkable),
    }


def collect_room_files(root: Path, recursive: bool):
    pattern = "**/*.room" if recursive else "*.room"
    return sorted(p for p in root.glob(pattern) if p.is_file())


def main():
    parser = argparse.ArgumentParser(description="Scale enemy/resource spawners in .room files.")
    parser.add_argument("input_dir", type=Path, help="directory containing .room files")
    parser.add_argument("--out", type=Path, default=None,
                        help="output directory (default: <input_dir>_scaled). Ignored if --in-place.")
    parser.add_argument("--enemy-mult",    type=float, default=1.75)
    parser.add_argument("--resource-mult", type=float, default=2.5)
    parser.add_argument("--walkable-tile", type=int, default=1,
                        help="TileID considered walkable (default 1)")
    parser.add_argument("--seed", type=int, default=42, help="RNG seed for reproducibility")
    parser.add_argument("--in-place", action="store_true",
                        help="overwrite originals (creates .bak backups)")
    parser.add_argument("--recursive", action="store_true",
                        help="recurse into subdirectories")
    parser.add_argument("--dry-run", action="store_true", help="show what would change without writing")

    args = parser.parse_args()

    if not args.input_dir.is_dir():
        print(f"error: {args.input_dir} is not a directory", file=sys.stderr)
        return 2

    files = collect_room_files(args.input_dir, args.recursive)
    if not files:
        print(f"no .room files found in {args.input_dir}", file=sys.stderr)
        return 1

    if args.in_place:
        out_dir = args.input_dir
    else:
        out_dir = args.out or args.input_dir.with_name(args.input_dir.name + "_scaled")

    rng = random.Random(args.seed)

    print(f"input         : {args.input_dir}")
    print(f"output        : {out_dir}{' (IN-PLACE, .bak backups)' if args.in_place else ''}")
    print(f"enemy mult    : x{args.enemy_mult}")
    print(f"resource mult : x{args.resource_mult}")
    print(f"walkable tile : {args.walkable_tile}")
    print(f"files         : {len(files)}")
    print(f"dry-run       : {args.dry_run}")
    print("-" * 78)

    total_enemy_before = total_enemy_after = 0
    total_res_before   = total_res_after   = 0
    failures = []

    for in_path in files:
        rel = in_path.relative_to(args.input_dir)
        out_path = out_dir / rel

        try:
            if args.in_place and not args.dry_run:
                bak = in_path.with_suffix(in_path.suffix + ".bak")
                if not bak.exists():
                    shutil.copy2(in_path, bak)

            if args.dry_run:
                data = in_path.read_bytes()
                raw_room, _, enemies, resources = parse_room(data)
                walkable = collect_walkable_cells(raw_room, args.walkable_tile)
                new_enemies   = scale_spawners(enemies,   args.enemy_mult,    walkable, rng)
                new_resources = scale_spawners(resources, args.resource_mult, walkable, rng)
                stat = {
                    "file": in_path.name,
                    "enemies":   (len(enemies),   len(new_enemies)),
                    "resources": (len(resources), len(new_resources)),
                    "walkable":  len(walkable),
                }
            else:
                stat = process_file(in_path, out_path, args, rng)

            eb, ea = stat["enemies"]
            rb, ra = stat["resources"]
            total_enemy_before += eb; total_enemy_after += ea
            total_res_before   += rb; total_res_after   += ra

            print(f"  {stat['file']:<24} enemies {eb:>3} -> {ea:>3}  resources {rb:>3} -> {ra:>3}  (walkable cells: {stat['walkable']:>3})")

        except Exception as e:
            failures.append((in_path.name, str(e)))
            print(f"  {in_path.name:<24} FAILED: {e}", file=sys.stderr)

    print("-" * 70)
    print(f"TOTAL enemies   : {total_enemy_before} -> {total_enemy_after}")
    print(f"TOTAL resources : {total_res_before} -> {total_res_after}")
    if failures:
        print(f"FAILURES        : {len(failures)}")
        for name, err in failures:
            print(f"  - {name}: {err}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
