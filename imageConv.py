from PIL import Image
import sys

def png_to_rle_array(filename, out_width=640, out_height=400, array_name="logo"):
    img = Image.open(filename).convert("L")
    img = img.resize((out_width, out_height), Image.Resampling.LANCZOS)

    # threshold → 0/1
    threshold = 128
    bw = img.point(lambda x: 0 if x < threshold else 1, "1")
    pixels = list(bw.getdata())

    # RLE encode (store runs of 0/1)
    rle = []
    current = pixels[0]
    length = 1
    for p in pixels[1:]:
        if p == current and length < 255:  # max length for 1 byte
            length += 1
        else:
            rle.append((length, current))
            current = p
            length = 1
    rle.append((length, current))

    # Build C array (store [length, color] as two bytes each run)
    data = []
    for length, color in rle:
        data.append(length)
        data.append(color)

    # Print C array
    print(f"// RLE-compressed image")
    print(f"// Original: {len(pixels)//8} bytes (raw 1bpp)")
    print(f"// Compressed: {len(data)} bytes")
    print(f"// Dimensions: {out_width} x {out_height}\n")

    print(f"const unsigned char {array_name}[] = {{")
    for i, val in enumerate(data):
        if i % 16 == 0:
            print("   ", end="")
        print(f"0x{val:02X}, ", end="")
        if i % 16 == 15:
            print()
    print("\n};")

if __name__ == "__main__":
    filename = sys.argv[1] if len(sys.argv) > 1 else "logo.png"
    png_to_rle_array(filename)
