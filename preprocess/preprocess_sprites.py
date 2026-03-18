import argparse
import os
import json
from PIL import Image
import numpy as np

# Big Pokémon IDs - these go to the big sprite sheet
BIG_IDS = [
    208, 249, 250, 483, 484, 486, 487, 493, 321, 382, 383, 384
]

def has_visible_content(frames):
    """Check if any frame has visible content"""
    for frame in frames:
        if frame.mode != 'RGBA':
            frame = frame.convert('RGBA')
        arr = np.array(frame)
        if np.any(arr[:, :, 3] > 10):
            return True
    return False

def extract_frame_block(img, col_idx, row_idx, frame_width, frame_height):
    """Extract 8 frames from a block"""
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
    """Extract 8 frames for big Pokémon"""
    row_idx = pokemon_idx * 2
    frames = []

    for r in range(2):
        for c in range(n_cols):
            x0 = c * frame_width
            y0 = (row_idx + r) * frame_height
            frames.append(img.crop((x0, y0, x0 + frame_width, y0 + frame_height)))

    # Reorder: aa, ba, ac, bc, ab, bb, ad, bd
    aa, ab, ac, ad, ba, bb, bc, bd = frames
    return [aa, ba, ac, bc, ab, bb, ad, bd]

def center_sprite_in_frame(frame, target_width, target_height):
    """Center sprite content within frame"""
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    arr = np.array(frame)
    alpha = arr[:, :, 3]
    non_transparent = alpha > 10

    if not np.any(non_transparent):
        return Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))

    rows = np.any(non_transparent, axis=1)
    cols = np.any(non_transparent, axis=0)

    y_min, y_max = np.where(rows)[0][[0, -1]]
    x_min, x_max = np.where(cols)[0][[0, -1]]

    content_width = x_max - x_min + 1
    content_height = y_max - y_min + 1
    center_x = (target_width - content_width) // 2
    center_y = (target_height - content_height) // 2

    new_frame = Image.new('RGBA', (target_width, target_height), (0, 0, 0, 0))
    sprite_content = frame.crop((x_min, y_min, x_max + 1, y_max + 1))
    new_frame.paste(sprite_content, (center_x, center_y), sprite_content)

    return new_frame

def process_regular_sheets(image_paths, frame_width, frame_height):
    """Process all regular sprite sheets with FIXED dimensions"""
    all_sprites = []
    metadata = []

    # Fixed dimensions from original script
    N_ROWS = 20
    N_COLS = 15

    for sheet_idx, image_path in enumerate(image_paths):
        print(f"Processing sheet {sheet_idx+1}: {os.path.basename(image_path)}")
        img = Image.open(image_path)

        # Verify image can accommodate these dimensions
        expected_width = N_COLS * (2 * frame_width + 1)
        expected_height = N_ROWS * (4 * frame_height + 1)

        actual_width, actual_height = img.size
        print(f"  Expected: {expected_width}x{expected_height}, Actual: {actual_width}x{actual_height}")

        # Process ALL rows and columns
        sprites_in_sheet = 0
        for row in range(N_ROWS):
            for col in range(N_COLS):
                # Check if we're still within image bounds
                block_bottom = row * (4 * frame_height + 1) + 4 * frame_height
                block_right = col * (2 * frame_width + 1) + 2 * frame_width

                if block_bottom > actual_height or block_right > actual_width:
                    print(f"  Warning: Block at row {row}, col {col} exceeds image bounds")
                    continue

                frames = extract_frame_block(img, col, row, frame_width, frame_height)

                if has_visible_content(frames):
                    # Center each frame
                    centered_frames = [center_sprite_in_frame(f, frame_width, frame_height) for f in frames]
                    all_sprites.append(centered_frames)
                    metadata.append({
                        "sheet": sheet_idx + 1,
                        "row": row,
                        "col": col,
                        "sprite_index": len(all_sprites) - 1
                    })
                    sprites_in_sheet += 1

        print(f"  Found {sprites_in_sheet} sprites in this sheet")

    return all_sprites, metadata

def process_big_sheet(image_path, frame_width, frame_height):
    """Process big sprite sheet"""
    all_big_sprites = []
    metadata = []

    print(f"Processing big sheet: {os.path.basename(image_path)}")
    img = Image.open(image_path)

    # Big sheet has 13 Pokémon arranged in pairs of rows
    N_POKEMON = 13
    N_COLS = 4

    for idx in range(N_POKEMON):
        if idx >= len(BIG_IDS):
            break

        pokedex_id = BIG_IDS[idx]
        frames = extract_big_frames(img, idx, N_COLS, frame_width, frame_height)

        if has_visible_content(frames):
            centered_frames = [center_sprite_in_frame(f, frame_width, frame_height) for f in frames]
            all_big_sprites.append((pokedex_id, centered_frames))
            metadata.append({"id": pokedex_id, "row": idx})
            print(f"  Processed big Pokémon #{pokedex_id}")

    return all_big_sprites, metadata

def create_output(sprites, frame_width, frame_height, output_path, metadata_path, is_big=False):
    """Create output PNG and metadata JSON"""
    if not sprites:
        print("No sprites to save!")
        return

    # Create image
    cols_per_sprite = 8
    rows = len(sprites)
    output_img = Image.new("RGBA", (cols_per_sprite * frame_width, rows * frame_height))

    for row_idx, sprite_data in enumerate(sprites):
        if is_big:
            frames = sprite_data[1]  # (pokedex_id, frames)
        else:
            frames = sprite_data  # just frames

        for col_idx, frame in enumerate(frames):
            x = col_idx * frame_width
            y = row_idx * frame_height
            output_img.paste(frame, (x, y))

    # Save
    output_img.save(output_path)

    # Save metadata
    if is_big:
        metadata = {
            "frame_width": frame_width,
            "frame_height": frame_height,
            "is_big": True,
            "sprites": [{"id": pid, "row": idx} for idx, (pid, _) in enumerate(sprites)]
        }
    else:
        metadata = {
            "frame_width": frame_width,
            "frame_height": frame_height,
            "is_big": False,
            "total_sprites": len(sprites)
        }

    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)

    print(f"Saved {len(sprites)} sprites to {output_path}")
    print(f"Saved metadata to {metadata_path}")

def main():
    parser = argparse.ArgumentParser(description="Process Pokémon sprites")
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

    os.makedirs(args.output_dir, exist_ok=True)

    # Process regular sprites
    regular_paths = [os.path.join(args.input_path, f) for f in args.regular_inputs]
    big_path = os.path.join(args.input_path, args.big_input)

    print("=" * 60)
    print(f"Processing with {args.n_rows} rows, {args.n_cols} columns per sheet")
    print("=" * 60)

    # Use fixed dimensions
    N_ROWS = args.n_rows
    N_COLS = args.n_cols

    all_sprites = []
    metadata = []

    for sheet_idx, image_path in enumerate(regular_paths):
        print(f"Processing sheet {sheet_idx+1}: {os.path.basename(image_path)}")
        img = Image.open(image_path)

        sprites_in_sheet = 0
        for row in range(N_ROWS):
            for col in range(N_COLS):
                frames = extract_frame_block(img, col, row, args.regular_width, args.regular_height)

                if has_visible_content(frames):
                    centered_frames = [center_sprite_in_frame(f, args.regular_width, args.regular_height) for f in frames]
                    all_sprites.append(centered_frames)
                    metadata.append({
                        "sheet": sheet_idx + 1,
                        "row": row,
                        "col": col,
                        "sprite_index": len(all_sprites) - 1
                    })
                    sprites_in_sheet += 1

        print(f"  Found {sprites_in_sheet} sprites in this sheet")

    print("\n" + "=" * 60)
    # Process big sprites
    all_big_sprites = []
    big_metadata = []

    print(f"Processing big sheet: {os.path.basename(big_path)}")
    img = Image.open(big_path)

    # Big sheet has 13 Pokémon
    for idx in range(13):
        if idx >= len(BIG_IDS):
            break

        pokedex_id = BIG_IDS[idx]
        frames = extract_big_frames(img, idx, 4, args.big_width, args.big_height)

        if has_visible_content(frames):
            centered_frames = [center_sprite_in_frame(f, args.big_width, args.big_height) for f in frames]
            all_big_sprites.append((pokedex_id, centered_frames))
            big_metadata.append({"id": pokedex_id, "row": idx})
            print(f"  Processed big Pokémon #{pokedex_id}")

    print("\n" + "=" * 60)
    # Save outputs
    create_output(
        all_sprites, args.regular_width, args.regular_height,
        os.path.join(args.output_dir, "reordered_sprites.png"),
        os.path.join(args.output_dir, "reordered_sprites_metadata.json"),
        is_big=False
    )

    create_output(
        all_big_sprites, args.big_width, args.big_height,
        os.path.join(args.output_dir, "reordered_sprites_big.png"),
        os.path.join(args.output_dir, "reordered_sprites_big_metadata.json"),
        is_big=True
    )

    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"Regular sprites: {len(all_sprites)}")
    print(f"Big sprites: {len(all_big_sprites)}")
    print(f"Total: {len(all_sprites) + len(all_big_sprites)}")

if __name__ == "__main__":
    main()
