import argparse
import os
from pathlib import Path
from PIL import Image
import numpy as np

def has_visible_content(frame):
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')
    arr = np.array(frame)
    return np.any(arr[:, :, 3] > 0)

def get_content_bounds(frame):
    """
    Get the bounding box of non-transparent content in the frame.
    Returns (left, top, right, bottom) or None if no content.
    """
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    arr = np.array(frame)
    alpha = arr[:, :, 3]

    # Find rows and columns with visible pixels
    rows_with_content = np.any(alpha > 0, axis=1)
    cols_with_content = np.any(alpha > 0, axis=0)

    if not np.any(rows_with_content) or not np.any(cols_with_content):
        return None

    top = np.argmax(rows_with_content)
    bottom = len(rows_with_content) - np.argmax(rows_with_content[::-1])
    left = np.argmax(cols_with_content)
    right = len(cols_with_content) - np.argmax(cols_with_content[::-1])

    return (left, top, right, bottom)

def center_frame_content(frame, target_size=32):
    """
    Center the non-transparent content of a frame in a target_size x target_size canvas.
    """
    if frame.mode != 'RGBA':
        frame = frame.convert('RGBA')

    bounds = get_content_bounds(frame)

    if bounds is None:
        # No content, return empty frame
        return Image.new('RGBA', (target_size, target_size), (0, 0, 0, 0))

    left, top, right, bottom = bounds

    # Extract the content
    content = frame.crop((left, top, right, bottom))
    content_width = right - left
    content_height = bottom - top

    # Create new centered frame
    centered_frame = Image.new('RGBA', (target_size, target_size), (0, 0, 0, 0))

    # Calculate position to center the content
    x_offset = (target_size - content_width) // 2
    y_offset = (target_size - content_height) // 2

    # Paste content at centered position
    centered_frame.paste(content, (x_offset, y_offset))

    return centered_frame

def extract_bc_frames(img, n_block_rows, n_block_cols, frame_size, skip_blocks):
    """
    Extract only the 'bc' frame from each trainer block.

    Each block is 3 columns x 4 rows of frames:
    aa ab ac
    ba bb bc  <- we want 'bc' (column 2, row 1)
    ca cb cc
    da db dc

    Args:
        img: PIL Image
        n_block_rows: number of block rows (8)
        n_block_cols: number of block columns (10)
        frame_size: size of individual frame (32x32)
        skip_blocks: list of block indices to skip (row-major order, 0-79)
    """
    bc_frames = []
    skip_set = set(skip_blocks)

    # Size of each block in pixels
    block_width = 3 * frame_size  # 3 frames wide
    block_height = 4 * frame_size  # 4 frames tall

    block_idx = 0
    for block_row in range(n_block_rows):
        for block_col in range(n_block_cols):
            # Check if this block should be skipped
            if block_idx in skip_set:
                print(f"Skipping block {block_idx} (row {block_row}, col {block_col})")
                block_idx += 1
                continue

            # Calculate top-left corner of this block
            block_x0 = block_col * block_width
            block_y0 = block_row * block_height

            # Calculate position of 'bc' frame within the block
            # 'bc' is at column 2 (index 2), row 1 (index 1)
            bc_x0 = block_x0 + (2 * frame_size)
            bc_y0 = block_y0 + (1 * frame_size)

            # Extract the 'bc' frame
            bc_frame = img.crop((
                bc_x0,
                bc_y0,
                bc_x0 + frame_size,
                bc_y0 + frame_size
            ))

            if bc_frame.mode != 'RGBA':
                bc_frame = bc_frame.convert('RGBA')

            if has_visible_content(bc_frame):
                # Center the content in the frame
                centered_frame = center_frame_content(bc_frame, frame_size)
                bc_frames.append(centered_frame)

            block_idx += 1

    return bc_frames

def process_trainer_image(image_path, n_block_rows, n_block_cols, frame_size, skip_blocks):
    img = Image.open(image_path)
    print(f"Processing {image_path}...")

    bc_frames = extract_bc_frames(img, n_block_rows, n_block_cols, frame_size, skip_blocks)

    # Create output image (single column of bc frames)
    new_width = frame_size
    new_height = len(bc_frames) * frame_size
    new_img = Image.new("RGBA", (new_width, new_height))

    for idx, frame in enumerate(bc_frames):
        new_img.paste(frame, (0, idx * frame_size))

    input_path = Path(image_path)
    output_path = input_path.with_name("reordered_trainers.png")
    new_img.save(output_path)

    print(f"Saved to {output_path}")
    print(f"Processed {len(bc_frames)} trainer 'bc' frames (centered)")

def preprocess_trainers(image_paths, n_block_rows, n_block_cols, frame_size, skip_blocks):
    for image_path in image_paths:
        process_trainer_image(image_path, n_block_rows, n_block_cols, frame_size, skip_blocks)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    parser = argparse.ArgumentParser(
        description="Extract 'bc' frames from trainer sprite blocks into a single column."
    )
    parser.add_argument("--input_path", type=str, default="assets/HGSS")
    parser.add_argument("--inputs", nargs='+', default=["Trainers_transparent.png"])
    parser.add_argument("--n_block_rows", type=int, default=8,
                        help="Number of block rows")
    parser.add_argument("--n_block_cols", type=int, default=10,
                        help="Number of block columns")
    parser.add_argument("--frame_size", type=int, default=32,
                        help="Size of individual frame (32x32)")
    parser.add_argument("--skip_blocks", nargs='*', type=int, default=[4],
                        help="Block indices to skip (0-79, row-major order)")

    args = parser.parse_args()

    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)

    input_paths = [os.path.join(args.input_path, input) for input in args.inputs]

    preprocess_trainers(
        input_paths,
        args.n_block_rows,
        args.n_block_cols,
        args.frame_size,
        args.skip_blocks
    )
