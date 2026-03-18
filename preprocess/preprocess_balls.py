import argparse
import os
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

def extract_and_center_frames(img, col_idx, n_rows, frame_width, frame_height=16, target_height=23):
    """Extract frames from a column and center them in target_height."""
    x0 = col_idx * frame_width
    frames = []

    # Process regular frames (n_rows-1 of them)
    for row_idx in range(n_rows - 1):
        # Extract original frame
        original = img.crop((
            x0,
            row_idx * frame_height,
            x0 + frame_width,
            (row_idx + 1) * frame_height
        ))

        # Center it in target_height
        centered_frame = Image.new('RGBA', (frame_width, target_height))
        y_offset = (target_height - frame_height) // 2
        centered_frame.paste(original, (0, y_offset))
        frames.append(centered_frame)

    # Process the last frame (exception frame, already target_height tall)
    last_frame = img.crop((
        x0,
        (n_rows - 1) * frame_height,
        x0 + frame_width,
        (n_rows - 1) * frame_height + target_height
    ))

    if last_frame.mode != 'RGBA':
        last_frame = last_frame.convert('RGBA')

    frames.append(last_frame)

    return frames

def process_image(image_path, n_rows, n_cols, frame_width, frame_height=16, target_height=23):
    """Process a single image file with centered frames."""
    img = Image.open(image_path)
    print(f"Processing {image_path}...")

    all_rows = []
    # Extract all frame blocks
    for col_idx in range(n_cols-1, -1, -1):
        frames = extract_and_center_frames(img, col_idx, n_rows, frame_width, frame_height, target_height)

        # Only keep non-empty frame sets
        if has_visible_content(frames):
            all_rows.append(frames)

    # Create output image
    new_width = n_rows * frame_width
    new_height = len(all_rows) * target_height  # Use target_height for output
    new_img = Image.new("RGBA", (new_width, new_height))

    for row_idx, row in enumerate(all_rows):
        for col_idx, frame in enumerate(row):
            new_img.paste(frame, (col_idx * frame_width, row_idx * target_height))

    # Save with _reordered suffix
    input_path = Path(image_path)
    output_path = input_path.with_name("reordered_pokeballs.png")
    new_img.save(output_path)

    print(f"Saved to {output_path}")
    print(f"Processed {len(all_rows)} pokeball sprite rows")

def preprocess_poke(image_paths, n_rows, n_cols, frame_width, frame_height, target_height=23):
    """Process multiple image files."""
    for image_path in image_paths:
        process_image(image_path, n_rows, n_cols, frame_width, frame_height, target_height)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    parser = argparse.ArgumentParser(
        description="Reorder frames with centered 16px frames in 23px transparent background."
    )
    parser.add_argument("--input_path", type=str, default="assets/HGSS")  # <-- Changed to relative path
    parser.add_argument("--inputs", nargs='+', default=["Pokeballs_transparent.png"])
    parser.add_argument("--n_rows", type=int, default=10, help="Number of rows of sections per image")
    parser.add_argument("--n_cols", type=int, default=4, help="Number of columns of sections per image")
    parser.add_argument("--frame_width", type=int, default=16, help="Width of a single frame")
    parser.add_argument("--frame_height", type=int, default=16, help="Original height of regular frames")
    parser.add_argument("--target_height", type=int, default=23, help="Target height for all frames (centered)")

    args = parser.parse_args()

    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)

    input_paths = [os.path.join(args.input_path, input) for input in args.inputs]

    preprocess_poke(
        input_paths,
        args.n_rows,
        args.n_cols,
        args.frame_width,
        args.frame_height,
        args.target_height
    )
