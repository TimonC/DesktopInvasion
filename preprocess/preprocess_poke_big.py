import argparse
import os
import json
from pathlib import Path
from PIL import Image
import numpy as np

def has_visible_content(frames):
    for frame in frames:
        if frame.mode != 'RGBA':
            frame = frame.convert('RGBA')
        arr = np.array(frame)
        if np.any(arr[:, :, 3] > 0):
            return True
    return False

def analyze_frame_bounds(frame):
    """Analyze frame bounds and return width, height, x_min, y_min, x_max, y_max"""
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    arr = np.array(frame)
    alpha = arr[:, :, 3]

    non_transparent = alpha > 10

    if not np.any(non_transparent):
        return 0, 0, 0, 0, None, None

    rows = np.any(non_transparent, axis=1)
    cols = np.any(non_transparent, axis=0)

    y_min, y_max = np.where(rows)[0][[0, -1]]
    x_min, x_max = np.where(cols)[0][[0, -1]]

    width = x_max - x_min + 1
    height = y_max - y_min + 1

    return int(width), int(height), int(x_min), int(y_min), int(x_max), int(y_max)

def calculate_sprite_bounds(frames):
    min_x, min_y = float('inf'), float('inf')
    max_x, max_y = float('-inf'), float('-inf')

    # Store individual frame bounds for per-frame centering
    frame_bounds = []

    for frame in frames:
        width, height, x_min, y_min, x_max, y_max = analyze_frame_bounds(frame)
        frame_bounds.append((width, height, x_min, y_min, x_max, y_max))

        if width == 0 or height == 0:
            continue

        # Only update bounds if we have valid x_max/y_max values
        if x_max is not None and y_max is not None:
            min_x = min(min_x, x_min) if x_min is not None else min_x
            min_y = min(min_y, y_min) if y_min is not None else min_y
            max_x = max(max_x, x_max) if x_max is not None else max_x
            max_y = max(max_y, y_max) if y_max is not None else max_y

    if min_x == float('inf'):
        return 0, 0, 0, 0, frame_bounds

    union_width = max_x - min_x
    union_height = max_y - min_y

    return union_width, union_height, min_x, min_y, frame_bounds

def center_sprite_in_frame(frame, target_width=64, target_height=64):
    """Center a sprite within a new frame by measuring its content bounds"""
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    # Get the actual image data
    img_array = np.array(frame)

    # Find non-transparent pixels
    alpha = img_array[:, :, 3]
    non_transparent = alpha > 10

    if not np.any(non_transparent):
        # Empty frame, return transparent 64x64
        return Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))

    # Get bounds of non-transparent content
    rows = np.any(non_transparent, axis=1)
    cols = np.any(non_transparent, axis=0)

    y_min, y_max = np.where(rows)[0][[0, -1]]
    x_min, x_max = np.where(cols)[0][[0, -1]]

    content_width = x_max - x_min + 1
    content_height = y_max - y_min + 1

    # Calculate center position
    center_x = (target_width - content_width) // 2
    center_y = (target_height - content_height) // 2

    # Create new transparent frame
    new_frame = Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))

    # Crop the sprite content (only the non-transparent part)
    sprite_content = frame.crop((x_min, y_min, x_max + 1, y_max + 1))

    # Paste centered in new frame
    new_frame.paste(sprite_content, (center_x, center_y), sprite_content)

    return new_frame

def extract_pokemon_frames(img, pokemon_idx, n_cols, frame_width, frame_height):
    """Extract 8 frames for a single Pokémon from 2 rows x 4 cols layout

    Layout:
    aa ab ac ad
    ba bb bc bd

    Output order: aa, ba, ac, bc, ab, bb, ad, bd
    """
    row_idx = pokemon_idx * 2

    frames_2x4 = []
    for r in range(2):
        for c in range(n_cols):
            x0 = c * frame_width
            y0 = (row_idx + r) * frame_height
            frame = img.crop((x0, y0, x0 + frame_width, y0 + frame_height))
            frames_2x4.append(frame)

    # frames_2x4 is now: [aa, ab, ac, ad, ba, bb, bc, bd]
    # Reorder to: aa, ba, ac, bc, ab, bb, ad, bd
    aa, ab, ac, ad, ba, bb, bc, bd = frames_2x4
    reordered = [aa, ba, ac, bc, ab, bb, ad, bd]

    return reordered

def process_image(image_path, n_pokemon, n_cols, frame_width, frame_height):
    img = Image.open(image_path)
    print(f"Processing {image_path}...")

    all_rows = []
    metadata = {
        "frame_width": frame_width,
        "frame_height": frame_height,
        "rows": []
    }

    for pokemon_idx in range(n_pokemon):
        frames = extract_pokemon_frames(img, pokemon_idx, n_cols, frame_width, frame_height)

        if has_visible_content(frames):
            # Center each frame individually
            centered_frames = []
            frame_measurements = []

            for i, frame in enumerate(frames):
                # Get bounds before centering for metadata
                width, height, x_min, y_min, x_max, y_max = analyze_frame_bounds(frame)

                # Center this frame
                centered_frame = center_sprite_in_frame(frame, frame_width, frame_height)
                centered_frames.append(centered_frame)

                # Get bounds after centering for verification
                c_width, c_height, c_x_min, c_y_min, c_x_max, c_y_max = analyze_frame_bounds(centered_frame)

                frame_measurements.append({
                    "original": {"width": width, "height": height, "x_min": x_min, "y_min": y_min},
                    "centered": {"width": c_width, "height": c_height, "x_min": c_x_min, "y_min": c_y_min}
                })

            # Calculate union bounds of centered frames
            union_width, union_height, union_x, union_y, _ = calculate_sprite_bounds(centered_frames)

            metadata["rows"].append({
                "pokemon_index": pokemon_idx,
                "union_width": union_width,
                "union_height": union_height,
                "union_x": union_x,
                "union_y": union_y,
                "frame_measurements": frame_measurements
            })

            all_rows.append(centered_frames)

    if not all_rows:
        print("No visible sprites found!")
        return

    new_width = 8 * frame_width
    new_height = len(all_rows) * frame_height
    new_img = Image.new("RGBA", (new_width, new_height))

    for idx, row_frames in enumerate(all_rows):
        for j, frame in enumerate(row_frames):
            new_img.paste(frame, (j * frame_width, idx * frame_height))

    input_path = Path(image_path)
    output_path = input_path.with_name(input_path.stem + "_reordered.png")
    new_img.save(output_path)

    metadata_path = input_path.with_name(input_path.stem + "_reordered_metadata.json")
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)

    print(f"Saved to {output_path}")
    print(f"Saved metadata to {metadata_path}")
    print(f"Processed {len(all_rows)} sprite rows")

def preprocess_poke(image_paths, n_pokemon, n_cols, frame_width, frame_height):
    for image_path in image_paths:
        process_image(image_path, n_pokemon, n_cols, frame_width, frame_height)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    parser = argparse.ArgumentParser(
        description="Reorder frames from Pokémon sprite sheets (2-row layout), skipping empty sprites and generating metadata."
    )
    parser.add_argument("--input_path", type=str, default="assets/HGSS")
    parser.add_argument("--inputs", nargs='+', default=["bigboys.png"])
    parser.add_argument("--n_pokemon", type=int, default=13, help="Number of Pokémon in the sprite sheet")
    parser.add_argument("--n_cols", type=int, default=4, help="Number of columns per row (always 4 for 2-row layout)")
    parser.add_argument("--frame_width", type=int, default=64)
    parser.add_argument("--frame_height", type=int, default=64)

    args = parser.parse_args()

    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)

    input_paths = [os.path.join(args.input_path, input) for input in args.inputs]
    preprocess_poke(
        input_paths, args.n_pokemon, args.n_cols, args.frame_width, args.frame_height
    )
