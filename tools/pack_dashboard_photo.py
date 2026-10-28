"""Encode the prepared photo as a 220x220, MSB-first Adafruit GFX bitmap."""
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parent.parent
source = Image.open(root / "assets" / "dashboard_photo_source.png")
assert source.width == source.height, "The prepared photo must be square"
size = 220
bitmap = source.convert("L").resize((size, size), Image.Resampling.LANCZOS).convert("1")
bitmap.save(root / "assets" / "dashboard_photo.png")
# Pillow uses 1 for white; Adafruit GFX uses 1 for the foreground (black).
packed = bytes(value ^ 0xFF for value in bitmap.tobytes())
assert len(packed) == size * ((size + 7) // 8) == 6160
rows = ["  " + ", ".join(f"0x{b:02X}" for b in packed[i:i+12]) for i in range(0, len(packed), 12)]
header = """#pragma once
// Personal photo, 1 bit per pixel, MSB first. Black pixels have value 1.
// Regenerate from assets/dashboard_photo_source.png with assets/pack_dashboard_photo.py.
static const int DASHBOARD_PHOTO_WIDTH = 220;
static const int DASHBOARD_PHOTO_HEIGHT = 220;
static const unsigned char DASHBOARD_PHOTO_BITMAP[] PROGMEM = {
""" + ",\n".join(rows) + "\n};\n"
(root / "src" / "dashboard_photo.local.h").write_text(header, encoding="utf-8")
bitmap.resize((440, 440), Image.Resampling.NEAREST).save(root / "assets" / "dashboard_photo_preview.png")
print("Photo bitmap encoded: 220x220 pixels, 6160 bytes")
