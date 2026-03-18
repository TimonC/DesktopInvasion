import argparse
import os
import json
from PIL import Image
import numpy as np

# Big Pokémon IDs
BIG_IDS = [
    208,  # Steelix
    249,  # Lugia
    250,  # Ho-Oh
    483,  # Dialga
    484,  # Palkia
    486,  # Regigigas
    487,  # Giratina
    493,  # Arceus
    321,  # Wailord
    382,  # Kyogre
    383,  # Groudon
    384,  # Rayquaza
]

GEN_LIMS = [151, 100, 135, 107]  # Gen1: 1-151, Gen2: 152-251, Gen3: 252-386, Gen4: 387-493

def calculate_weighted_hcenter(frame, threshold=10):
    """Calculate weighted horizontal center of mass for non-transparent pixels."""
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    arr = np.array(frame)
    alpha = arr[:, :, 3]

    # Create mask of non-transparent pixels
    mask = alpha > threshold

    if not np.any(mask):
        return None  # No visible content

    # Get bounding box to find sprite offset within frame
    rows = np.any(mask, axis=1)
    cols = np.any(mask, axis=0)

    if not np.any(rows) or not np.any(cols):
        return None

    y_min, y_max = np.where(rows)[0][[0, -1]]
    x_min, x_max = np.where(cols)[0][[0, -1]]

    # Calculate weighted center using alpha values as weights
    # We only consider pixels within the bounding box for efficiency
    cropped_alpha = alpha[y_min:y_max+1, x_min:x_max+1]
    cropped_mask = cropped_alpha > threshold

    if not np.any(cropped_mask):
        return None

    # Get x positions relative to bounding box
    y_indices, x_indices = np.where(cropped_mask)
    weights = cropped_alpha[y_indices, x_indices]

    # Calculate weighted average x position within bounding box
    weighted_x_sum = np.sum(x_indices.astype(np.float64) * weights)
    total_weight = np.sum(weights)

    if total_weight == 0:
        return None

    # Center relative to bounding box (0 = left edge, width-1 = right edge)
    center_in_bbox = weighted_x_sum / total_weight

    # Convert to position relative to sprite width
    bbox_width = x_max - x_min + 1
    center_relative = center_in_bbox / bbox_width if bbox_width > 0 else 0.5

    return center_relative

def analyze_frame_bounds(frame):
    """Analyze frame bounds and return width, height, x_min, y_min of non-transparent content"""
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    arr = np.array(frame)
    alpha = arr[:, :, 3]
    non_transparent = alpha > 10

    if not np.any(non_transparent):
        return 0, 0, 0, 0

    rows = np.any(non_transparent, axis=1)
    cols = np.any(non_transparent, axis=0)

    y_min, y_max = np.where(rows)[0][[0, -1]]
    x_min, x_max = np.where(cols)[0][[0, -1]]

    width = x_max - x_min + 1
    height = y_max - y_min + 1

    return int(width), int(height), int(x_min), int(y_min)

def calculate_sprite_bounds(frames):
    """Calculate bounds for vertical and horizontal frames separately"""
    # Frame order: 2 vertical, 2 horizontal, 2 vertical, 2 horizontal
    # Indices: 0,1 = vertical; 2,3 = horizontal; 4,5 = vertical; 6,7 = horizontal

    vertical_width, vertical_height = 0, 0
    horizontal_width, horizontal_height = 0, 0

    # Process vertical frames (indices 0, 1, 4, 5)
    vertical_indices = [0, 1, 4, 5]
    for idx in vertical_indices:
        width, height, _, _ = analyze_frame_bounds(frames[idx])
        if width > 0 and height > 0:
            vertical_width = max(vertical_width, width)
            vertical_height = max(vertical_height, height)

    # Process horizontal frames (indices 2, 3, 6, 7)
    horizontal_indices = [2, 3, 6, 7]
    for idx in horizontal_indices:
        width, height, _, _ = analyze_frame_bounds(frames[idx])
        if width > 0 and height > 0:
            horizontal_width = max(horizontal_width, width)
            horizontal_height = max(horizontal_height, height)

    return vertical_width, vertical_height, horizontal_width, horizontal_height

def calculate_horizontal_centers(frames, v_width):
    """Calculate horizontal centers for up and down frames."""
    # Frames: 0,1 = up (vertical), 4,5 = down (vertical)

    up_centers = []
    down_centers = []

    # Calculate for up frames (0, 1)
    for idx in [0, 1]:
        center = calculate_weighted_hcenter(frames[idx])
        if center is not None and v_width > 0:
            # Convert from relative (0-1) to actual pixel position within sprite
            up_centers.append(int(round(center * v_width)))

    # Calculate for down frames (4, 5)
    for idx in [4, 5]:
        center = calculate_weighted_hcenter(frames[idx])
        if center is not None and v_width > 0:
            down_centers.append(int(round(center * v_width)))

    # Average the centers for each direction
    h_center_up = int(round(np.mean(up_centers))) if up_centers else v_width // 2
    h_center_down = int(round(np.mean(down_centers))) if down_centers else v_width // 2

    return h_center_up, h_center_down

def has_visible_content(frames):
    """Check if any frame has visible content"""
    for frame in frames:
        width, height, _, _ = analyze_frame_bounds(frame)
        if width > 0 and height > 0:
            return True
    return False

def extract_frame_block(img, col_idx, row_idx, frame_width, frame_height):
    """Extract 8 frames from a block"""
    x0 = col_idx * (2 * frame_width + 1)
    y0 = row_idx * (4 * frame_height + 1)

    return [
        img.crop((x0, y0, x0 + frame_width, y0 + frame_height)),  # V1
        img.crop((x0, y0 + frame_height, x0 + frame_width, y0 + 2*frame_height)),  # V2
        img.crop((x0 + frame_width, y0, x0 + 2*frame_width, y0 + frame_height)),  # H1
        img.crop((x0 + frame_width, y0 + frame_height, x0 + 2*frame_width, y0 + 2*frame_height)),  # H2
        img.crop((x0, y0 + 2*frame_height, x0 + frame_width, y0 + 3*frame_height)),  # V3
        img.crop((x0, y0 + 3*frame_height, x0 + frame_width, y0 + 4*frame_height)),  # V4
        img.crop((x0 + frame_width, y0 + 2*frame_height, x0 + 2*frame_width, y0 + 3*frame_height)),  # H3
        img.crop((x0 + frame_width, y0 + 3*frame_height, x0 + 2*frame_width, y0 + 4*frame_height))  # H4
    ]

def extract_big_frames(img, pokemon_idx, n_cols, frame_width, frame_height):
    """Extract 8 frames for big Pokémon"""
    row_idx = pokemon_idx * 2
    frames = []

    for r in range(2):
        for c in range(n_cols):
            x0 = c * frame_width
            y0 = (row_idx + r) * frame_height
            frames.append(img.crop((x0, y0, x0 + frame_width, y0 + frame_height)))

    # Reorder: aa, ba, ac, bc, ab, bb, ad, bd
    # Based on pattern: vertical, vertical, horizontal, horizontal, vertical, vertical, horizontal, horizontal
    aa, ab, ac, ad, ba, bb, bc, bd = frames
    return [aa, ba, ac, bc, ab, bb, ad, bd]

def center_sprite_in_frame(frame, target_width, target_height):
    """Center sprite content within frame"""
    width, height, x_min, y_min = analyze_frame_bounds(frame)

    if width == 0 or height == 0:
        return Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))

    center_x = (target_width - width) // 2
    center_y = (target_height - height) // 2

    new_frame = Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))
    sprite_content = frame.crop((x_min, y_min, x_min + width, y_min + height))
    new_frame.paste(sprite_content, (center_x, center_y), sprite_content)

    return new_frame

def generate_cpp_asset_file(asset_data, output_cpp_path):
    """Generate C++ asset file matching the header"""

    cpp_content = """#include "data_poke_asset.h"

// Asset information for all 493 Pokémon
// This file is auto-generated by preprocess_sprites.py

namespace {

// Static storage for AssetInfo objects
"""

    # Create static AssetInfo objects for each Pokémon we found
    asset_entries = []

    for pid, info in asset_data.items():
        if info is None:
            continue

        sprite_sheet = "SpriteSheet::Big" if info["is_big"] else "SpriteSheet::Standard"
        cpp_content += f"static const AssetInfo asset_{pid} = "
        cpp_content += f"{{{info['width']}, {info['height']}, {info['h_width']}, {info['h_height']}, "
        cpp_content += f"{info['v_width']}, {info['v_height']}, "
        cpp_content += f"{info['h_center_up']}, {info['h_center_down']}, "
        cpp_content += f"{sprite_sheet}, {info['row_id']}}};\n"
        asset_entries.append(pid)

    cpp_content += "\n} // anonymous namespace\n\n"

    # Now create the main array
    cpp_content += "// Main array of pointers to AssetInfo for Pokémon 1-493\n"
    cpp_content += "// nullptr entries indicate missing Pokémon\n"
    cpp_content += "const AssetInfo* const kAssetInfo[493] = {\n"

    # Create array entries for Pokémon 1-493
    for i in range(1, 494):
        if i in asset_data and asset_data[i] is not None:
            cpp_content += f"    &asset_{i},  // #{i}"
            if asset_data[i]["is_big"]:
                cpp_content += " (big)"
            cpp_content += "\n"
        else:
            cpp_content += f"    nullptr,  // #{i} (missing)\n"

    cpp_content += "};\n"

    # Write the file
    os.makedirs(os.path.dirname(output_cpp_path), exist_ok=True)
    with open(output_cpp_path, 'w') as f:
        f.write(cpp_content)

    print(f"Generated C++ asset file at {output_cpp_path}")
    print(f"Total Pokémon with asset data: {len(asset_entries)}")

    # Print missing Pokémon
    missing = [i for i in range(1, 494) if i not in asset_data or asset_data[i] is None]
    if missing:
        print(f"Missing Pokémon IDs: {missing}")

def process_sprites_and_generate_assets(image_paths, big_image_path, regular_width, regular_height, big_width, big_height, n_rows, n_cols, output_dir, cpp_output_path):
    """Process sprites and generate C++ asset file"""

    # Asset data for all 493 Pokémon
    asset_data = {i: None for i in range(1, 494)}  # Initialize all to None

    # Track rows for output sheets
    regular_rows = []  # List of (pokedex_id, frames)
    big_rows = []      # List of (pokedex_id, frames)

    # Process regular sheets with generation tracking
    current_pokedex_id = 1

    for sheet_idx, image_path in enumerate(image_paths):
        print(f"Processing sheet {sheet_idx+1}: {os.path.basename(image_path)}")
        img = Image.open(image_path)

        # Determine generation bounds
        if sheet_idx == 0:
            gen_start, gen_end = 1, 151
        elif sheet_idx == 1:
            gen_start, gen_end = 152, 251
        elif sheet_idx == 2:
            gen_start, gen_end = 252, 386
        elif sheet_idx == 3:
            gen_start, gen_end = 387, 493
        else:
            break

        # Reset to start of this generation
        current_pokedex_id = gen_start

        sprites_in_sheet = 0

        for row in range(n_rows):
            for col in range(n_cols):
                if current_pokedex_id > gen_end:
                    break

                # Check if this is a big Pokémon
                if current_pokedex_id in BIG_IDS:
                    # Big Pokémon - skip in regular sheet
                    current_pokedex_id += 1
                    # Skip this position (don't extract)
                    continue

                frames = extract_frame_block(img, col, row, regular_width, regular_height)

                if has_visible_content(frames):
                    # Calculate vertical and horizontal bounds separately
                    v_width, v_height, h_width, h_height = calculate_sprite_bounds(frames)

                    # Calculate horizontal centers for up and down frames
                    h_center_up, h_center_down = calculate_horizontal_centers(frames, v_width)

                    # Center frames
                    centered_frames = [center_sprite_in_frame(f, regular_width, regular_height) for f in frames]

                    # Add to regular rows
                    row_id = len(regular_rows)
                    regular_rows.append((current_pokedex_id, centered_frames))

                    # Store asset data
                    asset_data[current_pokedex_id] = {
                        "width": max(v_width, h_width),
                        "height": max(v_height, h_height),
                        "h_width": h_width,
                        "h_height": h_height,
                        "v_width": v_width,
                        "v_height": v_height,
                        "h_center_up": h_center_up,
                        "h_center_down": h_center_down,
                        "is_big": False,
                        "row_id": row_id
                    }

                    sprites_in_sheet += 1
                    current_pokedex_id += 1
                else:
                    # Empty block - skip without incrementing
                    # (This assumes empty blocks are just padding)
                    pass

        print(f"  Found {sprites_in_sheet} sprites in this sheet")

    # Process big sheet
    print(f"\nProcessing big sheet: {os.path.basename(big_image_path)}")
    img = Image.open(big_image_path)

    # Big sheet has 13 slots
    for idx in range(13):
        if idx >= len(BIG_IDS):
            break

        pokedex_id = BIG_IDS[idx]

        frames = extract_big_frames(img, idx, 4, big_width, big_height)

        if has_visible_content(frames):
            # Calculate vertical and horizontal bounds separately
            v_width, v_height, h_width, h_height = calculate_sprite_bounds(frames)

            # Calculate horizontal centers for up and down frames
            h_center_up, h_center_down = calculate_horizontal_centers(frames, v_width)

            # Center frames
            centered_frames = [center_sprite_in_frame(f, big_width, big_height) for f in frames]

            # Add to big rows
            row_id = len(big_rows)
            big_rows.append((pokedex_id, centered_frames))

            # Store asset data
            asset_data[pokedex_id] = {
                "width": max(v_width, h_width),
                "height": max(v_height, h_height),
                "h_width": h_width,
                "h_height": h_height,
                "v_width": v_width,
                "v_height": v_height,
                "h_center_up": h_center_up,
                "h_center_down": h_center_down,
                "is_big": True,
                "row_id": row_id
            }

            print(f"  Processed big Pokémon #{pokedex_id}: V={v_width}x{v_height}, H={h_width}x{h_height}, centers: up={h_center_up}, down={h_center_down}")
        else:
            print(f"  WARNING: Big Pokémon #{pokedex_id} has no visible content!")

    # Create output images
    print(f"\nCreating output images...")

    # Regular sprites output
    if regular_rows:
        output_img = Image.new("RGBA", (8 * regular_width, len(regular_rows) * regular_height))
        for row_idx, (pokedex_id, frames) in enumerate(regular_rows):
            for col_idx, frame in enumerate(frames):
                x = col_idx * regular_width
                y = row_idx * regular_height
                output_img.paste(frame, (x, y))

        regular_output = os.path.join(output_dir, "reordered_sprites.png")
        output_img.save(regular_output)
        print(f"Saved {len(regular_rows)} regular sprites to {regular_output}")

    # Big sprites output
    if big_rows:
        output_img = Image.new("RGBA", (8 * big_width, len(big_rows) * big_height))
        for row_idx, (pokedex_id, frames) in enumerate(big_rows):
            for col_idx, frame in enumerate(frames):
                x = col_idx * big_width
                y = row_idx * big_height
                output_img.paste(frame, (x, y))

        big_output = os.path.join(output_dir, "reordered_sprites_big.png")
        output_img.save(big_output)
        print(f"Saved {len(big_rows)} big sprites to {big_output}")

    # Generate C++ file
    generate_cpp_asset_file(asset_data, cpp_output_path)

    return len(regular_rows), len(big_rows), asset_data

def main():
    parser = argparse.ArgumentParser(description="Process Pokémon sprites and generate C++ asset file")
    parser.add_argument("--input_path", default="assets/HGSS")
    parser.add_argument("--regular_inputs", nargs='+',
                       default=["PokGen1_transparent.png", "PokGen2_transparent.png",
                               "PokGen3_transparent.png", "PokGen4_transparent.png"])
    parser.add_argument("--big_input", default="bigboys.png")
    parser.add_argument("--regular_width", type=int, default=32)
    parser.add_argument("--regular_height", type=int, default=32)
    parser.add_argument("--big_width", type=int, default=64)
    parser.add_argument("--big_height", type=int, default=64)
    parser.add_argument("--output_dir", default="assets/HGSS")
    parser.add_argument("--cpp_output", default="src/data_poke_asset.cpp")
    parser.add_argument("--n_rows", type=int, default=20, help="Number of rows in each sprite sheet")
    parser.add_argument("--n_cols", type=int, default=15, help="Number of columns in each sprite sheet")

    args = parser.parse_args()

    # Make paths absolute
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)
    if not os.path.isabs(args.output_dir):
        args.output_dir = os.path.join(project_root, args.output_dir)
    if not os.path.isabs(args.cpp_output):
        args.cpp_output = os.path.join(project_root, args.cpp_output)

    os.makedirs(args.output_dir, exist_ok=True)

    # Process sprites
    regular_paths = [os.path.join(args.input_path, f) for f in args.regular_inputs]
    big_path = os.path.join(args.input_path, args.big_input)

    print("=" * 60)
    print(f"Processing sprites with {args.n_rows} rows, {args.n_cols} columns per sheet")
    print("=" * 60)

    regular_count, big_count, asset_data = process_sprites_and_generate_assets(
        regular_paths, big_path,
        args.regular_width, args.regular_height,
        args.big_width, args.big_height,
        args.n_rows, args.n_cols,
        args.output_dir,
        args.cpp_output
    )

    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"Regular sprites: {regular_count}")
    print(f"Big sprites: {big_count}")
    print(f"Total: {regular_count + big_count}")
    print(f"Expected: 493")
    print(f"Missing: {493 - (regular_count + big_count)}")

    # Print some sample asset data
    print("\nSample asset data (first 10 Pokémon):")
    for pid in range(1, 11):
        if asset_data[pid]:
            info = asset_data[pid]
            sheet = "Big" if info["is_big"] else "Standard"
            print(f"  #{pid}: width={info['width']}, height={info['height']}, V={info['v_width']}x{info['v_height']}, H={info['h_width']}x{info['h_height']}, centers: up={info['h_center_up']}, down={info['h_center_down']}, {sheet}, row {info['row_id']}")

if __name__ == "__main__":
    main()
