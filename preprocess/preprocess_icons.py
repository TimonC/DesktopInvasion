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

def extract_all_frames(img, n_rows, n_cols, frame_width, frame_height):
    frames = []

    for row_idx in range(n_rows):
        for col_idx in range(n_cols):
            x0 = col_idx * frame_width
            y0 = row_idx * frame_height

            frame = img.crop((
                x0,
                y0,
                x0 + frame_width,
                y0 + frame_height
            ))

            if frame.mode != 'RGBA':
                frame = frame.convert('RGBA')

            if has_visible_content(frame):
                frames.append(frame)

    return frames

def process_image(image_path, n_rows, n_cols, frame_width, frame_height):
    img = Image.open(image_path)
    print(f"Processing {image_path}...")

    frames = extract_all_frames(img, n_rows, n_cols, frame_width, frame_height)

    new_width = frame_width
    new_height = len(frames) * frame_height
    new_img = Image.new("RGBA", (new_width, new_height))

    for idx, frame in enumerate(frames):
        new_img.paste(frame, (0, idx * frame_height))

    input_path = Path(image_path)
    output_path = input_path.with_name(input_path.stem + "_reordered.png")
    new_img.save(output_path)

    print(f"Saved to {output_path}")
    print(f"Processed {len(frames)} pokemon icon frames")

def preprocess_icons(image_paths, n_rows, n_cols, frame_width, frame_height):
    for image_path in image_paths:
        process_image(image_path, n_rows, n_cols, frame_width, frame_height)

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    parser = argparse.ArgumentParser(
        description="Extract non-empty Pokemon icon frames into a single column."
    )
    parser.add_argument("--input_path", type=str, default="assets/HGSS")
    parser.add_argument("--inputs", nargs='+', default=["PokemonIcons_filtered.png"])
    parser.add_argument("--n_rows", type=int, default=50)
    parser.add_argument("--n_cols", type=int, default=50)
    parser.add_argument("--frame_width", type=int, default=40)
    parser.add_argument("--frame_height", type=int, default=30)

    args = parser.parse_args()

    if not os.path.isabs(args.input_path):
        args.input_path = os.path.join(project_root, args.input_path)

    input_paths = [os.path.join(args.input_path, input) for input in args.inputs]

    preprocess_icons(
        input_paths,
        args.n_rows,
        args.n_cols,
        args.frame_width,
        args.frame_height
    )
