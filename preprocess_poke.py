import argparse
from pathlib import Path
from PIL import Image
import numpy as np


def has_visible_content(frames):
    """Check if any frame in the set has non-transparent pixels."""
    for frame in frames:
        if frame.mode != 'RGBA':
            frame = frame.convert('RGBA')
        arr = np.array(frame)
        if np.any(arr[:, :, 3] > 0):
            return True
    return False


def extract_frame_block(img, col_idx, row_idx, frame_width, frame_height):
    """Extract a 2x4 block of frames and reorder them."""
    x0 = col_idx * (2 * frame_width + 1)
    y0 = row_idx * (4 * frame_height + 1)

    # Extract 2x4 grid of frames
    aa = img.crop((x0, y0, x0 + frame_width, y0 + frame_height))
    ab = img.crop((x0 + frame_width, y0, x0 + 2*frame_width, y0 + frame_height))

    ba = img.crop((x0, y0 + frame_height, x0 + frame_width, y0 + 2*frame_height))
    bb = img.crop((x0 + frame_width, y0 + frame_height, x0 + 2*frame_width, y0 + 2*frame_height))

    ca = img.crop((x0, y0 + 2*frame_height, x0 + frame_width, y0 + 3*frame_height))
    cb = img.crop((x0 + frame_width, y0 + 2*frame_height, x0 + 2*frame_width, y0 + 3*frame_height))

    da = img.crop((x0, y0 + 3*frame_height, x0 + frame_width, y0 + 4*frame_height))
    db = img.crop((x0 + frame_width, y0 + 3*frame_height, x0 + 2*frame_width, y0 + 4*frame_height))

    # Reorder as: aa, ba, ab, bb, ca, da, cb, db
    return [aa, ba, ab, bb, ca, da, cb, db]


def process_image(image_path, n_rows, n_cols, frame_width, frame_height):
    """Process a single image file."""
    img = Image.open(image_path)
    print(f"Processing {image_path}...")

    all_rows = []

    # Extract all frame blocks
    for row_idx in range(n_rows):
        for col_idx in range(n_cols):
            frames = extract_frame_block(img, col_idx, row_idx, frame_width, frame_height)

            # Only keep non-empty frame sets
            if has_visible_content(frames):
                all_rows.append(frames)

    # Create output image
    new_width = 8 * frame_width
    new_height = len(all_rows) * frame_height
    new_img = Image.new("RGBA", (new_width, new_height))

    for idx, row in enumerate(all_rows):
        for j, frame in enumerate(row):
            new_img.paste(frame, (j * frame_width, idx * frame_height))

    # Save with _reordered suffix
    input_path = Path(image_path)
    output_path = input_path.with_name(input_path.stem + "_reordered.png")
    new_img.save(output_path)
    print(f"Saved to {output_path}")


def preprocess_poke(image_paths, n_rows, n_cols, frame_width, frame_height):
    """Process multiple image files, creating separate output for each."""
    for image_path in image_paths:
        process_image(image_path, n_rows, n_cols, frame_width, frame_height)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Reorder frames from multiple PNGs, skipping empty sprites."
    )
    parser.add_argument("inputs", nargs='+', help="Input PNG file paths")
    parser.add_argument("--n_rows", type=int, default=15,
                        help="Number of rows of sections per image (default 15)")
    parser.add_argument("--n_cols", type=int, default=11,
                        help="Number of columns of sections per image (default 11)")
    parser.add_argument("--frame_width", type=int, default=32,
                        help="Width of a single frame (default 32)")
    parser.add_argument("--frame_height", type=int, default=32,
                        help="Height of a single frame (default 32)")

    args = parser.parse_args()

    preprocess_poke(
        args.inputs, args.n_rows, args.n_cols, args.frame_width, args.frame_height
    )
