import argparse
from pathlib import Path
from PIL import Image

def preprocess_poke(image_path, n_rows, n_cols, frame_width, frame_height):
    img = Image.open(image_path)

    all_rows = []

    for row_idx in range(n_rows):
        for col_idx in range(n_cols):
            x0 = col_idx * (2 * frame_width + 1)
            y0 = row_idx *(4 * frame_height + 1)

            # Extract the 2x4 block of frames
            aa = img.crop((x0, y0, x0 + frame_width, y0 + frame_height))
            ab = img.crop((x0 + frame_width, y0, x0 + 2*frame_width, y0 + frame_height))

            ba = img.crop((x0, y0 + frame_height, x0 + frame_width, y0 + 2*frame_height))
            bb = img.crop((x0 + frame_width, y0 + frame_height, x0 + 2*frame_width, y0 + 2*frame_height))

            ca = img.crop((x0, y0 + 2*frame_height, x0 + frame_width, y0 + 3*frame_height))
            cb = img.crop((x0 + frame_width, y0 + 2*frame_height, x0 + 2*frame_width, y0 + 3*frame_height))

            da = img.crop((x0, y0 + 3*frame_height, x0 + frame_width, y0 + 4*frame_height))
            db = img.crop((x0 + frame_width, y0 + 3*frame_height, x0 + 2*frame_width, y0 + 4*frame_height))

            # Reorder frames as: aa, ba, ab, bb, ca, da, cb, db
            new_row = [aa, ba, ab, bb, ca, da, cb, db]
            all_rows.append(new_row)

    new_width = 8 * frame_width
    new_height = len(all_rows)  * frame_height
    new_img = Image.new("RGBA", (new_width, new_height))

    for idx, row in enumerate(all_rows):
        for j, frame in enumerate(row):
            new_img.paste(frame, (j * frame_width, idx * frame_height))

    # Generate output path in the same folder as input
    input_path = Path(image_path)
    output_path = input_path.with_name(input_path.stem + "_reordered.png")
    new_img.save(output_path)
    print(f"Saved reordered image to {output_path}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Reorder frames from a PNG into 8-frame rows.")
    parser.add_argument("input", help="Input PNG file path")
    parser.add_argument("--n_rows", type=int, default=15, help="Number of rows of sections (default 15)")
    parser.add_argument("--n_cols", type=int, default=11, help="Number of columns of sections (default 11)")
    parser.add_argument("--frame_width", type=int, default=32, help="Width of a single frame (default 32)")
    parser.add_argument("--frame_height", type=int, default=32, help="Height of a single frame (default 32)")

    args = parser.parse_args()

    preprocess_poke(
        args.input, args.n_rows, args.n_cols, args.frame_width, args.frame_height
    )

