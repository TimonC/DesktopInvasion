import argparse
import os
import json
from PIL import Image
import numpy as np
BIG_IDS = [
    208,
    249,
    250,
    483,
    484,
    486,
    487,
    493,
    321,
    382,
    383,
    384,
]
GEN_LIMS = [151, 100, 135, 107]
def calculate_weighted_hcenter(frame, threshold=10, density_threshold=0.5):
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')
    arr = np.array(frame)
    alpha = arr[:, :, 3]
    mask = alpha > threshold
    if not np.any(mask):
        return None
    rows = np.any(mask, axis=1)
    cols = np.any(mask, axis=0)
    y_min, y_max = np.where(rows)[0][[0, -1]]
    x_min, x_max = np.where(cols)[0][[0, -1]]

    cropped_alpha = alpha[y_min:y_max+1, x_min:x_max+1]
    cropped_mask = mask[y_min:y_max+1, x_min:x_max+1]

    bbox_width = x_max - x_min + 1
    if bbox_width == 0:
        return 0.5

    col_densities = np.sum(cropped_alpha * cropped_mask, axis=0)

    x_coords = np.arange(bbox_width)
    total_mass = np.sum(col_densities)
    if total_mass == 0:
        return 0.5
    weighted_center = np.sum(x_coords * col_densities) / total_mass

    return weighted_center / bbox_width
def analyze_frame_bounds(frame):
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
    vertical_width, vertical_height = 0, 0
    horizontal_width, horizontal_height = 0, 0
    vertical_indices = [0, 1, 4, 5]
    for idx in vertical_indices:
        width, height, _, _ = analyze_frame_bounds(frames[idx])
        if width > 0 and height > 0:
            vertical_width = max(vertical_width, width)
            vertical_height = max(vertical_height, height)
    horizontal_indices = [2, 3, 6, 7]
    for idx in horizontal_indices:
        width, height, _, _ = analyze_frame_bounds(frames[idx])
        if width > 0 and height > 0:
            horizontal_width = max(horizontal_width, width)
            horizontal_height = max(horizontal_height, height)
    return vertical_width, vertical_height, horizontal_width, horizontal_height
def calculate_horizontal_centers(frames, v_width):
    up_centers = []
    down_centers = []
    for idx in [0, 1]:
        center = calculate_weighted_hcenter(frames[idx])
        if center is not None and v_width > 0:
            up_centers.append(int(round(center * v_width)))
    for idx in [4, 5]:
        center = calculate_weighted_hcenter(frames[idx])
        if center is not None and v_width > 0:
            down_centers.append(int(round(center * v_width)))
    h_center_up = int(round(np.mean(up_centers))) if up_centers else v_width // 2
    h_center_down = int(round(np.mean(down_centers))) if down_centers else v_width // 2
    return h_center_up, h_center_down
def has_visible_content(frames):
    for frame in frames:
        width, height, _, _ = analyze_frame_bounds(frame)
        if width > 0 and height > 0:
            return True
    return False
def extract_frame_block(img, col_idx, row_idx, frame_width, frame_height):
    x0 = col_idx * (2 * frame_width + 1)
    y0 = row_idx * (4 * frame_height + 1)
    return [
        img.crop((x0, y0, x0 + frame_width, y0 + frame_height)),
        img.crop((x0, y0 + frame_height, x0 + frame_width, y0 + 2*frame_height)),
        img.crop((x0 + frame_width, y0, x0 + 2*frame_width, y0 + frame_height)),
        img.crop((x0 + frame_width, y0 + frame_height, x0 + 2*frame_width, y0 + 2*frame_height)),
        img.crop((x0, y0 + 2*frame_height, x0 + frame_width, y0 + 3*frame_height)),
        img.crop((x0, y0 + 3*frame_height, x0 + frame_width, y0 + 4*frame_height)),
        img.crop((x0 + frame_width, y0 + 2*frame_height, x0 + 2*frame_width, y0 + 3*frame_height)),
        img.crop((x0 + frame_width, y0 + 3*frame_height, x0 + 2*frame_width, y0 + 4*frame_height))
    ]
def extract_big_frames(img, pokemon_idx, n_cols, frame_width, frame_height):
    row_idx = pokemon_idx * 2
    frames = []
    for r in range(2):
        for c in range(n_cols):
            x0 = c * frame_width
            y0 = (row_idx + r) * frame_height
            frames.append(img.crop((x0, y0, x0 + frame_width, y0 + frame_height)))
    aa, ab, ac, ad, ba, bb, bc, bd = frames
    return [aa, ba, ac, bc, ab, bb, ad, bd]
def center_sprite_in_frame(frame, target_width, target_height):
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
    cpp_content = """#include "data_poke_asset.h"
namespace {
"""
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
    cpp_content += "\n} \n\n"
    cpp_content += "const AssetInfo* const kAssetInfo[493] = {\n"
    for i in range(1, 494):
        if i in asset_data and asset_data[i] is not None:
            cpp_content += f"    &asset_{i},"
            if asset_data[i]["is_big"]:
                cpp_content += " "
            cpp_content += "\n"
        else:
            cpp_content += f"    nullptr,\n"
    cpp_content += "};\n"
    os.makedirs(os.path.dirname(output_cpp_path), exist_ok=True)
    with open(output_cpp_path, 'w') as f:
        f.write(cpp_content)
    print(f"Generated C++ asset file at {output_cpp_path}")
    print(f"Total Pokémon with asset data: {len(asset_entries)}")
    missing = [i for i in range(1, 494) if i not in asset_data or asset_data[i] is None]
    if missing:
        print(f"Missing Pokémon IDs: {missing}")
def process_sprites_and_generate_assets(image_paths, big_image_path, regular_width, regular_height, big_width, big_height, n_rows, n_cols, output_dir, cpp_output_path):
    asset_data = {i: None for i in range(1, 494)}
    regular_rows = []
    big_rows = []
    current_pokedex_id = 1
    for sheet_idx, image_path in enumerate(image_paths):
        print(f"Processing sheet {sheet_idx+1}: {os.path.basename(image_path)}")
        img = Image.open(image_path)
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
        current_pokedex_id = gen_start
        sprites_in_sheet = 0
        for row in range(n_rows):
            for col in range(n_cols):
                if current_pokedex_id > gen_end:
                    break
                if current_pokedex_id in BIG_IDS:
                    current_pokedex_id += 1
                    continue
                frames = extract_frame_block(img, col, row, regular_width, regular_height)
                if has_visible_content(frames):
                    v_width, v_height, h_width, h_height = calculate_sprite_bounds(frames)
                    h_center_up, h_center_down = calculate_horizontal_centers(frames, v_width)
                    centered_frames = [center_sprite_in_frame(f, regular_width, regular_height) for f in frames]
                    row_id = len(regular_rows)
                    regular_rows.append((current_pokedex_id, centered_frames))
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
                    pass
        print(f"  Found {sprites_in_sheet} sprites in this sheet")
    print(f"\nProcessing big sheet: {os.path.basename(big_image_path)}")
    img = Image.open(big_image_path)
    for idx in range(13):
        if idx >= len(BIG_IDS):
            break
        pokedex_id = BIG_IDS[idx]
        frames = extract_big_frames(img, idx, 4, big_width, big_height)
        if has_visible_content(frames):
            v_width, v_height, h_width, h_height = calculate_sprite_bounds(frames)
            h_center_up, h_center_down = calculate_horizontal_centers(frames, v_width)
            centered_frames = [center_sprite_in_frame(f, big_width, big_height) for f in frames]
            row_id = len(big_rows)
            big_rows.append((pokedex_id, centered_frames))
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
    print(f"\nCreating output images...")
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
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)
    if not os.path.isabs(args.output_dir):
        args.output_dir = os.path.join(project_root, args.output_dir)
    if not os.path.isabs(args.cpp_output):
        args.cpp_output = os.path.join(project_root, args.cpp_output)
    os.makedirs(args.output_dir, exist_ok=True)
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
    print("\nSample asset data (first 10 Pokémon):")

    pid = 336
    if asset_data[pid]:
        info = asset_data[pid]
        sheet = "Big" if info["is_big"] else "Standard"
        print(f" Seviper ---  #{pid}: width={info['width']}, height={info['height']}, V={info['v_width']}x{info['v_height']}, H={info['h_width']}x{info['h_height']}, centers: up={info['h_center_up']}, down={info['h_center_down']}, {sheet}, row {info['row_id']}")

if __name__ == "__main__":
    main()
