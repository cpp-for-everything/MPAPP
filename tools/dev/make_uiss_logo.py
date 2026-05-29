#!/usr/bin/env python3
"""Generate a small placeholder УИСС / TU-Sofia logo PNG (no deps).

Writes examples/uiss/assets/tu_logo.png — a navy rounded plate with a
lighter inner band, standing in for the real TU logo so the app
demonstrates real image loading (GtkPicture / BitmapImage / BitmapFactory
all decode this PNG on their platform). Pure stdlib: manual PNG encoding
via zlib, so it runs anywhere Python does.
"""
import struct
import zlib
import os

W, H = 200, 80
NAVY  = (29, 53, 87)      # #1D3557
LIGHT = (168, 218, 220)   # #A8DADC band

def px(x, y):
    # A simple plate: navy everywhere, a light horizontal band in the middle.
    if 30 <= y < 50:
        return LIGHT
    return NAVY

def main():
    raw = bytearray()
    for y in range(H):
        raw.append(0)  # filter type 0 (None) per scanline
        for x in range(W):
            r, g, b = px(x, y)
            raw += bytes((r, g, b))

    def chunk(tag, data):
        c = struct.pack(">I", len(data)) + tag + data
        c += struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        return c

    sig  = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0)  # 8-bit RGB
    idat = zlib.compress(bytes(raw), 9)

    out_dir = os.path.join(os.path.dirname(__file__), "..", "..",
                           "examples", "uiss", "assets")
    out_dir = os.path.abspath(out_dir)
    os.makedirs(out_dir, exist_ok=True)
    out = os.path.join(out_dir, "tu_logo.png")
    with open(out, "wb") as f:
        f.write(sig)
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", idat))
        f.write(chunk(b"IEND", b""))
    print("wrote", out, os.path.getsize(out), "bytes")

if __name__ == "__main__":
    main()
