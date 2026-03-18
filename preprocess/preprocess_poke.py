import argparse
import os
import json
from pathlib import Path
from PIL import Image
import numpy as np

from sprite_exceptions import FORM_IDS_AND_NRS, BIG_IDS

def has_visible_content(frames):
    for frame in frames:
        if frame.mode != 'RGBA':
            frame = frame.convert('RGBA')
        arr = np.array(frame)
        if np.any(arr[:, :, 3] > 0):
            return True
    return False

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
    min_x, min_y = float('inf'), float('inf')
    max_x, max_y = float('-inf'), float('-inf')

    for frame in frames:
        width, height, x_min, y_min = analyze_frame_bounds(frame)
        if width == 0 or height == 0:
            continue

        x_max = x_min + width
        y_max = y_min + height

        min_x = min(min_x, x_min)
        min_y = min(min_y, y_min)
        max_x = max(max_x, x_max)
        max_y = max(max_y, y_max)

    if min_x == float('inf'):
        return 0, 0, 0, 0

    union_width = max_x - min_x
    union_height = max_y - min_y

    return union_width, union_height, min_x, min_y

def calculate_center_offset(frame_width, frame_height, content_width, content_height, content_x, content_y):
    extra_width = frame_width - content_width
    extra_height = frame_height - content_height

    desired_x = extra_width // 2
    desired_y = extra_height // 2

    offset_x = desired_x - content_x
    offset_y = desired_y - content_y

    return offset_x, offset_y

def apply_offset_to_frame(frame, offset_x, offset_y):
    if offset_x == 0 and offset_y == 0:
        return frame

    new_frame = Image.new('RGBA', frame.size)
    new_frame.paste(frame, (offset_x, offset_y), frame)

    return new_frame

def extract_frame_block(img, col_idx, row_idx, frame_width, frame_height):
    x0 = col_idx * (2 * frame_width + 1)
    y0 = row_idx * (4 * frame_height + 1)

    aa = img.crop((x0, y0, x0 + frame_width, y0 + frame_height))
    ab = img.crop((x0 + frame_width, y0, x0 + 2*frame_width, y0 + frame_height))

    ba = img.crop((x0, y0 + frame_height, x0 + frame_width, y0 + 2*frame_height))
    bb = img.crop((x0 + frame_width, y0 + frame_height, x0 + 2*frame_width, y0 + 2*frame_height))

    ca = img.crop((x0, y0 + 2*frame_height, x0 + frame_width, y0 + 3*frame_height))
    cb = img.crop((x0 + frame_width, y0 + 2*frame_height, x0 + 2*frame_width, y0 + 3*frame_height))

    da = img.crop((x0, y0 + 3*frame_height, x0 + frame_width, y0 + 4*frame_height))
    db = img.crop((x0 + frame_width, y0 + 3*frame_height, x0 + 2*frame_width, y0 + 4*frame_height))

    return [aa, ba, ab, bb, ca, da, cb, db]

def process_image(image_path, n_rows, n_cols, frame_width, frame_height):
    img = Image.open(image_path)
    print(f"Processing {image_path}...")

    all_rows = []
    metadata = {
        "frame_width": frame_width,
        "frame_height": frame_height,
        "rows": []
    }

    for row_idx in range(n_rows):
        for col_idx in range(n_cols):
            frames = extract_frame_block(img, col_idx, row_idx, frame_width, frame_height)

            if has_visible_content(frames):
                union_width, union_height, union_x, union_y = calculate_sprite_bounds(frames)

                if union_width > 0 and union_height > 0:
                    offset_x, offset_y = calculate_center_offset(
                        frame_width, frame_height,
                        union_width, union_height,
                        union_x, union_y
                    )

                    centered_frames = [apply_offset_to_frame(frame, offset_x, offset_y) for frame in frames]

                    metadata["rows"].append({
                        "union_width": union_width,
                        "union_height": union_height,
                        "offset_x": offset_x,
                        "offset_y": offset_y
                    })

                    all_rows.append(centered_frames)

    new_width = 8 * frame_width
    new_height = len(all_rows) * frame_height
    new_img = Image.new("RGBA", (new_width, new_height))

    for idx, row in enumerate(all_rows):
        for j, frame in enumerate(row):
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

def preprocess_poke(image_paths, n_rows, n_cols, frame_width, frame_height):
    for image_path in image_paths:
        process_image(image_path, n_rows, n_cols, frame_width, frame_height)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    parser = argparse.ArgumentParser(
        description="Reorder frames from multiple PNGs, skipping empty sprites and generating metadata."
    )
    parser.add_argument("--input_path", type=str, default="assets/HGSS")
    parser.add_argument("--inputs", nargs='+', default=["PokGen1_transparent.png", "PokGen2_transparent.png", "PokGen3_transparent.png", "PokGen4_transparent.png"])
    parser.add_argument("--n_rows", type=int, default=20)
    parser.add_argument("--n_cols", type=int, default=15)
    parser.add_argument("--frame_width", type=int, default=32)
    parser.add_argument("--frame_height", type=int, default=32)

    args = parser.parse_args()

    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)

    input_paths = [os.path.join(args.input_path, input) for input in args.inputs]
    preprocess_poke(
        input_paths, args.n_rows, args.n_cols, args.frame_width, args.frame_height
    )
